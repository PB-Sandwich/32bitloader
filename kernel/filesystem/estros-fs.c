#include "estros-fs.h"
#include <filesystem/virtual-filesystem.h>
#include <harddrive/hdd.h>
#include <heap.h>
#include <memutils.h>
#include <stdlib.h>

// typedef enum {
//     VFS_BEG = 1,
//     VFS_CUR = 2,
//     VFS_END = 3,
// } VFSWhence;
//
// typedef enum {
//     VFS_ERROR = 0,
//     VFS_REGULAR_FILE = 1,
//     VFS_BLOCK_DEVICE = 2,
//     VFS_CHARACTER_DEVICE = 3,
// } VFSFileType;
//
// typedef struct {
//     void* (*open)(void* inode); // returns VFSFile
//     void (*close)(void* file);
//     void (*read)(void* file, void* buffer, uint32_t buffer_size);
//     void (*write)(void* file, void* buffer, uint32_t buffer_size);
//     void (*ioctl)(void* file, uint32_t command, uint32_t arg);
//     void (*seek)(void* file, uint32_t offset, uint32_t whence);
//     uint32_t (*tell)(void* file);
//     void (*flush)(void* file);
// } VFSFileOperations;
//
// typedef struct {
//     uint32_t type;
//     uint32_t size;
//     VFSFileOperations file_operations;
//     void* private_data;
//     uint32_t number_of_references;
// } VFSIndexNode;
//
// typedef struct {
//     VFSIndexNode* inode;
//     void* private_data;
//     uint32_t private_data_size;
//     uint32_t position;
// } VFSFile;
//
// typedef struct {
//     char* path;
// } VFSDirectoryEntry;
//
// typedef struct {
//     VFSIndexNode inode;
//     VFSDirectoryEntry* entries;
//     uint32_t entries_length;
// } VFSDirectory;
//
// typedef struct {
//     int (*create_inode)(char* path, VFSFileType type);
//     VFSIndexNode (*get_inode)(char* path);
//     VFSDirectory* (*get_directory)(char* path);
//     void (*free_inode_data)(VFSIndexNode inode);
// } VFSDriverOperations;

enum BitMap {
    FREE = 0,
    USED = 1,
};

enum InodeType {
    FILE = 1,
    DIRECTORY = 2
};

struct Inode {
    uint32_t type;
    uint64_t size;
    uint32_t blocks[13];
};

struct DirectoryEntry {
    uint32_t inode_number;
    uint32_t entry_length;
    uint32_t name_length;
    char name[];
};

struct SuperBlock {
    uint64_t size_bytes;
    uint32_t n_blocks;
    uint32_t max_inodes;
    uint32_t root_inode;
};

struct SuperBlock super_block = { 0 };

#define BLOCK_SIZE 1024
#define INODE_SIZE 64
#define MAX_INODES 8192
#define INODE_SPACE (MAX_INODES * INODE_SIZE)
#define SUPER_BLOCK_OFFSET 0x10000
#define BLOCK_BP_SIZE (0xffffffff / BLOCK_SIZE / 8)
#define INODE_BP_SIZE (MAX_INODES * INODE_SIZE / BLOCK_SIZE / 8)
#define FIRST_DATA_BLOCK (BLOCK_SIZE + BLOCK_BP_SIZE + INODE_BP_SIZE + INODE_SIZE) / BLOCK_SIZE + 1)

VFSFile* harddrive = NULL;
uint32_t loaded_block = 0;
uint8_t* block_data = NULL;

uint8_t harddrive_read_byte(uint32_t blocks[13], uint32_t pos)
{
    if (block_data == NULL) {
        block_data = (uint8_t*)malloc(BLOCK_SIZE);
        if (block_data == NULL) {
            return -1;
        }
    }

    uint32_t block_n = pos / BLOCK_SIZE;
    if (block_n > 13) {
        return -1;
    }

    if (blocks[block_n] != loaded_block) {
        vfs_seek(harddrive, blocks[block_n] * BLOCK_SIZE, VFS_BEG);
        vfs_read(harddrive, block_data, BLOCK_SIZE);
        loaded_block = blocks[block_n];
    }
    pos = pos - block_n * BLOCK_SIZE;
    return block_data[pos];
}

