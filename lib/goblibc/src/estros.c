#include <estros.h>

uint16_t *get_text_buffer_address()
{
    return (uint16_t *)0xb8000;
}

void sys_print(const char *str, uint32_t len)
{
    uint32_t stdout = 0;
    asm("int $0x40" : "=b"(stdout) : "a"(1));
    asm("int $0x40" ::"a"(5), "b"(stdout), "c"(len), "d"(str) : "memory");
}

uint32_t sys_read(char *buffer, uint32_t len)
{
    uint32_t stdin = 0;
    uint32_t res = 0;
    asm("int $0x40" : "=c"(stdin) : "a"(1));
    asm("int $0x40" : "=a"(res) : "a"(4), "b"(stdin), "c"(len), "d"(buffer) : "memory");
    return res;
}
