#include "keyboard.h"
#include <heap.h>
#include <memutils.h>
#include <print.h>
#include <stdint.h>

#define FILE_BUFFER_SIZE 32

struct KeyboardFileData {
    KeyboardEvent buffer[FILE_BUFFER_SIZE];
    KeyboardEvent* start;
    KeyboardEvent* end;
};

struct KeyboardFileData** keyboard_file_data = NULL;
uint32_t keyboard_file_data_length = 0;

void on_event(uint8_t scancode)
{
    for (uint32_t i = 0; i < keyboard_file_data_length; i++) {
        KeyboardEvent* buffer = keyboard_file_data[i]->buffer;
        KeyboardEvent** end = &keyboard_file_data[i]->end;
        KeyboardEvent** start = &keyboard_file_data[i]->start;

        (*end)->scancode = scancode & ~(1 << 7);
        (*end)->type = (scancode & (1 << 7)) ? KEY_RELEASED : KEY_PRESSED;

        (*end)++;
        if (*end >= buffer + FILE_BUFFER_SIZE) {
            *end = buffer;
        }
        if (*end == *start) {
            (*start)++;
            (*start)++;
            if (*start >= buffer + FILE_BUFFER_SIZE) {
                *start = buffer;
            }
        }
    }
}

VFSFile* keyboard_open(VFSIndexNode* inode)
{
    VFSFile* file = (VFSFile*)malloc(sizeof(VFSFile));
    if (file == NULL) {
        return NULL;
    }
    file->private_data = (void*)malloc(sizeof(struct KeyboardFileData));
    struct KeyboardFileData* fd = file->private_data;
    if (fd == NULL) {
        free(file);
        return NULL;
    }
    memset(fd->buffer, 0, sizeof(KeyboardEvent) * FILE_BUFFER_SIZE);
    fd->start = fd->buffer;
    fd->end = fd->buffer;
    file->inode = inode;
    file->position = 0;
    file->private_data_size = 0;

    if (keyboard_file_data_length == 0) {
        keyboard_file_data_length++;
        keyboard_file_data = malloc(sizeof(void*));
        keyboard_file_data[0] = fd;
    } else {
        keyboard_file_data_length++;
        void* temp = realloc(keyboard_file_data, sizeof(void*) * keyboard_file_data_length);
        if (temp == NULL) {
            free(fd);
            free(file);
            return NULL;
        }
        keyboard_file_data = temp;
        keyboard_file_data[keyboard_file_data_length - 1] = fd;
    }
    return file;
}
void keyboard_close(VFSFile* file)
{
    keyboard_file_data_length--;
    if (keyboard_file_data_length == 0) {
        free(keyboard_file_data);
    } else {
        uint32_t replace_index = keyboard_file_data_length;
        for (uint32_t i = 0; i < keyboard_file_data_length + 1; i++) {
            if (file->private_data == keyboard_file_data) {
                replace_index = i;
                break;
            }
        }
        keyboard_file_data[replace_index] = keyboard_file_data[keyboard_file_data_length];
        keyboard_file_data = realloc(keyboard_file_data, sizeof(void*) * keyboard_file_data_length);
    }
    free(file->private_data);
    free(file);
}

uint32_t keyboard_read(VFSFile* file, void* buffer, uint32_t buffer_size)
{
    uint32_t number_to_read = buffer_size / sizeof(KeyboardEvent);

    struct KeyboardFileData* fd = file->private_data;
    KeyboardEvent* file_buffer = fd->buffer;
    KeyboardEvent** end = &fd->end;
    KeyboardEvent** start = &fd->start;

    uint32_t i = 0;
    for (; i < number_to_read; i++) {
        if (*start == *end) {
            break;
        }

        memcpy(buffer, *start, sizeof(KeyboardEvent));
        buffer += sizeof(KeyboardEvent);

        (*start)++;
        if (*start >= file_buffer + FILE_BUFFER_SIZE) {
            *start = file_buffer;
        }
    }
    return i;
}

uint32_t keyboard_write(VFSFile* file, void* buffer, uint32_t buffer_size) { return 0; }
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
