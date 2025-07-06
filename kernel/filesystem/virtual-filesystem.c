#include "virtual-filesystem.h"
#include <cstdint>
#include <hashmap/hashmap.h>
#include <heap.h>
#include <memutils.h>
#include <print.h>

void* null_function()
{
    return NULL;
}

VFSDriverOperations dops = {
    .get_directory_entries = (void*)null_function,
    .get_inode = (void*)null_function,
    .write_directory_entries = (void*)null_function,
    .create_inode = (void*)null_function
};
VFSIndexNode* inodes = NULL;
uint32_t inodes_size = 0;
struct hashmap_s hashmap;

int vfs_init()
{
    if (hashmap_create(2, &hashmap) != 0) {
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

struct virtual_inode_ret {
    char* path;
    VFSIndexNode* ptr;
};

int get_virtual_inode(void* const context, struct hashmap_element_s* const e)
{
    if (strcmp(e->key, ((struct virtual_inode_ret*)context)->path) == 0) {
        ((struct virtual_inode_ret*)context)->ptr = e->data;
        return 0;
    }
    return 1;
}

void free_directory(VFSDirectory* dir)
{
    for (uint32_t i = 0; i < dir->entries_length; i++) {
        if (dir->entries[i].inode != NULL) {
            free(dir->entries[i].inode);
        }
        if (dir->entries[i].name != NULL) {
            free(dir->entries[i].name);
        }
        if (dir->entries->vfs_directory != NULL) {
            free_directory(dir->entries->vfs_directory);
        }
    }
    if (dir->entries != NULL) {
        free(dir->entries);
    }
    free(dir);
}

// loads directories if they're not
VFSDirectory* load_directories(char* path)
{
    char* name = (char*)malloc(strlen(path));
    char* cur_path = (char*)malloc(strlen(path));
    uint32_t offset = 0;
    while (path[offset] == '/') {
        offset++;
    }
    VFSDirectory* dir = &root;
    while (strchr(path + offset, '/') - path + offset) {
        if (strchr(path + offset, '/') - path + offset == strlen(path + offset)) {
            break;
        }
        strncpy(name, path + offset, strchr(path + offset, '/') - path + offset - 1);
        name[strchr(path, '/') - path] = '\0';
        strncpy(cur_path, path, offset - strlen(name));

        uint8_t found_dir = 0;
        for (uint32_t i = 0; i < dir->entries_length; i++) {
            if (strcmp(name, dir->entries[i].name) != 0) {
                continue;
            }
            if (dir->entries[i].vfs_directory == NULL) {
                continue;
            }
            dir = dir->entries[i].vfs_directory;
            found_dir = 1;
        }

        if (found_dir == 0) {
            VFSDirectory* subdir = dops.get_directory_entries(cur_path);
            if (subdir == NULL) {
                free(name);
                free(cur_path);
                return NULL;
            }
            VFSIndexNode* subdir_inode = dops.get_inode(cur_path);
            if (subdir_inode == NULL) {
                free(name);
                free(cur_path);
                return NULL;
            }

            dir->entries_length++;
            if (dir->entries != NULL) {
                dir->entries = (VFSDirectoryEntry*)realloc(dir->entries, sizeof(VFSDirectoryEntry) * dir->entries_length);
            } else {
                dir->entries = (VFSDirectoryEntry*)malloc(sizeof(VFSDirectoryEntry) * dir->entries_length);
            }
            if (dir->entries == NULL) {
                free(name);
                free(cur_path);
                return NULL;
            }
            dir->entries[dir->entries_length - 1].vfs_directory = subdir;
            dir->entries[dir->entries_length - 1].inode = subdir_inode;
            if (strlen(name) != 0) {
                dir->entries[dir->entries_length - 1].name = (char*)malloc(strlen(name));
            } else {
                dir->entries[dir->entries_length - 1].name = (char*)malloc(1);
            }
            if (dir->entries[dir->entries_length - 1].name == NULL) {
                free(name);
                free(cur_path);
                return NULL;
            }
            if (strlen(name) != 0) {
                strcpy(dir->entries[dir->entries_length - 1].name, name);
            } else {
                *dir->entries[dir->entries_length - 1].name = '\0';
            }
            dir = subdir;
        }
    }
    free(cur_path);
    free(name);
    return dir;
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

    if (get_virtual_inode(path, &root) != NULL) {
        return 1; // already exists in virtual filesystem
    };
    dops.create_inode(path, VFS_REGULAR_FILE); // will check if it already exists

    return 0;
}

int vfs_create_device_file(char* path, VFSFileOperations fops, VFSFileType type)
{
    if (path[0] != '/') {
        return 1; // not root path
    }
    if (path[strlen(path) - 1] == '/') {
        return 1; // directory
    }

    if (get_virtual_inode(path, &root) != NULL) {
        return 1; // already exists in virtual filesystem
    };
    VFSIndexNode* inode = dops.get_inode(path);
    if (inode != NULL) {
        return 1; // already exists in physical filesystem
    }
    free(inode);

    uint32_t offset = 0;
    for (uint32_t i = strlen(path) - 1; i >= 0; i--) {
        if (path[i] == '/') {
            path[i] = '\0';
            offset = i;
            break;
        }
    }

    VFSDirectory* directory = load_directories(path);
    if (directory == NULL) {
        return 1; // directory does not exist
    }
    path[offset] = '/';

    directory->entries_length++;
    if (directory->entries != NULL) {
        directory->entries = (VFSDirectoryEntry*)realloc(directory->entries, sizeof(VFSDirectoryEntry) * directory->entries_length);
    } else {
        directory->entries = (VFSDirectoryEntry*)malloc(sizeof(VFSDirectoryEntry) * directory->entries_length);
    }
    if (directory->entries == NULL) {
        return 1;
    }

    path += offset + 1;
    uint32_t name_length = strlen(path);

    VFSDirectoryEntry entry = {
        .inode = (VFSIndexNode*)malloc(sizeof(VFSIndexNode)),
        .vfs_directory = NULL,
        .name = (char*)malloc(name_length + 1),
    };
    entry.inode->type = type;
    entry.inode->file_operations = fops;
    strncpy(entry.name, path, name_length + 1);
    entry.name[name_length] = '\0';

    directory->entries[directory->entries_length - 1] = entry;

    return 0;
}

int vfs_create_directory(char* path);

VFSDirectory* vfs_open_directory(char* path)
{
    VFSDirectory* dir = load_directories(path);
    return dir;
}

void vfs_close_directory(VFSDirectory* vfs_directory)
{
    free_directory(vfs_directory);
}

// returns NULL on fail
VFSFile* vfs_open_file(char* path)
{
    VFSIndexNode* inode = get_virtual_inode(path, &root);
    if (inode == NULL) {
        inode = dops.get_inode(path);
        if (inode == NULL) {
            return NULL;
        }
    }
    VFSFile* file = inode->file_operations.open(inode);
    return file;
}

void vfs_close_file(VFSFile* file)
{
    file->inode->file_operations.close(file);
}

void vfs_read(VFSFile* file, void* buffer, uint32_t buffer_size)
{
    file->inode->file_operations.read(file, buffer, buffer_size);
}

void vfs_write(VFSFile* file, void* buffer, uint32_t buffer_size)
{
    file->inode->file_operations.write(file, buffer, buffer_size);
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
