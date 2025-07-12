#include "virtual-filesystem.h"
#include <hashmap/hashmap.h>
#include <heap.h>
#include <memutils.h>
#include <print.h>

void* null_function()
{
    return NULL;
}

VFSDriverOperations dops = {
    .create_inode = (void*)null_function,
    .get_inode = (void*)null_function,
    .free_inode_data = (void*)null_function,
    .get_directory = (void*)null_function,
};

VFSIndexNode* inodes = NULL;
uint32_t inodes_size = 0;
struct hashmap_s hashmap;

int vfs_init()
{
    if (hashmap_create(16, &hashmap) != 0) {
        return 1;
    }
    inodes = (VFSIndexNode*)malloc(sizeof(VFSIndexNode));
    if (inodes == NULL) {
        hashmap_destroy(&hashmap);
        return 1;
    }
    return 0;
}

void vfs_set_driver(VFSDriverOperations driver_operations)
{
    dops = driver_operations;
    return;
}

VFSIndexNode* insert_new_inode(VFSIndexNode inode, char* path)
{
    void* temp = realloc(inodes, sizeof(VFSIndexNode) * (inodes_size + 1));
    if (temp == NULL) {
        return NULL;
    }
    inodes = (VFSIndexNode*)temp;
    inodes[inodes_size] = inode;

    if (hashmap_put(&hashmap, path, strlen(path), &inodes[inodes_size]) != 0) {
        temp = realloc(inodes, sizeof(VFSIndexNode) * inodes_size);
        if (temp == NULL) {
            return NULL;
        }
        inodes = (VFSIndexNode*)temp;
        if (inode.type == VFS_REGULAR_FILE) {
            dops.free_inode_data(inode);
        }
        return NULL;
    }
    inodes_size++;
    return &inodes[inodes_size - 1];
}

void free_directory(VFSDirectory* dir)
{
    for (uint32_t i = 0; i < dir->entries_length; i++) {
        if (dir->entries[i].path != NULL) {
            free(dir->entries[i].path);
        }
    }
    if (dir->entries != NULL) {
        free(dir->entries);
    }
    free(dir);
    dir = NULL;
    return;
}

// returns 0 on success
int vfs_create_regular_file(char* path)
{
    if (path[0] != '/') {
        return 1; // not root path
    }
    if (path[strlen(path) - 1] == '/') {
        return 1; // directory
    }

    if (hashmap_get(&hashmap, path, strlen(path)) != NULL) {
        return 1; // already exists in virtual filesystem
    };

    uint32_t offset = 0;
    for (int i = strlen(path); i > 0; i--) {
        if (path[i] == '/') {
            path[i] = '\0';
            offset = i;
        }
    }
    char* name = path + offset + 1;

    return dops.create_inode(path, name, VFS_REGULAR_FILE); // will check if it already exists

    path[offset] = '/';
}

int vfs_create_device_file(char* path, VFSFileOperations fops, VFSFileType type)
{
    if (path[0] != '/') {
        return 1; // not root path
    }
    if (path[strlen(path) - 1] == '/') {
        return 1; // directory
    }

    if (hashmap_get(&hashmap, path, strlen(path)) != NULL) {
        return 1; // already exists
    }

    VFSIndexNode physical_inode = dops.get_inode(path);
    if (physical_inode.type == VFS_ERROR) {
        dops.free_inode_data(physical_inode);
        return 1; // directory does not exist
    }

    return vfs_create_device_file_no_checks(path, fops, type);
}

int vfs_create_device_file_no_checks(char* path, VFSFileOperations fops, VFSFileType type)
{
    VFSIndexNode inode = {
        .type = type,
        .size = 0,
        .file_operations = fops,
        .private_data = NULL,
        .number_of_references = 0,
    };
    if (insert_new_inode(inode, path) == NULL) {
        return 1;
    }
    return 0;
}

int vfs_create_directory(char* path);

struct hash_dir_iter_t {
    char* path;
    VFSDirectory* dir;
};

