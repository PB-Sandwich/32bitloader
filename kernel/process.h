
#pragma once

#include <stdint.h>

#include <filesystem/virtual-filesystem.h>
#include <pager.h>
#include <time.h>
#include <x86_64_structures.h>

struct process {
    uintptr_t esp;
    PageTable* page_table;
    VFSFile *stdout, *stdin, *stderr;
    struct time wake_time;
    uint8_t state;
    char name[64];
    uint32_t id;
};

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
    PageTable* page_table;
    VFSFile *stdout, *stdin, *stderr;
};

struct process* create_process(char* name, uint8_t start_state, uint32_t entry_point, uint32_t stack_base_current_table, uint32_t stack_base_apps_table,
    PageTable* table, VFSFile* stdout, VFSFile* stdin, VFSFile* stderr);

struct process* get_process_by_id(uint32_t id);
void remove_process(uint32_t id);

struct process* set_current_process(uint32_t id);
struct process* get_current_process();
struct process* get_next_process();
