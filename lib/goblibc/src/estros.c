#include <estros.h>

uint16_t *get_text_buffer_address()
{
    return (uint16_t *)0xb8000;
}

void sys_print(const char *str, uint32_t len)
{
    __asm__ volatile("int $0x40" ::"a"(5), "b"(estros_stdout), "c"(len), "d"(str) : "memory");
}

uint32_t sys_read(char *buffer, uint32_t len)
{
    uint32_t res = 0;
    __asm__ volatile("int $0x40" : "=a"(res) : "a"(4), "b"(estros_stdin), "c"(len), "d"(buffer) : "memory");
    return res;
}

void sys_clear()
{
    uint32_t command = 0;
    __asm__ volatile("int $0x40" ::"a"(6), "b"(estros_stdout), "c"(&command));
}

void sys_set_cursor(uint16_t x, uint16_t y)
{
    uint32_t command = 3;
    uint32_t arg = (x << 16) | y;
    __asm__ volatile("int $0x40" ::"a"(6), "b"(estros_stdout), "c"(&command), "d"(arg));
}
