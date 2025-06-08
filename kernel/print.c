#include <stdarg.h>
#include <stdint.h>
#include "tty.h"

void print_int(int num)
{
    char buf[12]; // enough for -2,147,483,648
    int i = 0;

    if (num == 0) {
        print_char('0');
        return;
    }

    if (num < 0) {
        print_char('-');
        num = -num;
    }

    while (num > 0) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }

    // Print in reverse
    while (i--) {
        print_char(buf[i]);
    }
}

void print_hex(unsigned int num)
{
    char hex_chars[] = "0123456789ABCDEF";
    char buf[9]; // 8 digits max for 32-bit + null terminator
    int i = 0;

    if (num == 0) {
        print_string((uint8_t*)"0x0");
        return;
    }

    while (num > 0) {
        buf[i++] = hex_chars[num & 0xF];
        num >>= 4;
    }

    print_string((uint8_t*)"0x");
    while (i--) {
        print_char(buf[i]);
    }
}

void printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    for (const char* p = fmt; *p != '\0'; p++) {
        if (*p == '%') {
            p++; // look at next format specifier
            switch (*p) {
                case 's': {
                    const char* str = va_arg(args, const char*);
                    print_string((uint8_t*)str);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    print_char(c);
                    break;
                }
                case 'd': {
                    int num = va_arg(args, int);
                    print_int(num);
                    break;
                }
                case 'x': {
                    unsigned int num = va_arg(args, unsigned int);
                    print_hex(num);
                    break;
                }
                case '%': {
                    print_char('%');
                    break;
                }
                default:
                    print_char('%');
                    print_char(*p);
                    break;
            }
        } else {
            print_char(*p);
        }
    }

    va_end(args);
}
