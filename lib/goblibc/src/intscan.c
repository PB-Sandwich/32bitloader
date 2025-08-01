#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <file.h>
#include <scan_helpers.h>

/* Lookup table for digit values. -1==255>=36 -> invalid */

static const unsigned char table[] = { -1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,
-1,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
25,26,27,28,29,30,31,32,33,34,35,-1,-1,-1,-1,-1,
-1,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
25,26,27,28,29,30,31,32,33,34,35,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
};

unsigned long long gob_intscan(FILE *file, unsigned base, int pok, unsigned long long lim)
{
    const unsigned char *val = table + 1;
    int c, neg = 0;
    unsigned x;
    unsigned long long y;
    if (base > 36 || base == 1)
    {
        errno = EINVAL;
        return 0;
    }
    while (isspace((c = scan_help_getc(file))))
        ;
    if (c == '+' || c == '-')
    {
        neg = -(c == '-');
        c = scan_help_getc(file);
    }
    if ((base == 0 || base == 16) && c == '0')
    {
        c = scan_help_getc(file);
        if ((c | 32) == 'x')
        {
            c = scan_help_getc(file);
            if (val[c] >= 16)
            {
                scan_help_unget(file);
                if (pok)
                    scan_help_unget(file);
                else
                    scan_help_lim(file, 0);
                return 0;
            }
            base = 16;
        }
        else if (base == 0)
        {
            base = 8;
        }
    }
    else
    {
        if (base == 0)
            base = 10;
        if (val[c] >= base)
        {
            scan_help_unget(file);
            scan_help_lim(file, 0);
            errno = EINVAL;
            return 0;
        }
    }
    if (base == 10)
    {
        for (x = 0; c - '0' < 10U && x <= UINT_MAX / 10 - 1; c = scan_help_getc(file))
            x = x * 10 + (c - '0');
        for (y = x; c - '0' < 10U && y <= ULLONG_MAX / 10 && 10 * y <= ULLONG_MAX - (c - '0'); c = scan_help_getc(file))
            y = y * 10 + (c - '0');
        if (c - '0' >= 10U)
            goto done;
    }
    else if (!(base & base - 1))
    {
        int bs = "\0\1\2\4\7\3\6\5"[(0x17 * base) >> 5 & 7];
        for (x = 0; val[c] < base && x <= UINT_MAX / 32; c = scan_help_getc(file))
            x = x << bs | val[c];
        for (y = x; val[c] < base && y <= ULLONG_MAX >> bs; c = scan_help_getc(file))
            y = y << bs | val[c];
    }
    else
    {
        for (x = 0; val[c] < base && x <= UINT_MAX / 36 - 1; c = scan_help_getc(file))
            x = x * base + val[c];
        for (y = x; val[c] < base && y <= ULLONG_MAX / base && base * y <= ULLONG_MAX - val[c]; c = scan_help_getc(file))
            y = y * base + val[c];
    }
    if (val[c] < base)
    {
        for (; val[c] < base; c = scan_help_getc(file))
            ;
        errno = ERANGE;
        y = lim;
        if (lim & 1)
            neg = 0;
    }
done:
    scan_help_unget(file);
    if (y >= lim)
    {
        if (!(lim & 1) && !neg)
        {
            errno = ERANGE;
            return lim - 1;
        }
        else if (y > lim)
        {
            errno = ERANGE;
            return lim;
        }
    }
    return (y ^ neg) - neg;
}
