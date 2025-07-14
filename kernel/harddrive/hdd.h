
#pragma once

#include <filesystem/virtual-filesystem.h>
#include <stdint.h>

#define SECTOR_SIZE 512


VFSFileOperations get_hdd_file_operations();
