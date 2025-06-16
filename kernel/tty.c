#include "tty.h"
#include <stdint.h>

uint8_t cursor_row = 0;
uint8_t cursor_col = 0;

uint8_t color = White | Black << 4;

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

uint8_t get_cursor_pos()
{
    return cursor_col << 4 | cursor_row;
}
void set_cursor_pos(uint8_t pos)
{
    cursor_row = pos & 0b1111;
    cursor_col = pos >> 4 & 0b1111;
}
