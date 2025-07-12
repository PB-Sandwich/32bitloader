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
    uint32_t size;
    uint32_t blocks[13];
    uint32_t reserved;
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
    uint32_t n_inodes;
    uint32_t root_inode;
};

struct FileData {
    uint8_t* data;
    uint32_t range_low;
    uint32_t range_high;
};

struct SuperBlock super_block = { 0 };

#define SINGLE_INDIRECT_INDEX NUM_DIRECT_BLOCKS
#define DOUBLE_INDIRECT_INDEX (NUM_DIRECT_BLOCKS + 1)
#define TRIPLE_INDIRECT_INDEX (NUM_DIRECT_BLOCKS + 2)
#define PTRS_PER_BLOCK (BLOCK_SIZE / sizeof(uint32_t))
#define NUM_DIRECT_BLOCKS 10
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
    harddrive = vfs_open_file(path, 0);
    uint8_t sector[512];
    vfs_seek(harddrive, 0x10000, VFS_BEG);
    vfs_read(harddrive, sector, 512);
    memcpy(&super_block, sector, sizeof(struct SuperBlock));
    return;
};

void* harddrive_load_blocks(void* buffer, uint32_t blocks[13], uint32_t pos, uint32_t num_blocks)
{
    for (uint32_t i = 0; i < num_blocks; i++) {

        uint32_t block_n = (pos + i * BLOCK_SIZE) / BLOCK_SIZE;
        uint32_t real_block = 0xFFFFFFFF;

        if (block_n < NUM_DIRECT_BLOCKS) {
            real_block = blocks[block_n];
        } else if (block_n < NUM_DIRECT_BLOCKS + PTRS_PER_BLOCK) {
            // Single indirect
            uint32_t index = block_n - NUM_DIRECT_BLOCKS;
            uint32_t single_ptrs[PTRS_PER_BLOCK];
            vfs_seek(harddrive, blocks[SINGLE_INDIRECT_INDEX] * BLOCK_SIZE, VFS_BEG);
            vfs_read(harddrive, single_ptrs, BLOCK_SIZE);
            real_block = single_ptrs[index];
        } else if (block_n < NUM_DIRECT_BLOCKS + PTRS_PER_BLOCK + PTRS_PER_BLOCK * PTRS_PER_BLOCK) {
            // Double indirect
            uint32_t index = block_n - NUM_DIRECT_BLOCKS - PTRS_PER_BLOCK;
            uint32_t l1 = index / PTRS_PER_BLOCK;
            uint32_t l2 = index % PTRS_PER_BLOCK;

            uint32_t double_ptrs[PTRS_PER_BLOCK];
            uint32_t single_ptrs[PTRS_PER_BLOCK];

            vfs_seek(harddrive, blocks[DOUBLE_INDIRECT_INDEX] * BLOCK_SIZE, VFS_BEG);
            vfs_read(harddrive, double_ptrs, BLOCK_SIZE);

            vfs_seek(harddrive, double_ptrs[l1] * BLOCK_SIZE, VFS_BEG);
            vfs_read(harddrive, single_ptrs, BLOCK_SIZE);

            real_block = single_ptrs[l2];
        } else {
            // Triple indirect
            uint32_t index = block_n - NUM_DIRECT_BLOCKS - PTRS_PER_BLOCK - PTRS_PER_BLOCK * PTRS_PER_BLOCK;
            uint32_t l1 = index / (PTRS_PER_BLOCK * PTRS_PER_BLOCK);
            uint32_t l2 = (index / PTRS_PER_BLOCK) % PTRS_PER_BLOCK;
            uint32_t l3 = index % PTRS_PER_BLOCK;

            uint32_t triple_ptrs[PTRS_PER_BLOCK];
            uint32_t double_ptrs[PTRS_PER_BLOCK];
            uint32_t single_ptrs[PTRS_PER_BLOCK];

            vfs_seek(harddrive, blocks[TRIPLE_INDIRECT_INDEX] * BLOCK_SIZE, VFS_BEG);
            vfs_read(harddrive, triple_ptrs, BLOCK_SIZE);

            vfs_seek(harddrive, triple_ptrs[l1] * BLOCK_SIZE, VFS_BEG);
            vfs_read(harddrive, double_ptrs, BLOCK_SIZE);

            vfs_seek(harddrive, double_ptrs[l2] * BLOCK_SIZE, VFS_BEG);
            vfs_read(harddrive, single_ptrs, BLOCK_SIZE);

            real_block = single_ptrs[l3];
        }

        if (real_block == 0 || real_block == 0xFFFFFFFF) {
            return NULL;
        }

        void* temp = realloc(buffer, (i * BLOCK_SIZE) + BLOCK_SIZE);
        if (temp == NULL) {
            return buffer;
        }
        buffer = temp;

        vfs_seek(harddrive, real_block * BLOCK_SIZE, VFS_BEG);
        vfs_read(harddrive, buffer + (i * BLOCK_SIZE), BLOCK_SIZE);
    }

    return buffer;
}

