
#include <estros/file.h>
#include <estros/pager.h>
#include <estros/process.h>
#include <stdint.h>
#include <string.h>

#ifndef NULL
#define NULL (void*)0
#endif

// int strlen(const char* str)
// {
//     int i = 0;
//     for (; str[i] != '\0'; i++)
//         ;
//     return i;
// }
// void* memcpy(void* dest, void* source, uint32_t size)
// {
//     for (uint32_t i = 0; i < size; i++) {
//         ((uint8_t*)dest)[i] = ((uint8_t*)source)[i];
//     }
//     return dest;
// }

Process* launch_file(char* path, File* stdout_file, File* stdin_file, File* stderr_file)
{
    File* exe = open_file(path, ESTROS_READ);
    if (exe == NULL) {
        return NULL;
    }

    // apps size and pages
    uint32_t size = exe->inode->size;

    uint32_t number_of_pages = 0x1000; // TODO: revert back to this after heap init is changed: (size / PAGE_SIZE + 1);

    uintptr_t page_addresses[number_of_pages];

    uint32_t page_addresses_length = 0;
    for (; page_addresses_length < number_of_pages; page_addresses_length++) {

        page_addresses[page_addresses_length] = (uintptr_t)new_page(PAGER_ERROR, PAGER_ERROR);

        if (page_addresses[page_addresses_length] == (uintptr_t)PAGER_ERROR) {
            break;
        }
    }

    if (page_addresses_length < number_of_pages) {
        for (uint32_t i = 0; i < page_addresses_length; i++) {
            free_page(page_addresses, PAGER_ERROR);
        }
        close_file(exe);
        return NULL;
    }

    PDETable* new_apps_table = &create_new_table()->pde;

    if (new_apps_table == PAGER_ERROR) {
        for (uint32_t i = 0; i < page_addresses_length; i++) {
            free_page((void*)page_addresses[i], PAGER_ERROR);
        }
        close_file(exe);
        return NULL;
    }

    for (uint32_t i = 0; i < number_of_pages; i++) {
        new_page(
            (void*)virt_to_phys(page_addresses[i], &get_current_process()->page_table->pde),
            new_apps_table);
    }
    uintptr_t stack_base_current_table = (uintptr_t)new_page(PAGER_ERROR, PAGER_ERROR);
    uintptr_t stack_base_apps_table = (uintptr_t)new_page((void*)virt_to_phys(stack_base_current_table, &get_current_process()->page_table->pde), new_apps_table);

    // load file data
    uintptr_t entry_point = (uint32_t)-1;
    read_file(exe, &entry_point, 4);

    for (uint32_t i = 0; i < number_of_pages; i++) {
        read_file(exe, (void*)page_addresses[i], PAGE_SIZE);
    }

    char name[64];
    uint32_t name_offset = 63;

    for (int i = 0; i < 63; i++) {
        if (path[strlen(path) - 1 - i] == '/') {
            name_offset = strlen(path) - i;
        }
    }

    memcpy(name, path + name_offset, strlen(path + name_offset));

    Process* new_process = create_process(name, PROCESS_RUNNING, entry_point, stack_base_current_table + PAGE_SIZE - 16, stack_base_apps_table + PAGE_SIZE - 16, new_apps_table, stdout_file, stdin_file, stderr_file);

    return new_process;
}
