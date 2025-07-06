
#pragma once

#include <stdint.h>

typedef enum {
    VFS_BEG = 1,
    VFS_CUR = 2,
    VFS_END = 3,
} VFSWhence;

typedef enum {
    VFS_ERROR = 0,
    VFS_REGULAR_FILE = 1,
    VFS_BLOCK_DEVICE = 2,
    VFS_CHARACTER_DEVICE = 3,
} VFSFileType;

typedef struct {
    void* (*open)(void* inode); // returns VFSFile
    void (*close)(void* file);
    void (*read)(void* file, void* buffer, uint32_t buffer_size);
    void (*write)(void* file, void* buffer, uint32_t buffer_size);
    void (*ioctl)(void* file, uint32_t command, uint32_t arg);
    void (*seek)(void* file, uint32_t offset, uint32_t whence);
    uint32_t (*tell)(void* file);
    void (*flush)(void* file);
} VFSFileOperations;

typedef struct {
    uint32_t type;
    uint32_t size;
    VFSFileOperations file_operations;
    void* private_data;
    uint32_t number_of_references;
} VFSIndexNode;

typedef struct {
    VFSIndexNode* inode;
    void* private_data;
    uint32_t private_data_size;
    uint32_t position;
} VFSFile;

typedef struct {
    char* path;
} VFSDirectoryEntry;

typedef struct {
    VFSIndexNode inode;
    VFSDirectoryEntry* entries;
    uint32_t entries_length;
} VFSDirectory;

typedef struct {
    int (*create_inode)(char* path, VFSFileType type);
    VFSIndexNode (*get_inode)(char* path);
    VFSDirectory* (*get_directory)(char* path);
    void (*free_inode_data)(VFSIndexNode inode);
} VFSDriverOperations;

/*
 *
 *  All driver functions should NOT modify the VFSIndexNode.number_of_refrences variable.
 *  Open and close will keep track of refrences to each index node.
 *  The VFS will auto remove inodes no longer in use.
 *  The VFS expects that a file will be safe to remove after calling file_operations.flush().
 *  The VFS expects that an inode will be safe to remove after calling free_inode_data.
 *  The filesystem drivers are expected to set the file operations when get_inode is called.
 *
 *  create_inode should return 0 on success
 *  get_inode should return an index node with a type of VFS_ERROR on fail
 *  free_inode_data should free just the private data of the inode that the driver allocated
 *  get_directory should return NULL on fail
 *
 */

int vfs_init();

void vfs_set_driver(VFSDriverOperations driver_operations);

// returns 0 on success
int vfs_create_regular_file(char* path);
int vfs_create_device_file(char* path, VFSFileOperations fops, VFSFileType type);
int vfs_create_device_file_no_checks(char* path, VFSFileOperations fops, VFSFileType type);
int vfs_create_directory(char* path);

VFSDirectory* vfs_open_directory(char* path);
void vfs_close_directory(VFSDirectory* vfs_directory);

// returns NULL on fail
VFSFile* vfs_open_file(char* path);
void vfs_close_file(VFSFile* file);
void vfs_read(VFSFile* file, void* buffer, uint32_t buffer_size);
void vfs_write(VFSFile* file, void* buffer, uint32_t buffer_size);
void vfs_ioctl(VFSFile* file, uint32_t command, uint32_t arg);
void vfs_seek(VFSFile* file, uint32_t offset, uint32_t whence);
uint32_t vfs_tell(VFSFile* file);
void vfs_flush(VFSFile* file);