int hash_dir_iter(void* const context, struct hashmap_element_s* const e)
{
    struct hash_dir_iter_t* iter = (struct hash_dir_iter_t*)context;
    char* path = iter->path;
    VFSDirectory* dir = iter->dir;

    if (strncmp((char*)e->key, path, strlen(path)) == 0) {

        char* rest = ((char*)e->key) + strlen(path);

        if (*rest == '\0') {
            return 0;
        }

        if (strchr(rest, '/') != NULL) {
            return 0;
        }

        if (rest[0] != '/') {
            return 0;
        }

        for (uint32_t i = 0; i < dir->entries_length; i++) {
            if (strcmp((char*)e->key, dir->entries[i].path) == 0) {
                return 0;
            }
        }

        void* temp = realloc(dir->entries, sizeof(VFSDirectoryEntry) * (dir->entries_length + 1));
        if (temp == NULL) {
            return 1;
        }
        dir->entries = (VFSDirectoryEntry*)temp;

        char* copied_path = (char*)malloc(e->key_len + 1);
        if (copied_path == NULL) {
            free_directory(dir);
            return 1;
        }

        strncpy(copied_path, e->key, e->key_len);
        copied_path[e->key_len] = '\0';

        dir->entries[dir->entries_length].path = copied_path;
        dir->entries_length++;
    }

    return 0;
}

VFSDirectory* vfs_open_directory(char* path)
{
    VFSIndexNode* virtual_inode = hashmap_get(&hashmap, path, strlen(path));

    if (virtual_inode == NULL) {
        VFSIndexNode physical_inode = dops.get_inode(path);
        if (physical_inode.type == VFS_ERROR) {
            dops.free_inode_data(physical_inode);
            return NULL;
        }
        virtual_inode = insert_new_inode(physical_inode, path);
    }

    VFSDirectory* dir = dops.get_directory(path, virtual_inode);
    if (dir == NULL) {
        return NULL;
    }
    dir->inode->number_of_references++;
    struct hash_dir_iter_t iter_value = {
        .path = path,
        .dir = dir,
    };
    hashmap_iterate_pairs(&hashmap, hash_dir_iter, &iter_value);
    if (dir == NULL) {
        return NULL;
    }
    return dir;
}

void vfs_close_directory(VFSDirectory* vfs_directory)
{
    vfs_directory->inode->number_of_references--;
    free_directory(vfs_directory);
}

// returns NULL on fail
VFSFile* vfs_open_file(char* path, VFSFileFlags flags)
{
    VFSIndexNode* virtual_inode = hashmap_get(&hashmap, path, strlen(path));

    if (virtual_inode == NULL) {
        VFSIndexNode physical_inode = dops.get_inode(path);
        if (physical_inode.type == VFS_ERROR) {
            dops.free_inode_data(physical_inode);
            return NULL;
        }
        virtual_inode = insert_new_inode(physical_inode, path);
    }
    VFSFile* file = virtual_inode->file_operations.open(virtual_inode, flags);
    virtual_inode->number_of_references++;
    return file;
}

void vfs_close_file(VFSFile* file)
{
    file->inode->number_of_references--;
    file->inode->file_operations.close(file);
}

uint32_t vfs_read(VFSFile* file, void* buffer, uint32_t buffer_size)
{
    return file->inode->file_operations.read(file, buffer, buffer_size);
}

uint32_t vfs_write(VFSFile* file, void* buffer, uint32_t buffer_size)
{
    return file->inode->file_operations.write(file, buffer, buffer_size);
}

void vfs_ioctl(VFSFile* file, uint32_t command, uint32_t arg)
{
    file->inode->file_operations.ioctl(file, command, arg);
}

void vfs_seek(VFSFile* file, uint32_t offset, uint32_t whence)
{
    file->inode->file_operations.seek(file, offset, whence);
}

uint32_t vfs_tell(VFSFile* file)
{
    return file->inode->file_operations.tell(file);
}

void vfs_flush(VFSFile* file)
{
    file->inode->file_operations.flush(file);
}
