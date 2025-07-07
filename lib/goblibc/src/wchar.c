#include <wchar.h>
#include <bits/alltypes.h>
#include <errno.h>
#define IS_CODEUNIT(c) ((unsigned)(c)-0xdf80 < 0x80)

size_t wcrtomb(char *restrict s, wchar_t wc)
{
    if (!s)
        return 1;
    if ((unsigned)wc < 0x80)
    {
        *s = wc;
        return 1;
    }
    else if (MB_CUR_MAX == 1)
    {
        if (!IS_CODEUNIT(wc))
        {
            errno = EILSEQ;
            return -1;
        }
        *s = wc;
        return 1;
    }
    else if ((unsigned)wc < 0x800)
    {
        *s++ = 0xc0 | (wc >> 6);
        *s = 0x80 | (wc & 0x3f);
        return 2;
    }
    else if ((unsigned)wc < 0xd800 || (unsigned)wc - 0xe000 < 0x2000)
    {
        *s++ = 0xe0 | (wc >> 12);
        *s++ = 0x80 | ((wc >> 6) & 0x3f);
        *s = 0x80 | (wc & 0x3f);
        return 3;
    }
    else if ((unsigned)wc - 0x10000 < 0x100000)
    {
        *s++ = 0xf0 | (wc >> 18);
        *s++ = 0x80 | ((wc >> 12) & 0x3f);
        *s++ = 0x80 | ((wc >> 6) & 0x3f);
        *s = 0x80 | (wc & 0x3f);
        return 4;
    }
    errno = EILSEQ;
    return -1;
}

int wctomb(char *s, wchar_t wc)
{
    if (!s)
        return 0;
    return wcrtomb(s, wc);
}