#include "memutils.h"
#include <stdint.h>

void* memcpy(void* dest, void* source, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++) {
        ((uint8_t*)dest)[i] = ((uint8_t*)source)[i];
    }
    return dest;
}

int strlen(char* str)
{
    int i = 0;
    for (; str[i] != '\0'; i++)
        ;
    return i;
}

int memcmp(void* buf1, void* buf2, uint32_t size)
{
    int sum = 0;
    uint8_t* u8buf1 = buf1;
    uint8_t* u8buf2 = buf2;
    for (int i = 0; i < size; i++) {
        sum += u8buf1[i] - u8buf2[i];
    }
    return sum;
}

char* strcpy(char* str_dest, char* str_source)
{
    int len1 = strlen(str_dest);
    int len2 = strlen(str_source);
    int len = len1;
    if (len2 < len1) {
        len = len2;
    }
    return memcpy(str_dest, str_source, len);
}

char* strncpy(char* str_dest, char* str_source, uint32_t len)
{
    return memcpy(str_dest, str_source, len);
}

int strcmp(const char* p1, const char* p2)
{
    const unsigned char* s1 = (const unsigned char*)p1;
    const unsigned char* s2 = (const unsigned char*)p2;
    unsigned char c1, c2;

    do {
        c1 = (unsigned char)*s1++;
        c2 = (unsigned char)*s2++;
        if (c1 == '\0')
            return c1 - c2;
    } while (c1 == c2);

    return c1 - c2;
}

int strncmp(const char* p1, const char* p2, uint32_t len)
{
    const unsigned char* s1 = (const unsigned char*)p1;
    const unsigned char* s2 = (const unsigned char*)p2;
    unsigned char c1, c2;
    int counter = 0;
    do {
        c1 = (unsigned char)*s1++;
        c2 = (unsigned char)*s2++;
        if (c1 == '\0')
            return c1 - c2;
        counter++;
    } while (c1 == c2 && counter < len);

    return c1 - c2;
}

char* strstr(char* str, char* substr)
{
    int len = strlen(str);
    int sublen = strlen(substr);
    char* ret = str;
    for (int i = 0; i <= len - sublen; i++) {
        if (strcmp(&str[i], substr) == 0) {
            ret = &str[i];
            break;
        }
    }
    return ret;
}

void* memset(void* buf, int n, uint32_t size)
{
    for (uint8_t* i = buf; i < (buf + size); i++) {
        *i = n;
    }
    return buf;
}

char* strchr(char* str, char c)
{
    int len = strlen(str);
    for (char* i = str; i < str + len; i++) {
        if (*i == c) {
            return i;
        }
    }
    return NULL;
}
