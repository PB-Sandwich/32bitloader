#include <stdint.h>

void print(const char *str, uint32_t len)
{
    uint32_t stdout = 0;
    asm("int $0x40" : "=b"(stdout) : "a"(1));
    asm("int $0x40" ::"a"(5), "b"(stdout), "c"(len), "d"(str) : "memory");
}

void clear()
{
}

int main()
{
    const char msg[] = "hello world";
    // clear();
    print(msg, sizeof(msg));
    return 0;
}