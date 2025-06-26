
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

int strcmp(char* str1, char* str2)
{
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    int len = len1;
    if (len2 < len1) {
        len = len2;
    }
    return memcmp(str1, str2, len);
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

void* memset(void* buf, int n, uint32_t size) {
    for (uint8_t* i = buf; i < (buf + size); i++) {
        *i = n;
    }
    return buf;
}
