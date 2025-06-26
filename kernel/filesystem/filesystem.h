
#pragma once

#include <stdint.h>

#define DIRECTORY_LENGTH 119
#define SECTOR_SIZE 512

typedef struct {
    uint32_t root;
    uint32_t fs_size;
} __attribute__((packed)) FileSystemHeader_t;

typedef enum {
    DIRECTORY = 1,
    FILE = 2,
    EXECUTABLE_FILE = 3,
    ERROR,
} FileType;

typedef enum {
    FILE_NOT_FOUND = 1,
    NOT_DIRECTORY = 2,
    TOO_LARGE = 3,
    NAME_TOO_LONG = 4,
} FSError_e;

typedef struct {
    uint8_t type;
    uint8_t name[31];
} __attribute__((packed)) FileHeader_t;

typedef struct {
    FileHeader_t header;
    uint32_t sector;
} DirectoryEntry_t;

typedef struct {
    uint8_t type;
    uint8_t name[31];
    uint32_t entries[119];
    uint32_t link;
} __attribute__((packed)) Directory_t;

typedef struct {
    uint8_t type;
    uint8_t name[31];
    uint32_t start; // in sectors
    uint32_t length; // in sectors
    uint32_t entry_point;
    uint32_t sector;
    uint16_t zero[249];
    uint32_t link;
} __attribute__((packed)) FileDescriptor_t;

typedef struct {
    uint8_t file_type;
    uint8_t type;
    uint8_t message[31];
} __attribute__((packed)) FSError_t;

typedef union {
    Directory_t directory;
    FileDescriptor_t file_descriptor;
    FSError_t error;
} File_t;

// buffer must be at least 512 bytes
void filesystem_init(uint32_t fs_header_sector, uint32_t* fs_buffer);

File_t read_file_descriptor(char* path);
File_t read_file_descriptor_sector(uint32_t sector);

void get_directory_list(DirectoryEntry_t* list, Directory_t dir);
void read_file(uint8_t* buffer, FileDescriptor_t file_descriptor);

FSError_t create_file_descriptor(char* path, char* name, FileDescriptor_t  file);
FSError_t write_file(uint8_t* data, uint32_t size, FileDescriptor_t file_descriptor);
