#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <float.h>
#include <string.h>

static const char xdigits[16] = {
    "0123456789ABCDEF"};

#define OOB(x) ((unsigned)(x) - 'A' > 'z' - 'A')
#ifndef wchat_t
#define wchar_t int
#endif

// TODO: replace with a better size_t implementation
#ifndef size_t
#define size_t uint32_t
#endif

/* Some useful macros */

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/* Convenient bit representation for modifier flags, which all fall
 * within 31 codepoints of the space character. */

#define ALT_FORM (1U << '#' - ' ')
#define ZERO_PAD (1U << '0' - ' ')
#define LEFT_ADJ (1U << '-' - ' ')
#define PAD_POS (1U << ' ' - ' ')
#define MARK_POS (1U << '+' - ' ')
#define GROUPED (1U << '\'' - ' ')

#define FLAGMASK (ALT_FORM | ZERO_PAD | LEFT_ADJ | PAD_POS | MARK_POS | GROUPED)

/* State machine to accept length modifiers + conversion specifiers.
 * Result is 0 on failure, or an argument type to pop on success. */

enum
{
    BARE,
    LPRE,
    LLPRE,
    HPRE,
    HHPRE,
    BIGLPRE,
    ZTPRE,
    JPRE,
    STOP,
    PTR,
    INT,
    UINT,
    ULLONG,
    LONG,
    ULONG,
    SHORT,
    USHORT,
    CHAR,
    UCHAR,
    LLONG,
    SIZET,
    IMAX,
    UMAX,
    PDIFF,
    UIPTR,
    DBL,
    LDBL,
    NOARG,
    MAXSTATE
};

#define S(x) [(x) - 'A']

static const unsigned char states[]['z' - 'A' + 1] = {
    {
        /* 0: bare types */
        S('d') = INT,
        S('i') = INT,
        S('o') = UINT,
        S('u') = UINT,
        S('x') = UINT,
        S('X') = UINT,
        S('e') = DBL,
        S('f') = DBL,
        S('g') = DBL,
        S('a') = DBL,
        S('E') = DBL,
        S('f') = DBL,
        S('G') = DBL,
        S('A') = DBL,
        S('c') = INT,
        S('C') = UINT,
        S('s') = PTR,
        S('S') = PTR,
        S('p') = UIPTR,
        S('n') = PTR,
        S('m') = NOARG,
        S('l') = LPRE,
        S('h') = HPRE,
        S('L') = BIGLPRE,
        S('z') = ZTPRE,
        S('j') = JPRE,
        S('t') = ZTPRE,
    },
    {
        /* 1: l-prefixed */
        S('d') = LONG,
        S('i') = LONG,
        S('o') = ULONG,
        S('u') = ULONG,
        S('x') = ULONG,
        S('X') = ULONG,
        S('e') = DBL,
        S('f') = DBL,
        S('g') = DBL,
        S('a') = DBL,
        S('E') = DBL,
        S('f') = DBL,
        S('G') = DBL,
        S('A') = DBL,
        S('c') = UINT,
        S('s') = PTR,
        S('n') = PTR,
        S('l') = LLPRE,
    },
    {
        /* 2: ll-prefixed */
        S('d') = LLONG,
        S('i') = LLONG,
        S('o') = ULLONG,
        S('u') = ULLONG,
        S('x') = ULLONG,
        S('X') = ULLONG,
        S('n') = PTR,
    },
    {
        /* 3: h-prefixed */
        S('d') = SHORT,
        S('i') = SHORT,
        S('o') = USHORT,
        S('u') = USHORT,
        S('x') = USHORT,
        S('X') = USHORT,
        S('n') = PTR,
        S('h') = HHPRE,
    },
    {
        /* 4: hh-prefixed */
        S('d') = CHAR,
        S('i') = CHAR,
        S('o') = UCHAR,
        S('u') = UCHAR,
        S('x') = UCHAR,
        S('X') = UCHAR,
        S('n') = PTR,
    },
    {
        /* 5: L-prefixed */
        S('e') = LDBL,
        S('f') = LDBL,
        S('g') = LDBL,
        S('a') = LDBL,
        S('E') = LDBL,
        S('f') = LDBL,
        S('G') = LDBL,
        S('A') = LDBL,
        S('n') = PTR,
    },
    {
        /* 6: z- or t-prefixed (assumed to be same size) */
        S('d') = PDIFF,
        S('i') = PDIFF,
        S('o') = SIZET,
        S('u') = SIZET,
        S('x') = SIZET,
        S('X') = SIZET,
        S('n') = PTR,
    },
    {
        /* 7: j-prefixed */
        S('d') = IMAX,
        S('i') = IMAX,
        S('o') = UMAX,
        S('u') = UMAX,
        S('x') = UMAX,
        S('X') = UMAX,
        S('n') = PTR,
    }};