void harddrive_write_blocks(void* buffer, uint32_t blocks[13], uint32_t pos, uint32_t num_blocks)
{
    for (uint32_t i = 0; i < num_blocks; i++) {

        uint32_t block_n = (pos + i * BLOCK_SIZE) / BLOCK_SIZE;
        uint32_t real_block = 0xFFFFFFFF;

        if (block_n < NUM_DIRECT_BLOCKS) {
            real_block = blocks[block_n];
        } else if (block_n < NUM_DIRECT_BLOCKS + PTRS_PER_BLOCK) {
            // Single indirect
            uint32_t index = block_n - NUM_DIRECT_BLOCKS;
            uint32_t single_ptrs[PTRS_PER_BLOCK];
            vfs_seek(harddrive, blocks[SINGLE_INDIRECT_INDEX] * BLOCK_SIZE, VFS_BEG);
            vfs_read(harddrive, single_ptrs, BLOCK_SIZE);
            real_block = single_ptrs[index];
        } else if (block_n < NUM_DIRECT_BLOCKS + PTRS_PER_BLOCK + PTRS_PER_BLOCK * PTRS_PER_BLOCK) {
            // Double indirect
            uint32_t index = block_n - NUM_DIRECT_BLOCKS - PTRS_PER_BLOCK;
            uint32_t l1 = index / PTRS_PER_BLOCK;
            uint32_t l2 = index % PTRS_PER_BLOCK;

            uint32_t double_ptrs[PTRS_PER_BLOCK];
            uint32_t single_ptrs[PTRS_PER_BLOCK];

            vfs_seek(harddrive, blocks[DOUBLE_INDIRECT_INDEX] * BLOCK_SIZE, VFS_BEG);
            vfs_read(harddrive, double_ptrs, BLOCK_SIZE);

            vfs_seek(harddrive, double_ptrs[l1] * BLOCK_SIZE, VFS_BEG);
            vfs_read(harddrive, single_ptrs, BLOCK_SIZE);

            real_block = single_ptrs[l2];
        } else {
            // Triple indirect
            uint32_t index = block_n - NUM_DIRECT_BLOCKS - PTRS_PER_BLOCK - PTRS_PER_BLOCK * PTRS_PER_BLOCK;
            uint32_t l1 = index / (PTRS_PER_BLOCK * PTRS_PER_BLOCK);
            uint32_t l2 = (index / PTRS_PER_BLOCK) % PTRS_PER_BLOCK;
            uint32_t l3 = index % PTRS_PER_BLOCK;

            uint32_t triple_ptrs[PTRS_PER_BLOCK];
            uint32_t double_ptrs[PTRS_PER_BLOCK];
            uint32_t single_ptrs[PTRS_PER_BLOCK];

            vfs_seek(harddrive, blocks[TRIPLE_INDIRECT_INDEX] * BLOCK_SIZE, VFS_BEG);
            vfs_read(harddrive, triple_ptrs, BLOCK_SIZE);

            vfs_seek(harddrive, triple_ptrs[l1] * BLOCK_SIZE, VFS_BEG);
            vfs_read(harddrive, double_ptrs, BLOCK_SIZE);

            vfs_seek(harddrive, double_ptrs[l2] * BLOCK_SIZE, VFS_BEG);
            vfs_read(harddrive, single_ptrs, BLOCK_SIZE);

            real_block = single_ptrs[l3];
        }

        if (real_block == 0 || real_block == 0xFFFFFFFF) {
            return;
        }

        void* temp = realloc(buffer, (i * BLOCK_SIZE) + BLOCK_SIZE);
        if (temp == NULL) {
            return;
        }
        buffer = temp;

        vfs_seek(harddrive, real_block * BLOCK_SIZE, VFS_BEG);
        vfs_write(harddrive, buffer + (i * BLOCK_SIZE), BLOCK_SIZE);
    }

    return;
}
#define NUM_DIRECT_BLOCKS 10
#define PTRS_PER_BLOCK (BLOCK_SIZE / sizeof(uint32_t))

