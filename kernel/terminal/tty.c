#include "tty.h"
#include <filesystem/virtual-filesystem.h>
#include <heap.h>
#include <keyboard/input.h>
#include <print.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void print_char(uint8_t chr);
void print_string(uint8_t* str);
void set_color(uint8_t fg, uint8_t bg);
void newline();
void clear();
void get_cursor_pos(uint8_t* x, uint8_t* y);
void set_cursor_pos(uint8_t x, uint8_t y);

uint8_t cursor_row = 0;
uint8_t cursor_col = 0;

uint8_t color = White | Black << 4;

uint8_t* input_buffer_base = NULL;
uint8_t* input_buffer_pos = NULL;

VFSFile* tty_open(VFSIndexNode* inode)
{
    VFSFile* file = (VFSFile*)malloc(sizeof(VFSFile));
    file->inode = inode;
    file->position = 0;
    file->private_data = NULL;
    file->private_data_size = 0;
    return file;
}
void tty_close(VFSFile* file)
{
    free(file);
}

uint32_t tty_read(VFSFile* file, void* buffer, uint32_t buffer_size)
{
    if (input_buffer_base == NULL) {

        input_buffer_base = get_line();
        input_buffer_pos = input_buffer_base;

        if (input_buffer_pos == NULL) {
            return 0;
        }
    }

    uint32_t read_length = buffer_size;

    if (strlen((char*)input_buffer_pos) < buffer_size) {
        read_length = strlen((char*)input_buffer_pos);
    }

    memcpy(buffer, input_buffer_pos, read_length);
    input_buffer_pos += read_length;

    if (strlen((char*)input_buffer_pos) == 0) {
        free(input_buffer_base);
        input_buffer_base = NULL;
        input_buffer_pos = NULL;
    }

    return read_length;
}
uint32_t tty_write(VFSFile* file, void* buffer, uint32_t buffer_size)
{
    for (uint32_t i = 0; i < buffer_size; i++) {
        print_char(((char*)buffer)[i]);
    }
    return buffer_size;
}
void tty_ioctl(VFSFile* file, uint32_t* command, uint32_t* arg)
{
    switch (*command) {
    case TTY_CLEAR:
        clear();
        break;
    case TTY_SET_BG_COLOR:
        color = color | *arg << 4;
        break;
    case TTY_SET_FG_COLOR:
        color = (*arg & 0xf) | color;
        break;
    case TTY_SET_CURSOR_POS:
        cursor_row = *arg >> 16;
        cursor_col = *arg & 0xffff;
        break;
    case TTY_GET_CURSOR_POS:
        *arg = cursor_row << 16;
        *arg = *arg | cursor_col;
        break;
    }
}
void tty_seek(VFSFile* file, uint32_t offset, uint32_t whence) { }
uint32_t tty_tell(VFSFile* file) { return 0; }
void tty_flush(VFSFile* file) { }

VFSFileOperations get_tty_file_operations()
{
    VFSFileOperations fops = {
        .open = (void*)tty_open,
        .close = (void*)tty_close,
        .read = (void*)tty_read,
        .write = (void*)tty_write,
        .ioctl = (void*)tty_ioctl,
        .seek = (void*)tty_seek,
        .tell = (void*)tty_tell,
        .flush = (void*)tty_flush,
    };
    return fops;
}

void print_char(uint8_t chr)
{
    switch (chr) {
    case '\n':
        newline();
        return;
    case '\r':
        cursor_col = 0;
        return;
    case '\t':
        print_string((uint8_t*)"        ");
        return;
    case '\b':
        if (cursor_col > 0) {
            cursor_col--;
        }
        return;
    }
    text_buffer[cursor_col + cursor_row * VGA_WIDTH] = chr | color << 8;
    cursor_col++;
    if (cursor_col >= VGA_WIDTH) {
        newline();
    }
    return;
}

void print_string(uint8_t* str)
{
    for (uint8_t* c = str; *c != '\0'; c++) {
        print_char(*c);
    }
    return;
}

void set_color(uint8_t fg, uint8_t bg)
{
    color = fg | bg << 4;
}

void newline()
{
    cursor_col = 0;
    cursor_row++;
    if (cursor_row >= VGA_HEIGHT) {
        for (int y = 0; y < VGA_HEIGHT - 1; y++) {
            for (int x = 0; x < VGA_WIDTH; x++) {
                text_buffer[x + y * VGA_WIDTH] = text_buffer[x + (y + 1) * VGA_WIDTH];
            }
        }
        for (int x = 0; x < VGA_WIDTH; x++) {
            text_buffer[x + 24 * VGA_WIDTH] = '\0';
        }
        cursor_row = VGA_HEIGHT - 1;
    }
}

void clear()
{
    cursor_col = 0;
    cursor_row = 0;
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            text_buffer[x + y * VGA_WIDTH] = '0';
        }
    }
}

void get_cursor_pos(uint8_t* x, uint8_t* y)
{
    *x = cursor_col;
    *y = cursor_row;
}

void set_cursor_pos(uint8_t x, uint8_t y)
{
    cursor_col = x;
    cursor_row = y;
}