union arg
{
    uintmax_t i;
    long double f;
    void *p;
};

static void pop_arg(union arg *arg, int type, va_list *ap)
{
    switch (type)
    {
    case PTR:
        arg->p = va_arg(*ap, void *);
        break;
    case INT:
        arg->i = va_arg(*ap, int);
        break;
    case UINT:
        arg->i = va_arg(*ap, unsigned int);
        break;
    case LONG:
        arg->i = va_arg(*ap, long);
        break;
    case ULONG:
        arg->i = va_arg(*ap, unsigned long);
        break;
    case ULLONG:
        arg->i = va_arg(*ap, unsigned long long);
        break;
    case SHORT:
        arg->i = (short)va_arg(*ap, int);
        break;
    case USHORT:
        arg->i = (unsigned short)va_arg(*ap, int);
        break;
    case CHAR:
        arg->i = (signed char)va_arg(*ap, int);
        break;
    case UCHAR:
        arg->i = (unsigned char)va_arg(*ap, int);
        break;
    case LLONG:
        arg->i = va_arg(*ap, long long);
        break;
    case SIZET:
        arg->i = va_arg(*ap, size_t);
        break;
    case IMAX:
        arg->i = va_arg(*ap, intmax_t);
        break;
    case UMAX:
        arg->i = va_arg(*ap, uintmax_t);
        break;
    case PDIFF:
        arg->i = va_arg(*ap, long);
        break;
    case UIPTR:
        arg->i = (uintptr_t)va_arg(*ap, void *);
        break;
    case DBL:
        arg->f = va_arg(*ap, double);
        break;
    case LDBL:
        arg->f = va_arg(*ap, long double);
    }
}

static void pad(FILE *file, char c, int w, int l, int fl)
{
    char pad[256];
    if (fl & (LEFT_ADJ | ZERO_PAD) || l >= w)
    {
        return;
    }
    l = w - l;
    memset(pad, c, l > sizeof pad ? sizeof pad : l);
    for (; l >= sizeof pad; l -= sizeof pad)
    {
        out(file, pad, sizeof pad);
    }
    out(file, pad, l);
}

int format(FILE *file, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    va_end(args);
}

