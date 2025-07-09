/**
 * @file estros.h
 * @author Sofia "MetalPizzaCat"
 * @brief Very barebones file that contains basic structures required to to interact with kernel for EstrOS.
 * @version 0.1
 * @date 2025-06-27
 *
 * @copyright Copyright (c) 2025
 *
 */
#pragma once
#include <stdint.h>
#include <stdarg.h>
enum Keycode
{
    EKC_NONE = 0,
    EKC_ESC,   // 0x01
    EKC_KEY_1, // 0x02
    EKC_KEY_2,
    EKC_KEY_3,
    EKC_KEY_4,
    EKC_KEY_5,
    EKC_KEY_6,
    EKC_KEY_7,
    EKC_KEY_8,
    EKC_KEY_9,
    EKC_KEY_0, // 0x0B
    EKC_MINUS, // 0x0C
    EKC_EQUAL,
    EKC_BACKSPACE, // 0x0E
    EKC_TAB,       // 0x0F
    EKC_Q,         // 0x10
    EKC_W,
    EKC_E,
    EKC_R,
    EKC_T,
    EKC_Y,
    EKC_U,
    EKC_I,
    EKC_O,
    EKC_P,            // 0x19
    EKC_LEFT_BRACKET, // 0x1A
    EKC_RIGHT_BRACKET,
    EKC_ENTER,     // 0x1C
    EKC_LEFT_CTRL, // 0x1D
    EKC_A,         // 0x1E
    EKC_S,
    EKC_D,
    EKC_F,
    EKC_G,
    EKC_H,
    EKC_J,
    EKC_K,
    EKC_L,
    EKC_SEMICOLON, // 0x27
    EKC_APOSTROPHE,
    EKC_GRAVE,      // 0x29 `~
    EKC_LEFT_SHIFT, // 0x2A
    EKC_BACKSLASH,  // 0x2B
    EKC_Z,
    EKC_X,
    EKC_C,
    EKC_V,
    EKC_B,
    EKC_N,
    EKC_M,
    EKC_COMMA,
    EKC_DOT,
    EKC_SLASH, // 0x35
    EKC_RIGHT_SHIFT,
    EKC_KP_ASTERISK, // Numpad *
    EKC_LEFT_ALT,
    EKC_SPACE,    // 0x39
    EKC_CAPSLOCK, // 0x3A

    // You can continue with F1–F12 and navigation keys:
    EKC_F1,
    EKC_F2,
    EKC_F3,
    EKC_F4,
    EKC_F5,
    EKC_F6,
    EKC_F7,
    EKC_F8,
    EKC_F9,
    EKC_F10,
    EKC_F11,
    EKC_F12,
    EKC_SCROLLLOCK,
    EKC_NUMLOCK,
    EKC_KP_7,
    EKC_KP_8,
    EKC_KP_9,
    EKC_KP_MINUS,
    EKC_KP_4,
    EKC_KP_5,
    EKC_KP_6,
    EKC_KP_PLUS,
    EKC_KP_1,
    EKC_KP_2,
    EKC_KP_3,
    EKC_KP_0,
    EKC_KP_DOT,

    // Extended scancodes (0xE0 prefix) can be added as needed
    EKC_RIGHT_CTRL,
    EKC_RIGHT_ALT,
    EKC_ARROW_UP,
    EKC_ARROW_DOWN,
    EKC_ARROW_LEFT,
    EKC_ARROW_RIGHT,
    EKC_INSERT,
    EKC_DELETE,
    EKC_HOME,
    EKC_END,
    EKC_PAGE_UP,
    EKC_PAGE_DOWN,

    // Meta/GUI
    EKC_LEFT_GUI,
    EKC_RIGHT_GUI,
};

enum Colors
{
    EC_Black,
    EC_Blue,
    EC_Green,
    EC_Cyan,
    EC_Red,
    EC_Magenta,
    EC_Brown,
    EC_LightGray,
    EC_DarkGray,
    EC_LightBlue,
    EC_LightGreen,
    EC_LightCyan,
    EC_LightRed,
    EC_Pink,
    EC_Yellow,
    EC_White
};

/// @brief Colors available to use in the text buffer
typedef enum Colors KernelTerminalColors;

/// @brief Struct exposing functions for interacting with kernel
struct KernelExports
{
    void (*printf)(const char *fmt, ...);

    void (*print_char)(uint8_t chr);
    void (*print_string)(uint8_t *str);
    void (*set_color)(uint8_t fg, uint8_t bg);
    void (*newline)();
    void (*clear)();
    void (*get_cursor_pos)(uint8_t *x, uint8_t *y);
    void (*set_cursor_pos)(uint8_t x, uint8_t y);

    void (*clear_key_pressed)(); // clears the key pressed flag
    uint8_t (*key_pressed)();    // returns 1 if a key is pressed
    uint8_t (*scancode)();       // gets the last scancode
    void (*set_keyboard_function)(void (*(*keyboard_function_))(uint8_t scancode));

    uint8_t (*wait_for_keypress)();
    uint8_t *(*get_line)(); // returns the pointer to input buffer (*)(which will get overwritten on next call)
    uint8_t (*keycode_to_ascii)(enum Keycode kc);
    uint8_t (*scancode_to_keycode)(uint8_t sc);

    void (*ata_read_sector)(uint32_t lba, uint8_t *buffer);
    void (*ata_write_sector)(uint32_t lba, uint8_t *buffer);

    struct IDTEntry (*make_idt_entry)(uint32_t *offset, uint16_t selector, uint8_t type_attr);

    void (*memcpy)(void *dest, void *source, uint32_t size);
};

uint16_t* get_text_buffer_address();