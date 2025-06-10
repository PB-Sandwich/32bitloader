
#pragma once

#include <stdint.h>

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
uint8_t get_cursor_pos(); // high order are x, low order are y
void set_cursor_pos(uint8_t pos); // high order are x, low order are y
