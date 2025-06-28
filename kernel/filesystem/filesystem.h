
#pragma once

#include <stdint.h>

#define DIRECTORY_LENGTH 119
#define SECTOR_SIZE 512

typedef struct {
    uint32_t root;
    uint32_t fs_size;
} __attribute__((packed)) FS_FileSystemHeader;

enum {
    FS_NONE = 0,
    FS_DIRECTORY = 1,
    FS_FILE = 2,
    FS_BLOCK_DEVICE = 3,
};

enum {
    FS_EXECUTABLE = 1,
    FS_PROTECT_LOCATION = 0b10,
};

typedef struct {
    uint8_t type;
    uint32_t string_sector;
    uint32_t name_offset;
    uint32_t path_offset;
    uint32_t last_accessed_path_offset;
    uint32_t last_modified_path_offset;
    uint32_t attributes;
    uint32_t file_size_bytes;
    uint32_t framgment_size_sectors;
    uint32_t file_sector;
} __attribute__((packed)) FS_HDDFileDescriptor;

typedef struct {
    uint32_t hdd_sector;
    uint32_t position;
    char* name;
    char* path;
    char* last_accessed_path;
    char* last_modified_path;
    FS_HDDFileDescriptor hdd_file_descriptor;
} __attribute__((packed)) FS_RAMFileDescriptor;

void fs_init_filesystem(uint32_t fsheader_sector);

FS_RAMFileDescriptor* fs_open_file(char* path);
void fs_close_file(FS_RAMFileDescriptor* file);

uint32_t fs_read_file(void* buffer, uint32_t size, FS_RAMFileDescriptor* file);

FS_RAMFileDescriptor* fs_read_directory(FS_RAMFileDescriptor* file);
void fs_free_directory(FS_RAMFileDescriptor* entries, uint32_t size);
