#include "input.h"
#include <filesystem/virtual-filesystem.h>
#include <heap.h>
#include <keyboard/keyboard.h>
#include <print.h>
#include <stdint.h>
#include <terminal/tty.h>

VFSFile* kdb = NULL;

void set_input_kdb_dev(char* path)
{
    if (kdb != NULL) {
        vfs_close_file(kdb);
    }
    kdb = vfs_open_file(path, VFS_READ);
}

enum Keycode wait_for_keypress()
{
    KeyboardEvent event;
    while (!vfs_read(kdb, &event, sizeof(KeyboardEvent)))
        ;
    ;
    return scancode_to_keycode(event.scancode);
}

uint8_t* get_line(void (*printchar)(char))
{
    uint8_t* buffer = (uint8_t*)malloc(32);
    if (buffer == NULL) {
        return NULL;
    }
    uint32_t buffer_size = 32;

    uint32_t i = 0;
    uint8_t shifted = 0;
    while (1) {
        KeyboardEvent event;
        if (!vfs_read(kdb, &event, sizeof(KeyboardEvent))) {
            continue;
        }

        enum Keycode kc = scancode_to_keycode(event.scancode);

        if (kc == KC_LEFT_SHIFT || kc == KC_RIGHT_SHIFT) {
            if (event.type == KEY_PRESSED) {
                shifted = 1;
            } else if (event.type == KEY_RELEASED) {
                shifted = 0;
            }
        }

        if (event.type == KEY_RELEASED) {
            continue;
        }

        if (kc == KC_BACKSPACE && i > 0) {
            buffer[i] = '\0';
            printchar('\b');
            printchar(' ');
            printchar('\b');
            i--;
            continue;
        }

        uint8_t ascii = keycode_to_ascii(kc, shifted);

        if (ascii == '\0') {
            continue;
        }

        if (ascii == '\n') {
            buffer[i] = '\0';
            break;
        }

        buffer[i] = ascii;

        if (buffer_size >= i) {
            void* temp = realloc(buffer, buffer_size + 32);
            if (temp == NULL) {
                return buffer;
            }
            buffer = temp;
        }

        printchar(buffer[i]);
        i++;
    }
    printchar('\n');
    return buffer;
}

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