void fs_set_harddrive(char* path)
{
    harddrive = vfs_open_file(path);
    uint8_t sector[512];
    vfs_seek(harddrive, 0x10000, VFS_BEG);
    vfs_read(harddrive, sector, 512);
    memcpy(&super_block, sector, sizeof(struct SuperBlock));
    return;
};

VFSFile* fs_open(VFSIndexNode* inode)
{
    VFSFile* file = (VFSFile*)malloc(sizeof(VFSFile));
    if (file == NULL) {
        return NULL;
    }
    file->inode = inode;
    file->private_data = NULL;
    file->private_data_size = 0;
    file->position = 0;
    return file;
}

void fs_close(VFSFile* file)
{
    if (file->private_data != NULL) {
        free(file->private_data);
    }
    free(file);
}

void fs_read(VFSFile* file, void* buffer, uint32_t buffer_size)
{
    if (buffer_size + file->position > file->inode->size) {
        buffer_size = file->inode->size - file->position;
    }
    for (uint32_t i = 0; i < buffer_size; i++) {
        ((uint8_t*)buffer)[i] = harddrive_read_byte(file->inode->private_data, file->position++);
    }
    return;
}
void fs_write(VFSFile* file, void* buffer, uint32_t buffer_size) { }
void fs_ioctl(VFSFile* file, uint32_t command, uint32_t arg) { }
void fs_seek(VFSFile* file, uint32_t offset, uint32_t whence) { }
uint32_t fs_tell(VFSFile* file) { }
void fs_flush(VFSFile* file) { }

VFSFileOperations get_fs_file_operations()
{
    VFSFileOperations fops = {
        .open = (void*)fs_open,
        .close = (void*)fs_close,
        .read = (void*)fs_read,
        .write = (void*)fs_write,
        .ioctl = (void*)fs_ioctl,
        .seek = (void*)fs_seek,
        .tell = (void*)fs_tell,
        .flush = (void*)fs_flush,
    };
    return fops;
}

struct Inode fetch_inode(uint32_t inode_number)
{
    uint32_t inode_offset = ((SUPER_BLOCK_OFFSET + BLOCK_SIZE + BLOCK_BP_SIZE + INODE_BP_SIZE) / BLOCK_SIZE + 1) * BLOCK_SIZE
        + (inode_number * INODE_SIZE);

    uint8_t buffer[512];
    vfs_seek(harddrive, inode_offset, VFS_BEG);
    vfs_read(harddrive, buffer, 512);
    uint32_t sub_offset = inode_number % 8;
    struct Inode inode;
    memcpy(&inode, &buffer[sub_offset * INODE_SIZE], INODE_SIZE);
    return inode;
}

struct DirectoryEntry* fetch_inode_directory(struct Inode inode)
{
    if (inode.size == 0) {
        return NULL;
    }
    struct DirectoryEntry* entries = (struct DirectoryEntry*)malloc(inode.size + sizeof(struct DirectoryEntry));
    if (entries == NULL) {
        return NULL;
    }
    memset(entries, 0, inode.size + sizeof(struct DirectoryEntry));
    for (uint32_t i = 0; i < inode.size; i++) {
        ((uint8_t*)entries)[i] = harddrive_read_byte(inode.blocks, i);
    }
    return entries;
}

struct Inode resolve_inode(char* path)
{
    struct Inode err = { 0 };
    while (*path == '/') {
        path++;
    }
    struct Inode inode = fetch_inode(super_block.root_inode);
    if (strlen(path) == 0) {
        return inode;
    }
    while (1) {
        struct DirectoryEntry* base_entry = fetch_inode_directory(inode);
        if (base_entry == NULL) {
            return err;
        }
        struct DirectoryEntry* entry = base_entry;

        uint32_t offset = 0;
        for (int i = 0; i < strlen(path); i++) {
            if (path[i] == '/') {
                offset = i;
                if (offset != 0) {
                    path[i] = '\0';
                }
                break;
            }
        }

        if (offset != 0) {
            if (inode.type != DIRECTORY) {
                free(base_entry);
                return err;
            }
        }

        while (strncmp(path, entry->name, entry->name_length) != 0 || strlen(path) > entry->name_length) {
            if (entry->entry_length == 0) {
                free(base_entry);
                return err;
            }
            entry = (struct DirectoryEntry*)((uint8_t*)entry + entry->entry_length);
        }
        inode = fetch_inode(entry->inode_number);
        if (offset != 0) {
            path[offset] = '/';
        }
        path += offset + 1;

        if (offset == 0) { // found
            free(base_entry);
            break;
        }

        free(base_entry);
    }
    return inode;
}

