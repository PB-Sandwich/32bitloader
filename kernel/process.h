
#pragma once

#include <stdint.h>

#include <filesystem/virtual-filesystem.h>
#include <pager.h>
#include <x86_64_structures.h>

struct process_data {
    struct interrupt_frame frame;
    struct registers registers;
    struct sse_registers sse_registers;
};

struct process {
    struct process_data process_data;
    PageTable* page_table;
    VFSFile *stdout, *stdin, *stderr;
    uint8_t state;
    char name[64];
    uint32_t id;
};

struct process_list {
    uint32_t length;
    struct process processes[];
};

enum {
    PROCESS_RUNNING,
    PROCESS_SUSPENDED,
    PROCESS_TERMINATED,
    PROCESS_SLEEPING,
};

struct process_init_data {
    uint8_t initial_state;
    char* name;
    uint32_t entry_point;
    uint32_t stack_base;
    uint32_t size;
    VFSFile *stdout, *stdin, *stderr;
};

struct process* create_process(char* name, uint8_t state, uint32_t entry_point, uint32_t stack_base,
    uint32_t size, VFSFile* stdout, VFSFile* stdin, VFSFile* stderr);

struct process* get_process_by_id(uint32_t id);
void remove_process(uint32_t id);

struct process* set_current_process(uint32_t id);
struct process* get_current_process();
struct process* get_next_process();
