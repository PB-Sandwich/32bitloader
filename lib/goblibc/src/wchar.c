#include <wchar.h>
#include <bits/alltypes.h>
#include <errno.h>
#include <mkinternal.h>

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

size_t mbrtowc(wchar_t *restrict wc, const char *restrict src, size_t n, mbstate_t *restrict st)
{
    static unsigned internal_state;
    unsigned c;
    const unsigned char *s = (const void *)src;
    const size_t N = n;
    wchar_t dummy;

    if (!st)
        st = (void *)&internal_state;
    c = *(unsigned *)st;

    if (!s)
    {
        if (c)
            goto ilseq;
        return 0;
    }
    else if (!wc)
        wc = &dummy;

    if (!n)
        return -2;
    if (!c)
    {
        if (*s < 0x80)
            return !!(*wc = *s);
        if (MB_CUR_MAX == 1)
            return (*wc = CODEUNIT(*s)), 1;
        if (*s - SA > SB - SA)
            goto ilseq;
        c = bittab[*s++ - SA];
        n--;
    }

    if (n)
    {
        if (OOB(c, *s))
            goto ilseq;
    loop:
        c = c << 6 | *s++ - 0x80;
        n--;
        if (!(c & (1U << 31)))
        {
            *(unsigned *)st = 0;
            *wc = c;
            return N - n;
        }
        if (n)
        {
            if (*s - 0x80u >= 0x40)
                goto ilseq;
            goto loop;
        }
    }

    *(unsigned *)st = c;
    return -2;
ilseq:
    *(unsigned *)st = 0;
    errno = EILSEQ;
    return -1;
}

int mbsinit(const mbstate_t *st)
{
    return !st || !*(unsigned *)st;
}

int wctomb(char *s, wchar_t wc)
{
    if (!s)
        return 0;
    return wcrtomb(s, wc);
}