
#pragma once

#include <stdint.h>

#define VGA_WIDTH 80 // 64
#define VGA_HEIGHT 25 // 25

static uint16_t* text_buffer = (uint16_t*)0xb8000;

enum {
    Black,
    Blue,
    Green,
    Cyan,
    Red,
    Magenta,
    Brown,
    LightGray,
    DarkGray,
    LightBlue,
    LightGreen,
    LightCyan,
    LightRed,
    Pink,
    Yellow,
    White
};

void print_char(uint8_t chr);
void print_string(uint8_t* str);
void set_color(uint8_t fg, uint8_t bg);
void newline();
void clear();
void get_cursor_pos(uint8_t* x, uint8_t* y);
void set_cursor_pos(uint8_t x, uint8_t y);
