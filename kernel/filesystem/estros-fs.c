#include "estros-fs.h"
#include <harddrive/hdd.h>
#include <heap.h>
#include <memutils.h>

int create_inode(char* path, VFSFileType type);
VFSIndexNode* get_inode(char* path);
VFSDirectory* get_directory_entries(char* path);
int write_directory_entries(VFSDirectory vfs_directory);
