
#pragma once

#include <filesystem/virtual-filesystem.h>
#include <stdint.h>

typedef enum {
    KEY_PRESSED = 1,
    KEY_RELEASED = 2,
} KeyboardEventType;

typedef struct {
    uint16_t scancode;
    uint8_t type;
} __attribute__((packed)) KeyboardEvent;

void on_event(uint8_t scancode);
VFSFileOperations get_keyboard_file_operations();