VFSIndexNode fstovfs(struct Inode inode)
{
    VFSIndexNode vfs_inode = {
        .type = inode.type,
        .size = inode.size,
        .private_data = (uint32_t*)malloc(sizeof(uint32_t) * 13),
        .file_operations = get_fs_file_operations(),
        .number_of_references = 0,
    };

    if (vfs_inode.private_data == NULL) {
        vfs_inode.type = VFS_ERROR;
        return vfs_inode;
    }
    memcpy(vfs_inode.private_data, inode.blocks, sizeof(uint32_t) * 13);
    return vfs_inode;
}

struct Inode vfstofs(VFSIndexNode* vfs_inode)
{
    struct Inode inode = {
        .type = vfs_inode->type,
        .size = vfs_inode->size,
    };
    memcpy(inode.blocks, vfs_inode->private_data, sizeof(uint32_t) * 13);
    return inode;
}

int create_inode(char* path, VFSFileType type) { }
VFSIndexNode get_inode(char* path)
{
    struct Inode inode = resolve_inode(path);
    if (inode.type == 0) {
        VFSIndexNode vfs_inode = { 0 };
        vfs_inode.type = VFS_ERROR;
        return vfs_inode;
    }

    VFSIndexNode vfs_inode = fstovfs(inode);

    return vfs_inode;
}

VFSDirectory* get_directory(char* path, VFSIndexNode* inode)
{
    if (inode->type != VFS_DIRECTORY) {
        return NULL;
    }

    VFSDirectory* vfs_dir = (VFSDirectory*)malloc(sizeof(VFSDirectory));
    if (vfs_dir == NULL) {
        return NULL;
    }
    vfs_dir->entries_length = 0;
    vfs_dir->entries = NULL;
    vfs_dir->inode = inode;

    struct DirectoryEntry* base_entry = fetch_inode_directory(vfstofs(inode));
    if (base_entry == NULL) {
        return vfs_dir;
    }
    struct DirectoryEntry* entry = base_entry;

    vfs_dir->entries = (VFSDirectoryEntry*)malloc(sizeof(VFSDirectoryEntry));
    if (vfs_dir->entries == NULL) {
        free(vfs_dir);
        free(base_entry);
        return NULL;
    }

    while (entry->entry_length != 0) {
        VFSDirectoryEntry vfs_entry;
        if (strlen(path) == 1) {
            vfs_entry.path = (char*)malloc(strlen(path) + entry->name_length + 1);
            strncpy(vfs_entry.path, path, strlen(path));
            strncpy(vfs_entry.path + strlen(path), entry->name, entry->name_length);
            vfs_entry.path[strlen(path) + entry->name_length] = '\0';
        } else {
            vfs_entry.path = (char*)malloc(strlen(path) + 1 + entry->name_length + 1);
            strncpy(vfs_entry.path, path, strlen(path));
            vfs_entry.path[strlen(path)] = '/';
            strncpy(vfs_entry.path + strlen(path) + 1, entry->name, entry->name_length);
            vfs_entry.path[strlen(path) + entry->name_length + 1] = '\0';
        }

        void* temp = (VFSDirectoryEntry*)realloc(vfs_dir->entries, sizeof(VFSDirectoryEntry) * (vfs_dir->entries_length + 1));
        if (temp == NULL) {
            free(temp);
            free(vfs_entry.path);
            free(vfs_dir->entries);
            free(vfs_dir);
            return NULL;
        }
        vfs_dir->entries = temp;
        vfs_dir->entries[vfs_dir->entries_length] = vfs_entry;
        vfs_dir->entries_length++;
        entry = (struct DirectoryEntry*)((uint8_t*)entry + entry->entry_length);
    }
    free(base_entry);
    return vfs_dir;
}

void free_inode_data(VFSIndexNode inode)
{
    free(inode.private_data);
    return;
}

VFSDriverOperations get_fs_driver_operations()
{
    VFSDriverOperations dops = {
        .create_inode = create_inode,
        .get_inode = get_inode,
        .get_directory = get_directory,
        .free_inode_data = free_inode_data,
    };
    return dops;
}
