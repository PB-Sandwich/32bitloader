#include "hdd.h"
#include "ata.h"
#include "filesystem/virtual-filesystem.h"
#include <print.h>
#include <stdint.h>
#include <stdlib.h>

VFSFile* hdd_open(VFSIndexNode* inode, VFSFileFlags flags)
{
    VFSFile* file = (VFSFile*)malloc(sizeof(VFSFile));
    file->inode = inode;
    file->position = 0;
    file->private_data = NULL;
    file->private_data_size = 0;
    return file;
}

void hdd_close(VFSFile* file)
{
    free(file);
}

uint32_t hdd_read(VFSFile* file, void* buffer, uint32_t buffer_size)
{
    uint32_t n_sectors = buffer_size / SECTOR_SIZE;
    uint32_t i;
    for (i = 0; i < n_sectors; i++) {
        ata_read_sector(file->position + i, buffer + i * SECTOR_SIZE);
    }
    return i * 512;
}

uint32_t hdd_write(VFSFile* file, void* buffer, uint32_t buffer_size)
{
    uint32_t n_sectors = buffer_size / SECTOR_SIZE;
    uint32_t i;
    for (i = 0; i < n_sectors; i++) {
        ata_write_sector(file->position + i, buffer + i * SECTOR_SIZE);
    }
    return i * 512;
}

void hdd_ioctl(VFSFile* file, uint32_t command, uint32_t arg)
{
    return;
}

void hdd_seek(VFSFile* file, uint32_t offset, uint32_t whence)
{
    switch (whence) {
    case VFS_BEG:
        file->position = offset / SECTOR_SIZE;
        break;
    case VFS_CUR:
        file->position += offset / SECTOR_SIZE;
        break;
    case VFS_END:
        break;
    }
    return;
}

uint32_t hdd_tell(VFSFile* file)
{
    return file->position * 512;
}

void hdd_flush(VFSFile* file)
{
    return;
}

VFSFileOperations get_hdd_file_operations()
{
    VFSFileOperations fops = {
        .open = (void*)hdd_open,
        .close = (void*)hdd_close,
        .read = (void*)hdd_read,
        .write = (void*)hdd_write,
        .ioctl = (void*)hdd_ioctl,
        .seek = (void*)hdd_seek,
        .tell = (void*)hdd_tell,
        .flush = (void*)hdd_flush,
    };
    return fops;
}
