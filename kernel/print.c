#include <filesystem/virtual-filesystem.h>
#include <memutils.h>
#include <stdarg.h>
#include <stdint.h>
#include <terminal/tty.h>

VFSFile* tty;

void set_print_output(char* path)
{
    if (tty != NULL) {
        vfs_close_file(tty);
    }
    tty = vfs_open_file(path, VFS_READ | VFS_WRITE | VFS_APPEND);
}

void pprint_char(uint8_t chr)
{
    vfs_write(tty, &chr, 1);
}

void pprint_string(uint8_t* str)
{
    vfs_write(tty, str, strlen((char*)str));
}

void print_int(int num)
{
    char buf[12]; // enough for -2,147,483,648
    int i = 0;

    if (num == 0) {
        pprint_char('0');
        return;
    }

    if (num < 0) {
        pprint_char('-');
        num = -num;
    }

    while (num > 0) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }

    // Print in reverse
    while (i--) {
        pprint_char(buf[i]);
    }
}

void print_hex(unsigned int num)
{
    char hex_chars[] = "0123456789ABCDEF";
    char buf[9]; // 8 digits max for 32-bit + null terminator
    int i = 0;

    if (num == 0) {
        pprint_string((uint8_t*)"0");
        return;
    }

    while (num > 0) {
        buf[i++] = hex_chars[num & 0xF];
        num >>= 4;
    }

    while (i--) {
        pprint_char(buf[i]);
    }
}

void printf(const char* fmt, ...)
{
    if (tty == NULL) {
        return;
    }

    va_list args;
    va_start(args, fmt);

    for (const char* p = fmt; *p != '\0'; p++) {
        if (*p == '%') {
            p++; // look at next format specifier
            switch (*p) {
            case 's': {
                const char* str = va_arg(args, const char*);
                pprint_string((uint8_t*)str);
                break;
            }
            case 'c': {
                char c = (char)va_arg(args, int);
                pprint_char(c);
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
                pprint_char('%');
                break;
            }
            default:
                pprint_char('%');
                pprint_char(*p);
                break;
            }
        } else {
            pprint_char(*p);
        }
    }

    va_end(args);
}
