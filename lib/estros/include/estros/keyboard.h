#ifndef ESTROS_KEYBOARD_H
#define ESTROS_KEYBOARD_H

#include <estros/file.h>
#include <stdint.h>

typedef enum : uint8_t {
    KEY_PRESSED = 1,
    KEY_RELEASED = 2,
} KeyboardEventType;

typedef struct {
    uint16_t scancode;
    uint8_t type;
} __attribute__((packed)) KeyboardEvent;

enum Keycode : uint8_t {
    KC_NONE = 0,
    KC_ESC, // 0x01
    KC_KEY_1, // 0x02
    KC_KEY_2,
    KC_KEY_3,
    KC_KEY_4,
    KC_KEY_5,
    KC_KEY_6,
    KC_KEY_7,
    KC_KEY_8,
    KC_KEY_9,
    KC_KEY_0, // 0x0B
    KC_MINUS, // 0x0C
    KC_EQUAL,
    KC_BACKSPACE, // 0x0E
    KC_TAB, // 0x0F
    KC_Q, // 0x10
    KC_W,
    KC_E,
    KC_R,
    KC_T,
    KC_Y,
    KC_U,
    KC_I,
    KC_O,
    KC_P, // 0x19
    KC_LEFT_BRACKET, // 0x1A
    KC_RIGHT_BRACKET,
    KC_ENTER, // 0x1C
    KC_LEFT_CTRL, // 0x1D
    KC_A, // 0x1E
    KC_S,
    KC_D,
    KC_F,
    KC_G,
    KC_H,
    KC_J,
    KC_K,
    KC_L,
    KC_SEMICOLON, // 0x27
    KC_APOSTROPHE,
    KC_GRAVE, // 0x29 `~
    KC_LEFT_SHIFT, // 0x2A
    KC_BACKSLASH, // 0x2B
    KC_Z,
    KC_X,
    KC_C,
    KC_V,
    KC_B,
    KC_N,
    KC_M,
    KC_COMMA,
    KC_DOT,
    KC_SLASH, // 0x35
    KC_RIGHT_SHIFT,
    KC_KP_ASTERISK, // Numpad *
    KC_LEFT_ALT,
    KC_SPACE, // 0x39
    KC_CAPSLOCK, // 0x3A

    // You can continue with F1–F12 and navigation keys:
    KC_F1,
    KC_F2,
    KC_F3,
    KC_F4,
    KC_F5,
    KC_F6,
    KC_F7,
    KC_F8,
    KC_F9,
    KC_F10,
    KC_F11,
    KC_F12,
    KC_SCROLLLOCK,
    KC_NUMLOCK,
    KC_KP_7,
    KC_KP_8,
    KC_KP_9,
    KC_KP_MINUS,
    KC_KP_4,
    KC_KP_5,
    KC_KP_6,
    KC_KP_PLUS,
    KC_KP_1,
    KC_KP_2,
    KC_KP_3,
    KC_KP_0,
    KC_KP_DOT,

    // Extended scancodes (0xE0 prefix) can be added as needed
    KC_RIGHT_CTRL,
    KC_RIGHT_ALT,
    KC_ARROW_UP,
    KC_ARROW_DOWN,
    KC_ARROW_LEFT,
    KC_ARROW_RIGHT,
    KC_INSERT,
    KC_DELETE,
    KC_HOME,
    KC_END,
    KC_PAGE_UP,
    KC_PAGE_DOWN,

    // Meta/GUI
    KC_LEFT_GUI,
    KC_RIGHT_GUI,
};

uint8_t ascii_table[256] = {
    [0x02] = '1',
    [0x03] = '2',
    [0x04] = '3',
    [0x05] = '4',
    [0x06] = '5',
    [0x07] = '6',
    [0x08] = '7',
    [0x09] = '8',
    [0x0A] = '9',
    [0x0B] = '0',
    [0x0C] = '-',
    [0x0D] = '=',
    [0x0E] = '\b',
    [0x0F] = '\t',
    [0x10] = 'q',
    [0x11] = 'w',
    [0x12] = 'e',
    [0x13] = 'r',
    [0x14] = 't',
    [0x15] = 'y',
    [0x16] = 'u',
    [0x17] = 'i',
    [0x18] = 'o',
    [0x19] = 'p',
    [0x1A] = '[',
    [0x1B] = ']',
    [0x1C] = '\n',
    [0x1E] = 'a',
    [0x1F] = 's',
    [0x20] = 'd',
    [0x21] = 'f',
    [0x22] = 'g',
    [0x23] = 'h',
    [0x24] = 'j',
    [0x25] = 'k',
    [0x26] = 'l',
    [0x27] = ';',
    [0x28] = '\'',
    [0x29] = '`',
    [0x2C] = 'z',
    [0x2D] = 'x',
    [0x2E] = 'c',
    [0x2F] = 'v',
    [0x30] = 'b',
    [0x31] = 'n',
    [0x32] = 'm',
    [0x33] = ',',
    [0x34] = '.',
    [0x35] = '/',
    [0x39] = ' ',
};

uint8_t shifted_ascii_table[256] = {
    [0x02] = '!',
    [0x03] = '@',
    [0x04] = '#',
    [0x05] = '$',
    [0x06] = '%',
    [0x07] = '^',
    [0x08] = '&',
    [0x09] = '*',
    [0x0A] = '(',
    [0x0B] = ')',
    [0x0C] = '_',
    [0x0D] = '+',
    [0x0E] = '\b',
    [0x0F] = '\t',
    [0x10] = 'Q',
    [0x11] = 'W',
    [0x12] = 'E',
    [0x13] = 'R',
    [0x14] = 'T',
    [0x15] = 'Y',
    [0x16] = 'U',
    [0x17] = 'I',
    [0x18] = 'O',
    [0x19] = 'P',
    [0x1A] = '{',
    [0x1B] = '}',
    [0x1C] = '\n',
    [0x1E] = 'A',
    [0x1F] = 'S',
    [0x20] = 'D',
    [0x21] = 'F',
    [0x22] = 'G',
    [0x23] = 'H',
    [0x24] = 'J',
    [0x25] = 'K',
    [0x26] = 'L',
    [0x27] = ':',
    [0x28] = '\"',
    [0x29] = '~',
    [0x2C] = 'Z',
    [0x2D] = 'X',
    [0x2E] = 'C',
    [0x2F] = 'V',
    [0x30] = 'B',
    [0x31] = 'N',
    [0x32] = 'M',
    [0x33] = '<',
    [0x34] = '>',
    [0x35] = '?',
    [0x39] = ' ',
};

uint8_t keycode_to_ascii(enum Keycode kc, uint8_t is_shifted)
{
    if (is_shifted) {
        return shifted_ascii_table[kc];
    }
    return ascii_table[kc];
}

enum Keycode scancode_to_keycode(uint8_t sc)
{
    sc &= 0b01111111; // remove released
    return (enum Keycode)sc;
}

enum Keycode wait_for_keypress(File* kdb_file)
{
    KeyboardEvent event = { 0 };
    while (!read_file(kdb_file, &event, sizeof(KeyboardEvent)) || event.type != KEY_PRESSED)
        ;
    ;
    return scancode_to_keycode(event.scancode);
}

#endif