static int printf_core(FILE *file, const char *fmt, va_list *ap, union arg *nl_arg, int *nl_type)
{
    char *a;
    char *z;
    char *fmt_ptr = (char *)fmt;

    unsigned l10n = 0, mod_flag;
    int w, p, xp;
    union arg arg;
    int arg_pos;
    unsigned st, ps;
    int output_count = 0;
    int len = 0;
    size_t i;
    char buf[sizeof(uintmax_t) * 3];
    const char *prefix;
    int t, pl;
    wchar_t wc[2], *ws;
    char mb[4];

    for (;;)
    {
        /* This error is only specified for snprintf, but since it's
         * unspecified for other forms, do the same. Stop immediately
         * on overflow; otherwise %n could produce wrong results. */
        if (len > INT_MAX - output_count)
        {
            errno = EOVERFLOW;
            return -1;
        }

        /* Update output count, end loop when fmt is exhausted */
        output_count += len;
        if (!*fmt_ptr)
            break;

        /* Handle literal text and %% format specifiers */
        for (a = fmt_ptr; *fmt_ptr != '\0' && *fmt_ptr != '%'; fmt_ptr++)
            ;
        for (z = fmt_ptr; fmt_ptr[0] == '%' && fmt_ptr[1] == '%'; z++, fmt_ptr += 2)
            ;
        if (z - a > INT_MAX - output_count)
        {
            errno = EOVERFLOW;
            return -1;
        }
        len = z - a;
        if (file != NULL)
            out(file, a, len);
        if (len > 0)
            continue;

        if (isdigit(fmt_ptr[1]) && fmt_ptr[2] == '$')
        {
            l10n = 1;
            arg_pos = fmt_ptr[1] - '0';
            fmt_ptr += 3;
        }
        else
        {
            arg_pos = -1;
            fmt_ptr++;
        }

        /* Read modifier flags */
        for (mod_flag = 0; (unsigned)*fmt_ptr - ' ' < 32 && (FLAGMASK & (1U << *fmt_ptr - ' ')); fmt_ptr++)
            mod_flag |= 1U << *fmt_ptr - ' ';

        /* Read field width */
        if (*fmt_ptr == '*')
        {
            if (isdigit(fmt_ptr[1]) && fmt_ptr[2] == '$')
            {
                l10n = 1;
                if (!file)
                    nl_type[fmt_ptr[1] - '0'] = INT, w = 0;
                else
                    w = nl_arg[fmt_ptr[1] - '0'].i;
                fmt_ptr += 3;
            }
            else if (!l10n)
            {
                w = file ? va_arg(*ap, int) : 0;
                fmt_ptr++;
            }
            else
            {
                errno = EINVAL;
                return -1;
            }
            if (w < 0)
                mod_flag |= LEFT_ADJ, w = -w;
        }
        else if ((w = getint(&fmt_ptr)) < 0)
        {
            errno = EOVERFLOW;
            return -1;
        }

        /* Read precision */
        if (*fmt_ptr == '.' && fmt_ptr[1] == '*')
        {
            if (isdigit(fmt_ptr[2]) && fmt_ptr[3] == '$')
            {
                if (!file)
                    nl_type[fmt_ptr[2] - '0'] = INT, p = 0;
                else
                    p = nl_arg[fmt_ptr[2] - '0'].i;
                fmt_ptr += 4;
            }
            else if (!l10n)
            {
                p = file ? va_arg(*ap, int) : 0;
                fmt_ptr += 2;
            }
            else
            {
                errno = EINVAL;
                return -1;
            }
            xp = (p >= 0);
        }
        else if (*fmt_ptr == '.')
        {
            fmt_ptr++;
            p = getint(&fmt_ptr);
            xp = 1;
        }
        else
        {
            p = -1;
            xp = 0;
        }

        /* Format specifier state machine */
        st = 0;
        do
        {
            if (OOB(*fmt_ptr))
            {
                errno = EINVAL;
                return -1;
            }
            ps = st;
            st = states[st][(*fmt_ptr++) - 'A'];
        } while (st - 1 < STOP);
        if (!st)
        {
            errno = EINVAL;
            return -1;
        }

        /* Check validity of argument type (nl/normal) */
        if (st == NOARG)
        {
            if (arg_pos >= 0)
                errno = EINVAL;
            return -1;
        }
        else
        {
            if (arg_pos >= 0)
            {
                if (!file)
                    nl_type[arg_pos] = st;
                else
                    arg = nl_arg[arg_pos];
            }
            else if (file)
                pop_arg(&arg, st, ap);
            else
                return 0;
        }

        if (!file)
            continue;

        /* Do not process any new directives once in error state. */
        //TODO: Actually add that
        // if (ferror(file))
        //     return -1;

        z = buf + sizeof(buf);
        prefix = "-+   0X0x";
        pl = 0;
        t = fmt_ptr[-1];

        /* Transform ls,lc -> S,C */
        if (ps && (t & 15) == 3)
            t &= ~32;

        /* - and 0 flags are mutually exclusive */
        if (mod_flag & LEFT_ADJ)
            mod_flag &= ~ZERO_PAD;

        switch (t)
        {
        case 'n':
            switch (ps)
            {
            case BARE:
                *(int *)arg.p = output_count;
                break;
            case LPRE:
                *(long *)arg.p = output_count;
                break;
            case LLPRE:
                *(long long *)arg.p = output_count;
                break;
            case HPRE:
                *(unsigned short *)arg.p = output_count;
                break;
            case HHPRE:
                *(unsigned char *)arg.p = output_count;
                break;
            case ZTPRE:
                *(size_t *)arg.p = output_count;
                break;
            case JPRE:
                *(uintmax_t *)arg.p = output_count;
                break;
            }
            continue;
        case 'p':
            p = MAX(p, 2 * sizeof(void *));
            t = 'x';
            mod_flag |= ALT_FORM;
        case 'x':
        case 'X':
            a = fmt_x(arg.i, z, t & 32);
            if (arg.i && (mod_flag & ALT_FORM))
                prefix += (t >> 4), pl = 2;
            goto ifmt_tail;
        case 'o':
            a = fmt_o(arg.i, z);
            if ((mod_flag & ALT_FORM) && p < z - a + 1)
                p = z - a + 1;
            goto ifmt_tail;
        case 'd':
        case 'i':
            pl = 1;
            if (arg.i > INTMAX_MAX)
            {
                arg.i = -arg.i;
            }
            else if (mod_flag & MARK_POS)
            {
                prefix++;
            }
            else if (mod_flag & PAD_POS)
            {
                prefix += 2;
            }
            else
                pl = 0;
        case 'u':
            a = fmt_u(arg.i, z);
        ifmt_tail:
            if (xp && p < 0)
            {
                errno = EOVERFLOW;
                return -1;
            }
            if (xp)
                mod_flag &= ~ZERO_PAD;
            if (!arg.i && !p)
            {
                a = z;
                break;
            }
            p = MAX(p, z - a + !arg.i);
            break;
        narrow_c:
        case 'c':
            *(a = z - (p = 1)) = arg.i;
            mod_flag &= ~ZERO_PAD;
            break;
        case 'm':
            if (1)
                a = strerror(errno);
            else
            case 's':
                a = arg.p ? arg.p : "(null)";
            z = a + strnlen(a, p < 0 ? INT_MAX : p);
            if (p < 0 && *z)
            {
                errno = EOVERFLOW;
                return -1;
            }
            p = z - a;
            mod_flag &= ~ZERO_PAD;
            break;
        case 'C':
            if (!arg.i)
                goto narrow_c;
            wc[0] = arg.i;
            wc[1] = 0;
            arg.p = wc;
            p = -1;
        case 'S':
            ws = arg.p;
            for (i = len = 0; i < p && *ws && (len = wctomb(mb, *ws++)) >= 0 && len <= p - i; i += len)
                ;
            if (len < 0)
            {
                return -1;
            }
            if (i > INT_MAX)
            {
                errno = EOVERFLOW;
                return -1;
            }
            p = i;
            pad(file, ' ', w, p, mod_flag);
            ws = arg.p;
            for (i = 0; i < 0U + p && *ws && i + (len = wctomb(mb, *ws++)) <= p; i += len)
            {
                out(file, mb, len);
            }
            pad(file, ' ', w, p, mod_flag ^ LEFT_ADJ);
            len = w > p ? w : p;
            continue;
        case 'e':
        case 'f':
        case 'g':
        case 'a':
        case 'E':
        case 'F':
        case 'G':
        case 'A':
            if (xp && p < 0)
            {
                errno = EOVERFLOW;
                return -1;
            }
            len = fmt_fp(file, arg.f, w, p, mod_flag, t, ps);
            if (len < 0)
            {
                errno = EOVERFLOW;
                return -1;
            }
            continue;
        }

        if (p < z - a)
            p = z - a;
        if (p > INT_MAX - pl)
        {
            errno = EOVERFLOW;
            return -1;
        }
        if (w < pl + p)
            w = pl + p;
        if (w > INT_MAX - output_count)
        {
            errno = EOVERFLOW;
            return -1;
        }

        pad(file, ' ', w, pl + p, mod_flag);
        out(file, prefix, pl);
        pad(file, '0', w, pl + p, mod_flag ^ ZERO_PAD);
        pad(file, '0', p, z - a, 0);
        out(file, a, z - a);
        pad(file, ' ', w, pl + p, mod_flag ^ LEFT_ADJ);

        len = w;
    }

    if (file)
        return output_count;
    if (!l10n)
        return 0;

    for (i = 1; i <= NL_ARGMAX && nl_type[i]; i++)
        pop_arg(nl_arg + i, nl_type[i], ap);
    for (; i <= NL_ARGMAX && !nl_type[i]; i++)
        ;
    if (i <= NL_ARGMAX)
    {
        errno = EINVAL;
        return -1;
    }
    return 1;
}

