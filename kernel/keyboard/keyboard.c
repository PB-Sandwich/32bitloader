#include "keyboard.h"
#include <heap.h>
#include <string.h>

struct keyboard_file_data {
    KeyboardEvent buffer[32];
};

VFSFile* keyboard_open(VFSIndexNode* inode)
{
    VFSFile* file = (VFSFile*)malloc(sizeof(VFSFile));
    if (file == NULL) {
        return NULL;
    }
    file->private_data = (void*)malloc(sizeof(struct keyboard_file_data));
    struct keyboard_file_data* fd = file->private_data;
    if (fd == NULL) {
        free(file);
        return NULL;
    }
    memset(fd->buffer, 0, sizeof(KeyboardEvent) * 32);
    file->inode = inode;
    file->position = 0;
    file->private_data_size = 0;
    return file;
}
void keyboard_close(VFSFile* file)
{
    free(file->private_data);
    free(file);
}

uint32_t keyboard_read(VFSFile* file, void* buffer, uint32_t buffer_size)
{
}
uint32_t keyboard_write(VFSFile* file, void* buffer, uint32_t buffer_size)
{
    return 0;
}
void keyboard_ioctl(VFSFile* file, uint32_t* command, uint32_t* arg) { }
void keyboard_seek(VFSFile* file, uint32_t offset, uint32_t whence) { }
uint32_t keyboard_tell(VFSFile* file) { return 0; }
void keyboard_flush(VFSFile* file) { }

VFSFileOperations get_keyboard_file_operations()
{
    VFSFileOperations fops = {
        .open = (void*)keyboard_open,
        .close = (void*)keyboard_close,
        .read = (void*)keyboard_read,
        .write = (void*)keyboard_write,
        .ioctl = (void*)keyboard_ioctl,
        .seek = (void*)keyboard_seek,
        .tell = (void*)keyboard_tell,
        .flush = (void*)keyboard_flush,
    };
    return fops;
}
