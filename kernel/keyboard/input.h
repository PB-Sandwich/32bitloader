
#pragma once

#include <stdint.h>

enum Keycode : uint8_t {
    NONE = 0,
    ESC, // 0x01
    KEY_1, // 0x02
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_0, // 0x0B
    MINUS, // 0x0C
    EQUAL,
    BACKSPACE, // 0x0E
    TAB, // 0x0F
    Q, // 0x10
    W,
    E,
    R,
    T,
    Y,
    U,
    I,
    O,
    P, // 0x19
    LEFT_BRACKET, // 0x1A
    RIGHT_BRACKET,
    ENTER, // 0x1C
    LEFT_CTRL, // 0x1D
    A, // 0x1E
    S,
    D,
    F,
    G,
    H,
    J,
    K,
    L,
    SEMICOLON, // 0x27
    APOSTROPHE,
    GRAVE, // 0x29 `~
    LEFT_SHIFT, // 0x2A
    BACKSLASH, // 0x2B
    Z,
    X,
    C,
    V,
    B,
    N,
    M,
    COMMA,
    DOT,
    SLASH, // 0x35
    RIGHT_SHIFT,
    KP_ASTERISK, // Numpad *
    LEFT_ALT,
    SPACE, // 0x39
    CAPSLOCK, // 0x3A

    // You can continue with F1–F12 and navigation keys:
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    SCROLLLOCK,
    NUMLOCK,
    KP_7,
    KP_8,
    KP_9,
    KP_MINUS,
    KP_4,
    KP_5,
    KP_6,
    KP_PLUS,
    KP_1,
    KP_2,
    KP_3,
    KP_0,
    KP_DOT,

    // Extended scancodes (0xE0 prefix) can be added as needed
    RIGHT_CTRL,
    RIGHT_ALT,
    ARROW_UP,
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT,
    INSERT,
    DELETE,
    HOME,
    END,
    PAGE_UP,
    PAGE_DOWN,

    // Meta/GUI
    LEFT_GUI,
    RIGHT_GUI,
};

enum Keycode wait_for_keypress();
uint8_t* get_line(); // returns the pointer to input buffer (which will get overwritten on next call)

uint8_t keycode_to_ascii(enum Keycode kc);
enum Keycode scancode_to_keycode(uint8_t sc);
