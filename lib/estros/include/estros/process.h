#ifndef ESTROS_PROCESS_H
#define ESTROS_PROCESS_H

#include <estros/file.h>
#include <estros/pager.h>
#include <estros/syscall.h>
#include <stdint.h>

typedef struct {
    uintptr_t esp;
    PageTable* page_table;
    File *stdout, *stdin, *stderr;
    // add time
    uint32_t time0;
    uint32_t time1;
    //
    uint8_t state;
    char name[64];
    uint32_t id;
} Process;

enum {
    PROCESS_TERMINATED = 0,
    PROCESS_RUNNING = 1,
    PROCESS_SUSPENDED = 2,
    PROCESS_SLEEPING = 3,
};

struct process_init_data {
    uint8_t initial_state;
    char* name;
    uint32_t entry_point;
    uint32_t stack_base_current_table;
    uint32_t stack_base_apps_table;
    PDETable* page_table;
    File *stdout, *stdin, *stderr;
};

static inline Process* get_current_process()
{
    Process* process;
    __asm__ volatile("int $0x40" : "=b"(process) : "a"(SYSCALL_GET_CURRENT_PROCESS));
    return process;
}

static inline Process* create_process(char* name, uint8_t initial_state, uint32_t entry_point,
    uint32_t stack_base_current_table, uint32_t stack_base_apps_table,
    PDETable* page_table, File* stdout_file, File* stdin_file, File* stderr_file)
{
    struct process_init_data arg = {
        .initial_state = initial_state,
        .name = name,
        .entry_point = entry_point,
        .stack_base_current_table = stack_base_current_table,
        .stack_base_apps_table = stack_base_apps_table,
        .page_table = page_table,
        .stdout = stdout_file,
        .stdin = stdin_file,
        .stderr = stderr_file,
    };
    Process* ret;
    __asm__ volatile("int $0x40" : "=b"(ret) : "a"(SYSCALL_CREATE_PROCESS), "b"(&arg));
    return ret;
}

Process* launch_file(char* path, File* stdout_file, File* stdin_file, File* stderr_file);

#endif
