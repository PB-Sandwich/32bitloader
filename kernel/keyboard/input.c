#include "input.h"
#include <interrupts/irq_handlers.h>
#include <print.h>
#include <terminal/tty.h>
#include <stdint.h>

uint8_t buffer[32] = { '\0' };

enum Keycode wait_for_keypress()
{
    clear_key_pressed();
    while (!key_pressed() || ((scancode() & 0b10000000) > 0))
        ;
    return scancode_to_keycode(scancode());
}

uint8_t* get_line()
{
    buffer[0] = keycode_to_ascii(wait_for_keypress());
    pprint_char(buffer[0]);
    int i = 1;
    while (1) {
        enum Keycode kc = wait_for_keypress();
        if (kc == BACKSPACE && i > 0) {
            buffer[i] = '\0';
            pprint_char('\b');
            pprint_char(' ');
            pprint_char('\b');
            i--;
            continue;
        }

        uint8_t ascii = keycode_to_ascii(kc);

        if (ascii == '\0') {
            continue;
        }

        if (ascii == '\n') {
            buffer[i] = '\0';
            break;
        }

        buffer[i] = ascii;

        if (i >= 31) {
            continue;
        }

        pprint_char(buffer[i]);
        i++;
    }
    pprint_char('\n');
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

uint8_t keycode_to_ascii(enum Keycode kc)
{
    if (is_shifted()) {
    return shifted_ascii_table[kc];
    }
    return ascii_table[kc];
}

enum Keycode scancode_to_keycode(uint8_t sc)
{
    sc &= 0b01111111; // remove released
    return (enum Keycode)sc;
}