// Bitmap sizes
#define BLOCK_BITMAP_SIZE (0xffffffff / BLOCK_SIZE / 8)
#define INODE_BITMAP_SIZE (MAX_INODES / 8)

// Offsets
#define BLOCK_BITMAP_OFFSET (SUPER_BLOCK_OFFSET + BLOCK_SIZE)
#define INODE_BITMAP_OFFSET (BLOCK_BITMAP_OFFSET + BLOCK_BITMAP_SIZE)
#define INODE_TABLE_OFFSET (INODE_BITMAP_OFFSET + INODE_BITMAP_SIZE)

uint8_t* block_bitmap = NULL;

uint32_t alloc_block()
{
    // Load bitmap once
    if (block_bitmap == NULL) {
        block_bitmap = (uint8_t*)malloc(BLOCK_BITMAP_SIZE);
        vfs_seek(harddrive, BLOCK_BITMAP_OFFSET, VFS_BEG);
        vfs_read(harddrive, block_bitmap, BLOCK_BITMAP_SIZE);
    }

    for (uint32_t i = 0; i < BLOCK_BITMAP_SIZE; ++i) {
        if (block_bitmap[i] != 0xFF) {
            for (uint8_t bit = 0; bit < 8; ++bit) {
                if (!(block_bitmap[i] & (1 << bit))) {
                    // Mark as used
                    block_bitmap[i] |= (1 << bit);
                    uint32_t block_number = i * 8 + bit;

                    // Write back just this byte
                    vfs_seek(harddrive, BLOCK_BITMAP_OFFSET + i, VFS_BEG);
                    vfs_write(harddrive, &block_bitmap[i], 1);

                    // Optionally: zero the new block
                    uint8_t zero_block[BLOCK_SIZE] = { 0 };
                    vfs_seek(harddrive, block_number * BLOCK_SIZE, VFS_BEG);
                    vfs_write(harddrive, zero_block, BLOCK_SIZE);

                    return block_number;
                }
            }
        }
    }

    return 0xFFFFFFFF; // Out of space
}

VFSFile* fs_open(VFSIndexNode* inode)
{
    VFSFile* file = (VFSFile*)malloc(sizeof(VFSFile));
    if (file == NULL) {
        return NULL;
    }
    file->inode = inode;
    file->private_data = malloc(sizeof(struct FileData));
    file->private_data_size = 0;
    file->position = 0;
    if (file->private_data == NULL) {
        free(file);
        return NULL;
    }
    struct FileData* fd = file->private_data;
    fd->range_low = 0;
    fd->range_high = 0;
    fd->data = malloc(BLOCK_SIZE);
    if (fd->data == NULL) {
        free(file->private_data);
        free(file);
        return NULL;
    }
    return file;
}

void fs_close(VFSFile* file)
{
    if (file->private_data != NULL) {
        free(file->private_data);
    }
    free(file);
}

