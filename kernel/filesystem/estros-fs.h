
#pragma once

#include <filesystem/virtual-filesystem.h>

void fs_set_harddrive(char* path);
VFSFileOperations get_fs_file_operations();
VFSDriverOperations get_fs_driver_operations();
