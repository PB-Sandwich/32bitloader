#include <ctype.h>


int tolower(int c)
{
    return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c;
}

int toupper(int c)
{
    return (c >= 'a' && c <= 'z') ? (c - ('a' - 'A')) : c;
}

int isalnum(int c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}

int isalpha(int c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int isdigit(int c)
{
    return (c >= '0' && c <= '9');
}