#include "virtual-filesystem.h"
#include <heap.h>
#include <memutils.h>
#include <stdlib.h>

VFSDirectory root = { 0 };
VFSDriverOperations dops = { 0 };
VFSFileOperations default_fops = { 0 };

int vfs_init(VFSDriverOperations driver_operations, VFSFileOperations default_file_operations)
{
    dops = driver_operations;
    default_fops = default_file_operations;
    root.entries_length++;
    root.entries = (VFSDirectoryEntry*)malloc(sizeof(VFSDirectoryEntry));
    if (root.entries == NULL) {
        return 1;
    }
    char* name = "dev";
    root.entries[0].name = (char*)malloc(strlen(name));
    strcpy(root.entries[0].name, name);
    root.entries[0].inode = NULL;
    root.entries->vfs_directory = (VFSDirectory*)malloc(sizeof(VFSDirectory));
    if (root.entries->vfs_directory == NULL) {
        return 1;
    }
    return 0;
}

VFSIndexNode* get_virtual_inode(char* path, VFSDirectory* dir)
{
    while (*path == '/') {
        path++;
    }
    for (int i = 0; i < dir->entries_length; i++) {
        if (strstr(path, dir->entries[i].name) == 0) {
            uint32_t sublen = strchr(path, '/') - path;
            if (sublen == 0) {
                return dir->entries[i].inode;
            }
            path = path + sublen;
            return get_virtual_inode(path, dir->entries[i].vfs_directory);
        }
    }
    return NULL;
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
        strncpy(name, path + offset, strchr(path + offset, '/') - path + offset);
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
            dir->entries[dir->entries_length - 1].name = (char*)malloc(strlen(name));
            if (dir->entries[dir->entries_length - 1].name == NULL) {
                free(name);
                free(cur_path);
                return NULL;
            }
            strcpy(dir->entries[dir->entries_length - 1].name, name);
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
    // VFSIndexNode* inode = dops.get_inode(path);
    // if (inode != NULL) {
    //     return 1; // already exists in physical filesystem
    // }
    // free(inode);

    uint32_t offset = 0;
    for (uint32_t i = strlen(path) - 1; i > 0; i--) {
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

    directory->entries_length++;
    if (directory->entries != NULL) {
        directory->entries = (VFSDirectoryEntry*)realloc(directory->entries, sizeof(VFSDirectoryEntry) * directory->entries_length);
    } else {
        directory->entries = (VFSDirectoryEntry*)malloc(sizeof(VFSDirectoryEntry) * directory->entries_length);
    }
    if (directory->entries == NULL) {
        return 1;
    }

    uint32_t name_length = 0;
    uint32_t name_offset = 0;
    for (int i = strlen(path) - 1; i > 0; i--) {
        if (path[i] == '/') {
            name_length = strlen(path) - i;
            name_offset = i;
            break;
        }
    }
    VFSDirectoryEntry entry = {
        .inode = (VFSIndexNode*)malloc(sizeof(VFSIndexNode)),
        .vfs_directory = NULL,
        .name = (char*)malloc(name_length),
    };
    entry.inode->type = type;
    entry.inode->file_operations = fops;
    strncpy(entry.name, path + name_offset, name_length);

    return 0;
}

int vfs_create_directory(char* path);

VFSDirectory* vfs_open_directory(char* path);
void vfs_close_directory(VFSDirectory* vfs_directory);

// returns NULL on fail
VFSFile* vfs_open_file(char* path)
{
    VFSIndexNode* inode = get_virtual_inode(path, &root);
    if (inode == NULL) {
        // inode = dops.get_inode(path);
        // if (inode == NULL) {
        //     return NULL;
        // }
    }
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

void vfs_close_file(VFSFile* file);
