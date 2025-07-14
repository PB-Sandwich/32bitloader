
#pragma once

#include <stdint.h>
#include <filesystem/virtual-filesystem.h>

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

enum {
    TTY_CLEAR = 0,
    TTY_SET_BG_COLOR = 1,
    TTY_SET_FG_COLOR = 2,
    TTY_SET_CURSOR_POS = 3,
    TTY_GET_CURSOR_POS = 4,
};

VFSFileOperations get_tty_file_operations();
