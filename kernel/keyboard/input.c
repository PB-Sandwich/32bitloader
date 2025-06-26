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
    print_char(buffer[0]);
    int i = 1;
    while (1) {
        enum Keycode kc = wait_for_keypress();
        if (kc == BACKSPACE && i > 0) {
            buffer[i] = '\0';
            print_char('\b');
            print_char(' ');
            print_char('\b');
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

        print_char(buffer[i]);
        i++;
    }
    newline();
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

uint8_t keycode_to_ascii(enum Keycode kc)
{
    return ascii_table[kc];
}

enum Keycode scancode_to_keycode(uint8_t sc)
{
    sc &= 0b01111111; // remove released
    return (enum Keycode)sc;
}
