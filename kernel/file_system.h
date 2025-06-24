
#pragma once

#include <stdint.h>

enum FileType {
    NONE = 0,
    DIRECTORy,
    FILE,
    EXECUTABLE_FILE,
};

struct Directory {
    uint8_t type;
    uint8_t name[31];
    uint32_t entries[120];
} __attribute__((packed));

struct File {
    uint8_t type;
    uint8_t name[31];
    uint32_t start; // in sectors
    uint32_t length; // in sectors
    uint32_t entry_point;
};

void set_root(uint32_t sector);
void get_file_type(char* path);
void read_directory(char* path);
void read_file_descriptor(char* path);
void read_file(struct File file_descriptor);
