#include <filesystem/virtual-filesystem.h>
#include <heap.h>
#include <memutils.h>
#include <pager.h>
#include <print.h>
#include <process.h>
#include <stdint.h>

struct process_entry {
    struct process process;
    struct process_entry* next;
    struct process_entry* prev;
};

struct process_entry* first_process = NULL;
uint32_t free_pid = 0;
uint32_t number_of_processes = 0;

struct process_entry* current_process = NULL;

uint32_t get_free_pid()
{
    uint32_t pid = free_pid;
    free_pid++;
    return pid;
}

struct process* create_process(char* name, uint8_t start_state, uint32_t entry_point, uint32_t stack_base,
    uint32_t size, VFSFile* stdout, VFSFile* stdin, VFSFile* stderr)
{
    if (strlen(name) > 64 - 1 || strlen(name) == 0) {
        printf("Invalid name length of %d for process\n", strlen(name));
        return NULL;
    }

    struct process_entry* entry = NULL;
    struct process_entry* prev = NULL;
    struct process_entry* next = NULL;
    if (first_process == NULL) {
        first_process = malloc(sizeof(struct process_entry));
        if (first_process == NULL) {
            printf("Failed to malloc space for new process\n");
            return NULL;
        }
        entry = first_process;
        prev = entry;
        next = entry;
    } else {

        entry = malloc(sizeof(struct process_entry));
        if (entry == NULL) {
            printf("Failed to malloc space for new process\n");
            return NULL;
        }

        next = first_process;
        prev = first_process->prev;
    }

    memset(entry, 0, sizeof(struct process_entry));
    entry->prev = prev;
    entry->next = next;

    struct process* process = &entry->process;
    process->id = get_free_pid();
    memcpy(process->name, name, strlen(name));
    process->stdout = stdout;
    process->stdin = stdin;
    process->stderr = stderr;
    process->state = start_state;

    process->page_table = soft_copy_table(kernel_table, 1);
    if (process->page_table == PAGER_ERROR) {
        free(entry);
        printf("Failed to create page table for new process\n");
        return NULL;
    }

    uint32_t page_aligned_size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    for (uint32_t i = 0; i < page_aligned_size / PAGE_SIZE; i++) {
        new_page(PAGER_ERROR, &process->page_table->pde, 0);
    }

    struct process_data* data = &entry->process.process_data;
    data->registers.esp = stack_base;
    data->registers.ebp = stack_base;

    struct interrupt_frame* frame = &data->frame;
    frame->eip = entry_point;
    frame->esp = stack_base;

    __asm__ volatile(
        "mov %%cs, %%ax\n\t"
        "mov %%ax, %0\n\t"
        "mov %%ss, %%ax\n\t"
        "mov %%ax, %1\n\t"
        "mov %%ds, %%ax\n\t"
        "mov %%ax, %2\n\t"
        "mov %%es, %%ax\n\t"
        "mov %%ax, %3\n\t"
        "mov %%fs, %%ax\n\t"
        "mov %%ax, %4\n\t"

        : "=m"(frame->cs), "=m"(frame->ss), "=m"(frame->ds),
        "=m"(frame->es), "=m"(frame->fs)
        :
        : "eax");

    __asm__ volatile(
        "mov %%gs, %%ax\n\t"
        "mov %%ax, %0\n\t"
        "pushf\n\t"
        "pop %%eax\n\t"
        "mov %%eax, %1\n\t"
        : "=m"(frame->gs), "=m"(frame->eflags)
        :
        : "eax"

    );
    frame->eflags |= 1 << 9; // make sure the interrupt flag is set

    number_of_processes++;
    next->prev = entry;
    prev->next = entry;
    return &entry->process;
}

struct process* get_process_by_id(uint32_t id)
{
    struct process_entry* entry = first_process;
    while (entry->process.id != id) {
        entry = entry->next;
        if (entry == NULL) {
            printf("Process with id %d does not exist and can not be retrieved\n", id);
            return NULL;
        }
    }
    return &entry->process;
}

void remove_process(uint32_t id)
{
    struct process_entry* entry = first_process;
    while (entry->process.id != id) {
        entry = entry->next;
        if (entry == first_process) {
            printf("Process with id %d does not exist and can not be removed\n", id);
            return;
        }
    }

    struct process* process = &entry->process;

    if (process->stdout != NULL) {
        vfs_close_file(process->stdout);
    }
    if (process->stdin != NULL) {
        vfs_close_file(process->stdin);
    }
    if (process->stderr != NULL) {
        vfs_close_file(process->stderr);
    }

    struct process_entry* prev = entry->prev;
    struct process_entry* next = entry->next;

    prev->next = next;
    next->prev = prev;

    free(entry);

    number_of_processes--;
    return;
}

struct process* set_current_process(uint32_t id)
{
    struct process_entry* entry = first_process;
    while (entry->process.id != id) {
        entry = entry->next;
        if (entry == first_process) {
            printf("Process with id %d does not exist and can not be set to current process\n", id);
            return NULL;
        }
    }

    current_process = entry;

    return &current_process->process;
}

struct process* get_current_process()
{
    return &current_process->process;
}
struct process* get_next_process()
{
    current_process = current_process->next;
    return &current_process->process;
}
