#include "estros-fs.h"
#include <filesystem/virtual-filesystem.h>
#include <harddrive/hdd.h>
#include <heap.h>
#include <memutils.h>

// typedef enum {
//     VFS_BEG = 1,
//     VFS_CuR = 2,
//     VFS_END = 3,
// } VFSWhence;
//
// typedef enum {
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
//     VFSIndexNode* inode;
//     void* vfs_directory;
//     char* name;
// } VFSDirectoryEntry;
//
// typedef struct {
//     VFSDirectoryEntry* entries;
//     uint32_t entries_length;
// } VFSDirectory;
//
// typedef struct {
//     int (*create_inode)(char* path, VFSFileType type);
//     VFSIndexNode* (*get_inode)(char* path);
//     VFSDirectory* (*get_directory_entries)(char* path);
//     int (*write_directory_entries)(VFSDirectory vfs_directory);
// } VFSDriverOperations;

typedef struct {
    uint32_t blocks[14];
} IndexNode;

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
    uint32_t size;
    uint32_t blocks[14];
};

struct DirectoryEntry {
    uint32_t inode_number;
    uint32_t entry_length;
    uint32_t name_length;
    char name[];
};

struct SuperBlock {
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
    inode->number_of_references++;
    return file;
}

void fs_close(VFSFile* file)
{
    if (file->private_data != NULL) {
        free(file->private_data);
    }
    file->inode->number_of_references--;
    free(file);
}

void fs_read(VFSFile* file, void* buffer, uint32_t buffer_size)
{
    uint8_t block[BLOCK_SIZE];

    vfs_seek(harddrive, (((uint32_t*)file->inode->private_data)[0]) * BLOCK_SIZE, VFS_BEG);

    vfs_read(harddrive, block, BLOCK_SIZE);
    memcpy(buffer, block, buffer_size);

    file->position += buffer_size;
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
    struct DirectoryEntry* entries = (struct DirectoryEntry*)malloc((inode.size / 512 + 1) * 512);
    if (entries == NULL) {
        return NULL;
    }
    memset(entries, 0, (inode.size / 512 + 1) * 512);
    vfs_seek(harddrive, inode.blocks[0] * BLOCK_SIZE, VFS_BEG);
    vfs_read(harddrive, entries, (inode.size / 512 + 1) * 512);
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
                path[i] = '\0';
                break;
            }
        }

        if (offset != 0) {
            if (inode.type != DIRECTORY) {
                free(base_entry);
                return err;
            }
        }

        while (strncmp(path, entry->name, entry->name_length) != 0) {
            if (entry->entry_length == 0) {
                free(base_entry);
                return err;
            }
            entry = (struct DirectoryEntry*)((uint8_t*)entry + entry->entry_length);
        }
        inode = fetch_inode(entry->inode_number);
        path[offset] = '/';
        path += offset + 1;

        if (offset == 0) { // found
            free(base_entry);
            break;
        }

        free(base_entry);
    }
    return inode;
}

VFSIndexNode* fstovfs(struct Inode inode)
{
    VFSIndexNode* vfs_inode = (VFSIndexNode*)malloc(sizeof(VFSIndexNode));
    if (vfs_inode == NULL) {
        return NULL;
    }

    vfs_inode->type = inode.type;
    vfs_inode->size = inode.size;
    vfs_inode->private_data = (uint32_t*)malloc(sizeof(uint32_t) * 14);
    vfs_inode->file_operations = get_fs_file_operations();
    if (vfs_inode->private_data == NULL) {
        free(vfs_inode);
        return NULL;
    }
    memcpy(vfs_inode->private_data, inode.blocks, sizeof(uint32_t) * 14);
    return vfs_inode;
}

int create_inode(char* path, VFSFileType type) { }
VFSIndexNode* get_inode(char* path)
{
    struct Inode inode = resolve_inode(path);
    if (inode.type == 0) {
        return NULL;
    }

    VFSIndexNode* vfs_inode = fstovfs(inode);

    return vfs_inode;
}

VFSDirectory* get_directory_entries(char* path)
{
    struct Inode inode = resolve_inode(path);
    if (inode.type == 0) {
        return NULL;
    }

    struct DirectoryEntry* base_entry = fetch_inode_directory(inode);
    if (base_entry == NULL) {
        return NULL;
    }
    struct DirectoryEntry* entry = base_entry;

    VFSDirectory* vfs_dir = (VFSDirectory*)malloc(sizeof(VFSDirectory));
    if (vfs_dir == NULL) {
        return NULL;
    }

    vfs_dir->entries = (VFSDirectoryEntry*)malloc(sizeof(VFSDirectoryEntry));
    if (vfs_dir->entries == NULL) {
        free(vfs_dir);
        return NULL;
    }
    vfs_dir->entries_length = 0;

    while (entry->entry_length != 0) {
        VFSDirectoryEntry vfs_entry = {
            .inode = fstovfs(fetch_inode(entry->inode_number)),
            .name = (char*)malloc(entry->name_length + 1),
            .vfs_directory = NULL,
        };
        strncpy(vfs_entry.name, entry->name, entry->name_length);
        vfs_entry.name[entry->name_length] = '\0';

        vfs_dir->entries = (VFSDirectoryEntry*)realloc(vfs_dir->entries, sizeof(VFSDirectoryEntry) * (vfs_dir->entries_length + 1));
        if (vfs_dir->entries == NULL) {
            free(vfs_entry.inode);
            free(vfs_entry.name);
            free(vfs_dir);
            return NULL;
        }
        vfs_dir->entries[vfs_dir->entries_length] = vfs_entry;
        vfs_dir->entries_length++;
        entry = (struct DirectoryEntry*)((uint8_t*)entry + entry->entry_length);
    }
    return vfs_dir;
}

int write_directory_entries(VFSDirectory vfs_directory) { }

VFSDriverOperations get_fs_driver_operations()
{
    VFSDriverOperations dops = {
        .create_inode = create_inode,
        .get_inode = get_inode,
        .write_directory_entries = write_directory_entries,
        .get_directory_entries = get_directory_entries,
    };
    return dops;
}