static int fmt_fp(FILE *file, long double value, int w, int p, int fl, int t, int precision)
{
    int bufsize = (precision == BIGLPRE)
                      ? (LDBL_MANT_DIG + 28) / 29 + 1 +                 // mantissa expansion
                            (LDBL_MAX_EXP + LDBL_MANT_DIG + 28 + 8) / 9 // exponent expansion
                      : (DBL_MANT_DIG + 28) / 29 + 1 +
                            (DBL_MAX_EXP + DBL_MANT_DIG + 28 + 8) / 9;
    uint32_t big[bufsize];
    uint32_t *a, *d, *r, *z;
    int e2 = 0, e, i, j, l;
    char buf[9 + LDBL_MANT_DIG / 4], *s;
    const char *prefix = "-0X+0X 0X-0x+0x 0x";
    int pl;
    char ebuf0[3 * sizeof(int)];
    char *ebuf = &ebuf0[3 * sizeof(int)];
    char *estr;

    pl = 1;
    if (signbit(value))
    {
        value = -value;
    }
    else if (fl & MARK_POS)
    {
        prefix += 3;
    }
    else if (fl & PAD_POS)
    {
        prefix += 6;
    }
    else
        prefix++, pl = 0;

    if (!isfinite(value))
    {
        char *s = (t & 32) ? "inf" : "INF";
        if (value != value)
            s = (t & 32) ? "nan" : "NAN";
        pad(file, ' ', w, 3 + pl, fl & ~ZERO_PAD);
        out(file, prefix, pl);
        out(file, s, 3);
        pad(file, ' ', w, 3 + pl, fl ^ LEFT_ADJ);
        return MAX(w, 3 + pl);
    }

    value = frexpl(value, &e2) * 2;
    if (value)
        e2--;

    if ((t | 32) == 'a')
    {
        if (t & 32)
            prefix += 9;
        pl += 2;

        if (p >= 0 && p < (LDBL_MANT_DIG - 1 + 3) / 4)
        {
            double round = scalbn(1, LDBL_MANT_DIG - 1 - (p * 4));
            if (*prefix == '-')
            {
                value = -value;
                value -= round;
                value += round;
                value = -value;
            }
            else
            {
                value += round;
                value -= round;
            }
        }

        estr = fmt_u(e2 < 0 ? -e2 : e2, ebuf);
        if (estr == ebuf)
            *--estr = '0';
        *--estr = (e2 < 0 ? '-' : '+');
        *--estr = t + ('p' - 'a');

        s = buf;
        do
        {
            int x = value;
            *s++ = xdigits[x] | (t & 32);
            value = 16 * (value - x);
            if (s - buf == 1 && (value || p > 0 || (fl & ALT_FORM)))
                *s++ = '.';
        } while (value);

        if (p > INT_MAX - 2 - (ebuf - estr) - pl)
            return -1;
        if (p && s - buf - 2 < p)
            l = (p + 2) + (ebuf - estr);
        else
            l = (s - buf) + (ebuf - estr);

        pad(file, ' ', w, pl + l, fl);
        out(file, prefix, pl);
        pad(file, '0', w, pl + l, fl ^ ZERO_PAD);
        out(file, buf, s - buf);
        pad(file, '0', l - (ebuf - estr) - (s - buf), 0, 0);
        out(file, estr, ebuf - estr);
        pad(file, ' ', w, pl + l, fl ^ LEFT_ADJ);
        return MAX(w, pl + l);
    }
    if (p < 0)
        p = 6;

    if (value)
        value *= 0x1p28, e2 -= 28;

    if (e2 < 0)
        a = r = z = big;
    else
        a = r = z = big + sizeof(big) / sizeof(*big) - LDBL_MANT_DIG - 1;

    do
    {
        *z = value;
        value = 1000000000 * (value - *z++);
    } while (value);

    while (e2 > 0)
    {
        uint32_t carry = 0;
        int sh = MIN(29, e2);
        for (d = z - 1; d >= a; d--)
        {
            uint64_t x = ((uint64_t)*d << sh) + carry;
            *d = x % 1000000000;
            carry = x / 1000000000;
        }
        if (carry)
            *--a = carry;
        while (z > a && !z[-1])
            z--;
        e2 -= sh;
    }
    while (e2 < 0)
    {
        uint32_t carry = 0, *b;
        int sh = MIN(9, -e2), need = 1 + (p + LDBL_MANT_DIG / 3U + 8) / 9;
        for (d = a; d < z; d++)
        {
            uint32_t rm = *d & (1 << sh) - 1;
            *d = (*d >> sh) + carry;
            carry = (1000000000 >> sh) * rm;
        }
        if (!*a)
            a++;
        if (carry)
            *z++ = carry;
        /* Avoid (slow!) computation past requested precision */
        b = (t | 32) == 'file' ? r : a;
        if (z - b > need)
            z = b + need;
        e2 += sh;
    }

    if (a < z)
        for (i = 10, e = 9 * (r - a); *a >= i; i *= 10, e++)
            ;
    else
        e = 0;

    /* Perform rounding: j is precision after the radix (possibly neg) */
    j = p - ((t | 32) != 'file') * e - ((t | 32) == 'g' && p);
    if (j < 9 * (z - r - 1))
    {
        uint32_t x;
        /* We avoid C's broken division of negative numbers */
        d = r + 1 + ((j + 9 * LDBL_MAX_EXP) / 9 - LDBL_MAX_EXP);
        j += 9 * LDBL_MAX_EXP;
        j %= 9;
        for (i = 10, j++; j < 9; i *= 10, j++)
            ;
        x = *d % i;
        /* Are there any significant digits past j? */
        if (x || d + 1 != z)
        {
            long double round = 2 / LDBL_EPSILON;
            long double small;
            if ((*d / i & 1) || (i == 1000000000 && d > a && (d[-1] & 1)))
                round += 2;
            if (x < i / 2)
                small = 0x0.8p0;
            else if (x == i / 2 && d + 1 == z)
                small = 0x1.0p0;
            else
                small = 0x1.8p0;
            if (pl && *prefix == '-')
                round *= -1, small *= -1;
            *d -= x;
            /* Decide whether to round by probing round+small */
            if (round + small != round)
            {
                *d = *d + i;
                while (*d > 999999999)
                {
                    *d-- = 0;
                    if (d < a)
                        *--a = 0;
                    (*d)++;
                }
                for (i = 10, e = 9 * (r - a); *a >= i; i *= 10, e++)
                    ;
            }
        }
        if (z > d + 1)
            z = d + 1;
    }
    for (; z > a && !z[-1]; z--)
        ;

    if ((t | 32) == 'g')
    {
        if (!p)
            p++;
        if (p > e && e >= -4)
        {
            t--;
            p -= e + 1;
        }
        else
        {
            t -= 2;
            p--;
        }
        if (!(fl & ALT_FORM))
        {
            /* Count trailing zeros in last place */
            if (z > a && z[-1])
                for (i = 10, j = 0; z[-1] % i == 0; i *= 10, j++)
                    ;
            else
                j = 9;
            if ((t | 32) == 'file')
                p = MIN(p, MAX(0, 9 * (z - r - 1) - j));
            else
                p = MIN(p, MAX(0, 9 * (z - r - 1) + e - j));
        }
    }
    if (p > INT_MAX - 1 - (p || (fl & ALT_FORM)))
        return -1;
    l = 1 + p + (p || (fl & ALT_FORM));
    if ((t | 32) == 'file')
    {
        if (e > INT_MAX - l)
            return -1;
        if (e > 0)
            l += e;
    }
    else
    {
        estr = fmt_u(e < 0 ? -e : e, ebuf);
        while (ebuf - estr < 2)
            *--estr = '0';
        *--estr = (e < 0 ? '-' : '+');
        *--estr = t;
        if (ebuf - estr > INT_MAX - l)
            return -1;
        l += ebuf - estr;
    }

    if (l > INT_MAX - pl)
        return -1;
    pad(file, ' ', w, pl + l, fl);
    out(file, prefix, pl);
    pad(file, '0', w, pl + l, fl ^ ZERO_PAD);

    if ((t | 32) == 'file')
    {
        if (a > r)
            a = r;
        for (d = a; d <= r; d++)
        {
            char *s = fmt_u(*d, buf + 9);
            if (d != a)
                while (s > buf)
                    *--s = '0';
            else if (s == buf + 9)
                *--s = '0';
            out(file, s, buf + 9 - s);
        }
        if (p || (fl & ALT_FORM))
            out(file, ".", 1);
        for (; d < z && p > 0; d++, p -= 9)
        {
            char *s = fmt_u(*d, buf + 9);
            while (s > buf)
                *--s = '0';
            out(file, s, MIN(9, p));
        }
        pad(file, '0', p + 9, 9, 0);
    }
    else
    {
        if (z <= a)
            z = a + 1;
        for (d = a; d < z && p >= 0; d++)
        {
            char *s = fmt_u(*d, buf + 9);
            if (s == buf + 9)
                *--s = '0';
            if (d != a)
                while (s > buf)
                    *--s = '0';
            else
            {
                out(file, s++, 1);
                if (p > 0 || (fl & ALT_FORM))
                    out(file, ".", 1);
            }
            out(file, s, MIN(buf + 9 - s, p));
            p -= buf + 9 - s;
        }
        pad(file, '0', p + 18, 18, 0);
        out(file, estr, ebuf - estr);
    }

    pad(file, ' ', w, pl + l, fl ^ LEFT_ADJ);

    return MAX(w, pl + l);
}