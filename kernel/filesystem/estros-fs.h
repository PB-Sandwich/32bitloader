
#pragma once

#include <filesystem/virtual-filesystem.h>

int create_inode(char* path, VFSFileType type);
VFSIndexNode* get_inode(char* path);
VFSDirectory* get_directory_entries(char* path);
int write_directory_entries(VFSDirectory vfs_directory);
