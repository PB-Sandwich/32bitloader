
#pragma once

#include <stdint.h>

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

void set_input_kdb_dev(char* path);

enum Keycode wait_for_keypress();
uint8_t* get_line(void (*printchar)(char));

uint8_t keycode_to_ascii(enum Keycode kc, uint8_t is_shifted);
enum Keycode scancode_to_keycode(uint8_t sc);
