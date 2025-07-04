
#pragma once

#include <stdint.h>

typedef enum {
    VFS_REGULAR_FILE = 1,
    VFS_BLOCK_DEVICE = 2,
    VFS_CHARACTER_DEVICE = 3,
} VFSFileType;

typedef struct {
    void (*read)(void* file, void* buffer, uint32_t buffer_size);
    void (*write)(void* file, void* buffer, uint32_t buffer_size);
    void (*ioctl)(void* file, uint32_t command, uint32_t arg);
    void (*seek)(void* file, uint32_t offset, uint32_t whence);
    void (*tell)(void* file);
    void (*flush)(void* file);
} VFSFileOperations;

typedef struct {
    uint32_t type;
    uint32_t size;
    VFSFileOperations file_operations;
    uint32_t blocks[14];
} VFSIndexNode;

typedef struct {
    VFSIndexNode* inode;
    void* private_data;
    uint32_t private_data_size;
    uint32_t position;
} VFSFile;

typedef struct {
    VFSIndexNode* inode;
    void* vfs_directory;
    char* name;
} VFSDirectoryEntry;

typedef struct {
    VFSDirectoryEntry* entries;
    uint32_t entries_length;
} VFSDirectory;

/*
 *
 *  Open and close will keep track of refrences to each index node.
 *  The VFS expects that a inode will be safe to remove after calling flush
 *
 */

typedef struct {
    int (*create_inode)(char* path, VFSFileType type);
    VFSIndexNode* (*get_inode)(char* path);
    VFSDirectory* (*get_directory_entries)(char* path);
    int (*write_directory_entries)(VFSDirectory vfs_directory);
} VFSDriverOperations;

// returns 0 on success
int vfs_init(VFSDriverOperations driver_operations, VFSFileOperations default_file_operations);

// returns 0 on success
int vfs_create_regular_file(char* path);
int vfs_create_device_file(char* path, VFSFileOperations fops, VFSFileType type);
int vfs_create_directory(char* path);

VFSDirectory* vfs_open_directory(char* path);
void vfs_close_directory(VFSDirectory* vfs_directory);

// returns NULL on fail
VFSFile* vfs_open_file(char* path);
void vfs_close_file(VFSFile* file);
