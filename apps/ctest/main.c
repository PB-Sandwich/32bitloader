#include <stdint.h>

#include <string.h>
void print(const char *str, uint32_t len)
{
    uint32_t stdout = 0;
    asm("int $0x40" : "=b"(stdout) : "a"(1));
    asm("int $0x40" ::"a"(5), "b"(stdout), "c"(len), "d"(str) : "memory");
}

uint32_t read(char *buffer, uint32_t len)
{
    uint32_t stdin = 0;
    uint32_t res = 0;
    asm("int $0x40" : "=c"(stdin) : "a"(1));
    asm("int $0x40" : "=a"(res) : "a"(4), "b"(stdin), "c"(len), "d"(buffer) : "memory");
    return res;
}

void clear()
{
}

int main()
{
    const char msg[] = "hello world. Type stuff: ";
    const char msg2[] = "\n   your input:";
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    // clear();
    print(msg, sizeof(msg));
    uint32_t bytes_read = read(buffer, 10);
    print(msg2, sizeof(msg2));
    print(buffer, 10);
    return 0;
}