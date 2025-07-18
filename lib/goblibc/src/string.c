
#include <string.h>
#include <stdint.h>

void *memcpy(void *dest, void *source, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++)
    {
        ((uint8_t *)dest)[i] = ((uint8_t *)source)[i];
    }
    return dest;
}

int strlen(const char *str)
{
    int i = 0;
    for (; str[i] != '\0'; i++)
        ;
    return i;
}

int strnlen(const char *str, int n)
{
    int i = 0;
    for (; str[i] != '\0' && i < n; i++)
        ;
    return i;
}

int memcmp(void *buf1, void *buf2, uint32_t size)
{
    int sum = 0;
    uint8_t *u8buf1 = buf1;
    uint8_t *u8buf2 = buf2;
    for (int i = 0; i < size; i++)
    {
        sum += u8buf1[i] - u8buf2[i];
    }
    return sum;
}

char *strcpy(char *str_dest, char *str_source)
{
    char *dest = str_dest;
    for (; (*dest = *str_source) != '\0'; dest++, str_source++)
        ;

    return str_dest;
}

char *strncpy(char *str_dest, char *str_source, int len)
{
    if (len == 0)
    {
        return str_dest;
    }
    char *dest = str_dest;
    for (len--; (*dest = *str_source) != '\0' && len > 0; dest++, str_source++, len--)
        ;
    // pad rest of the string with zeros. OpenBSD does it, idk how correct that actually is
    while (--len > 0)
    {
        *dest++ = '\0';
    }
    return str_dest;
}

int strcmp(const char *p1, const char *p2)
{
    const unsigned char *s1 = (const unsigned char *)p1;
    const unsigned char *s2 = (const unsigned char *)p2;
    unsigned char c1, c2;

    do
    {
        c1 = (unsigned char)*s1++;
        c2 = (unsigned char)*s2++;
        if (c1 == '\0')
            return c1 - c2;
    } while (c1 == c2);

    return c1 - c2;
}

int strncmp(const char *p1, const char *p2, int len)
{
    const unsigned char *s1 = (const unsigned char *)p1;
    const unsigned char *s2 = (const unsigned char *)p2;
    unsigned char c1, c2;
    int counter = 0;
    do
    {
        c1 = (unsigned char)*s1++;
        c2 = (unsigned char)*s2++;
        if (c1 == '\0')
            return c1 - c2;
        counter++;
    } while (c1 == c2 && counter < len);

    return c1 - c2;
}

char *strstr(char *str, char *substr)
{
    int len = strlen(str);
    int sublen = strlen(substr);
    char *ret = str;
    for (int i = 0; i <= len - sublen; i++)
    {
        if (strcmp(&str[i], substr) == 0)
        {
            ret = &str[i];
            break;
        }
    }
    return ret;
}

void *memset(void *buf, int n, uint32_t size)
{
    for (uint8_t *i = buf; i < (buf + size); i++)
    {
        *i = n;
    }
    return buf;
}