uint32_t fs_read(VFSFile* file, void* buffer, uint32_t buffer_size)
{
    if (buffer_size + file->position > file->inode->size) {
        buffer_size = file->inode->size - file->position;
    }
    if (file->position == file->inode->size) {
        return 0;
    }

    struct FileData* fd = file->private_data;

    if (file->position + buffer_size > fd->range_high || file->position < fd->range_low || fd->range_high == fd->range_low) {
        fd->data = harddrive_load_blocks(fd->data, file->inode->private_data, file->position, 5);
        if (fd->data == NULL) {
            return 0;
        }

        fd->range_low = (file->position / BLOCK_SIZE) * BLOCK_SIZE;
        fd->range_high = fd->range_low + (5 * BLOCK_SIZE);
    }

    uint32_t i;
    for (i = 0; i < buffer_size; i++) {
        ((uint8_t*)buffer)[i] = fd->data[i + file->position % (BLOCK_SIZE * 5)];
    }
    file->position += i;
    return i;
}

uint32_t fs_write(VFSFile* file, void* buffer, uint32_t buffer_size)
{
    struct FileData* fd = file->private_data;
    if (file->position + buffer_size > fd->range_high || file->position < fd->range_low || fd->range_high == fd->range_low) {
        fd->data = harddrive_load_blocks(fd->data, file->inode->private_data, file->position, 5);
        if (fd->data == NULL) {
            return 0;
        }

        fd->range_low = file->position;
        fd->range_high = fd->range_low + (5 * BLOCK_SIZE);
    }

    uint32_t i;
    for (i = 0; i < buffer_size; i++) {
        fd->data[i + file->position % (BLOCK_SIZE * 5)] = ((uint8_t*)buffer)[i];
    }

    return i;
}

void fs_ioctl(VFSFile* file, uint32_t command, uint32_t arg) { }
void fs_seek(VFSFile* file, uint32_t offset, uint32_t whence)
{
    switch (whence) {
    case VFS_BEG:
        if (offset < file->inode->size - 1) {
            file->position = offset;
        } else {
            file->position = file->inode->size - 1;
        }
        break;
    case VFS_CUR:
        if (offset + file->position < file->inode->size - 1) {
            file->position += offset;
        } else {
            file->position += file->inode->size - 1;
        }
        break;
    case VFS_END:
        if (file->inode->size - 1 - offset > 0) {
            file->position = file->inode->size - offset;
        } else {
            file->position = 0;
        }
        break;
    }
}
uint32_t fs_tell(VFSFile* file)
{
    return file->position;
}
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

int create_inode(char* path, char* name, VFSFileType type)
{
    struct Inode inode = resolve_inode(path);

    if (inode.type == 0) {
        return 1;
    }

    struct DirectoryEntry* entry = malloc(sizeof(struct DirectoryEntry) + strlen(name));

    entry->inode_number = super_block.n_inodes;
    entry->entry_length = sizeof(struct DirectoryEntry) + strlen(name);
    entry->name_length = strlen(name);
    strncpy(entry->name, name, strlen(name));

    for (uint32_t i = 0; i < entry->entry_length; i++) {
        // harddrive_write_byte(inode.blocks, inode.size, ((uint8_t*)entry)[i]);
    }

    inode.size += entry->entry_length;

    struct Inode new_inode = {
        .type = type,
        .size = 0,
        .blocks = { 0xffffffff },
    };

    uint32_t inode_number = entry->inode_number;

    uint32_t inode_offset = ((SUPER_BLOCK_OFFSET + BLOCK_SIZE + BLOCK_BP_SIZE + INODE_BP_SIZE) / BLOCK_SIZE + 1) * BLOCK_SIZE
        + (inode_number * INODE_SIZE);

    uint8_t buffer[512];

    vfs_seek(harddrive, inode_offset, VFS_BEG);
    vfs_read(harddrive, buffer, 512);

    uint32_t sub_offset = inode_number % 8;
    memcpy(&buffer[sub_offset * INODE_SIZE], &new_inode, INODE_SIZE);

    vfs_seek(harddrive, inode_offset, VFS_BEG);
    vfs_write(harddrive, buffer, 512);

    return 0;
}

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
