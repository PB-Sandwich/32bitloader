#include <format.h>
#include <scan_helpers.h>
#include <stdarg.h>
#include <float.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <wchar.h>
#include <stdbool.h>
/* Some useful macros */

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/* Convenient bit representation for modifier flags, which all fall
 * within 31 codepoints of the space character. */

#define ALT_FORM (1U << ('#' - ' '))
#define ZERO_PAD (1U << ('0' - ' '))
#define LEFT_ADJ (1U << ('-' - ' '))
#define PAD_POS (1U << (' ' - ' '))
#define MARK_POS (1U << ('+' - ' '))
#define GROUPED (1U << ('\'' - ' '))

#define FLAGMASK (ALT_FORM | ZERO_PAD | LEFT_ADJ | PAD_POS | MARK_POS | GROUPED)

union arg
{
    uintmax_t i;
    long double f;
    void *p;
};

/// @brief State machine to accept length modifiers + conversion specifiers. Result is 0 on failure, or an argument type to pop on success.
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

/// @brief Collection of all the states that are used by the formating symbols.
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

static const char xdigits[16] = {
    "0123456789ABCDEF"};

#define OOB(x) ((unsigned)(x) - 'A' > 'z' - 'A')

static int getint(char **s)
{
    int i;
    for (i = 0; isdigit(**s); (*s)++)
    {
        if (i > INT_MAX / 10U || **s - '0' > INT_MAX - 10 * i)
            i = -1;
        else
            i = 10 * i + (**s - '0');
    }
    return i;
}

static void out(FILE *file, const char *src_str, uint32_t len)
{
    scan_help_write_str(src_str, len, file);
}
static void pad(FILE *file, char c, int width, int l, int flags)
{
    char pad[256];
    if (flags & (LEFT_ADJ | ZERO_PAD) || l >= width)
    {
        return;
    }
    memset(pad, c, l > sizeof(pad) ? sizeof(pad) : l);
    for (; l >= sizeof(pad); l -= sizeof(pad))
    {
        out(file, pad, sizeof(pad));
    }
    out(file, pad, l);
}

static char *gob_format_hex(uintmax_t x, char *s, int lower)
{
    for (; x; x >>= 4)
        *--s = xdigits[(x & 15)] | lower;
    return s;
}

static char *gob_format_octal(uintmax_t x, char *s)
{
    for (; x; x >>= 3)
        *--s = '0' + (x & 7);
    return s;
}

static char *gob_format_unsigned(uintmax_t value, char *str)
{
    unsigned long y;
    for (; value > ULONG_MAX; value /= 10)
        *--str = '0' + value % 10;
    for (y = value; y >= 10; y /= 10)
        *--str = '0' + y % 10;
    if (y)
        *--str = '0' + y;
    return str;
}

// Used in place of the goto to finish up the number formatting blocks
#define IFMT_TAIL(xp, p, mod_flag, arg, a, z) \
    if (xp)                                   \
    {                                         \
        mod_flag &= ~ZERO_PAD;                \
    }                                         \
    \               
    if (!arg.i && !p)                         \
    {                                         \
        a = z;                                \
        break;                                \
    }                                         \
    p = MAX(p, z - a + !arg.i);


int gob_format_core(FILE *file, const char *fmt, va_list *args, union arg *nl_arg, int *nl_type)
{
    char *fmt_start;
    char *fmt_end;
    char *fmt_ptr = (char *)fmt;

    uint32_t using_position = 0;
    uint32_t mod_flag;
    int32_t format_width;
    int32_t precision;
    int32_t use_precision;
    union arg arg;
    int32_t arg_pos;
    uint32_t format_state;
    uint32_t prev_format_state;
    int32_t output_count = 0;
    int32_t len = 0;
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
        if (*fmt_ptr == '\0')
        {
            break;
        }

        // find format specifier start
        for (fmt_start = fmt_ptr; *fmt_ptr != '\0' && *fmt_ptr != '%'; fmt_ptr++)
            ;
        // check for "%%", which should just print '%' and skip past '%'
        // we do this by checking if current pair of symbols is '%%' and if it is, skip past it
        // only advance end by one so that we would skip the '%' sign
        for (fmt_end = fmt_ptr; fmt_ptr[0] == '%' && fmt_ptr[1] == '%'; fmt_end++, fmt_ptr += 2)
            ;
        if (fmt_end - fmt_start > INT_MAX - output_count)
        {
            errno = EOVERFLOW;
            return -1;
        }
        // if this len is 0 then we have nothing to output so we can go and deal with actual formatting symbols
        len = fmt_end - fmt_start;
        if (file != NULL)
        {
            out(file, fmt_start, len);
        }
        if (len > 0)
        {
            continue;
        }

        // %1$d is used for positional arguments
        // did you know printf could do that? cause i didn't
        if (isdigit(fmt_ptr[1]) && fmt_ptr[2] == '$')
        {
            using_position = 1;
            arg_pos = fmt_ptr[1] - '0';
            // skip by 3 to move past positional data
            fmt_ptr += 3;
        }
        else
        {
            arg_pos = -1;
            fmt_ptr++;
        }

        // read modifier flag symbols
        // first we check if it's a symbol from ' ' to '@'
        // then we check if it fits the mask checks
        // and if so, use that mask value to set the flag
        // a bit awkward if you ask me
        for (mod_flag = 0; (unsigned)*fmt_ptr - ' ' < 32 && (FLAGMASK & (1U << (*fmt_ptr - ' '))); fmt_ptr++)
        {
            mod_flag |= 1U << (*fmt_ptr - ' ');
        }

        // read how much space in the string to reserve for the next input
        if (*fmt_ptr == '*')
        {
            // check if it's another positional approach for specifying the value
            if (isdigit(fmt_ptr[1]) && fmt_ptr[2] == '$')
            {
                // to avoid duplicating the entire function
                // this function either fills out the arg array or uses arg array
                // depending on whether it has destination parameter
                using_position = 1;
                if (file == NULL)
                {
                    // store value for positional argument in the array at the right position
                    nl_type[fmt_ptr[1] - '0'] = INT;
                    format_width = 0;
                }
                else
                {
                    // otherwise read that value
                    format_width = nl_arg[fmt_ptr[1] - '0'].i;
                }
                fmt_ptr += 3;
            }
            else if (!using_position)
            {
                // read next value out of argument array, or if we are no actually
                // print, then do nothing
                format_width = file ? va_arg(*args, int) : 0;
                fmt_ptr++;
            }
            else
            {
                errno = EINVAL;
                return -1;
            }
            if (format_width < 0)
            {
                // negative values mean left justified text
                // so we mark it as such, and flip the sign for better operation
                mod_flag |= LEFT_ADJ, format_width = -format_width;
            }
        }
        // c++ has better syntax for this but this is not c++
        // check if current integer value(for '%69s' syntax as opposed to '%*s' syntax)
        // is actually a valid integer number
        else if ((format_width = getint(&fmt_ptr)) < 0)
        {
            errno = EOVERFLOW;
            return -1;
        }

        // check the floating value precision formatting
        if (*fmt_ptr == '.' && fmt_ptr[1] == '*')
        {
            // once more, check if the values are a passed positional argument
            if (isdigit(fmt_ptr[2]) && fmt_ptr[3] == '$')
            {
                if (file == NULL)
                {
                    nl_type[fmt_ptr[2] - '0'] = INT;
                    precision = 0;
                }
                else
                {
                    precision = nl_arg[fmt_ptr[2] - '0'].i;
                }
                fmt_ptr += 4;
            }
            else if (!using_position)
            {
                precision = file ? va_arg(*args, int) : 0;
                fmt_ptr += 2;
            }
            else
            {
                errno = EINVAL;
                return -1;
            }
            use_precision = (precision >= 0);
        }
        // otherwise we just read the precision from the string itself
        else if (*fmt_ptr == '.')
        {
            fmt_ptr++;
            precision = getint(&fmt_ptr);
            use_precision = 1;
        }
        else
        {
            // if none specified, don't use precision
            precision = -1;
            use_precision = 0;
        }

        // this is a fairly odd approach but it works by having a tree of options
        // and in the loop it picks paths in the tree
        format_state = 0;
        do
        {
            // check if it's a valid character
            if (OOB(*fmt_ptr))
            {
                errno = EINVAL;
                return -1;
            }
            prev_format_state = format_state;
            format_state = states[format_state][(*fmt_ptr++) - 'A'];
        } while (format_state - 1 < STOP);
        if (!format_state)
        {
            errno = EINVAL;
            return -1;
        }

        /* Check validity of argument type (nl/normal) */
        if (format_state == NOARG)
        {
            if (arg_pos >= 0)
            {
                errno = EINVAL;
                return -1;
            }
        }
        else
        {
            if (arg_pos >= 0)
            {
                if (file == NULL)
                {
                    nl_type[arg_pos] = format_state;
                }
                else
                {
                    arg = nl_arg[arg_pos];
                }
            }
            else if (file != NULL)
            {
                pop_arg(&arg, format_state, args);
            }
            else
            {
                return 0;
            }
        }

        // rest of the code is only viable if we are are actually printing values
        if (file == NULL)
            continue;

        /* Do not process any new directives once in error state. */
        // the original implementation relied  on ferror, because it used file instead of string, idk what to do here
        // TODO: add proper file state check
        // if (ferror(str))
        // if (0)
        // {
        //     return -1;
        // }

        fmt_end = buf + sizeof(buf);
        prefix = "-+   0X0x";
        pl = 0;
        t = fmt_ptr[-1];

        /* Transform ls,lc -> S,C */
        if (prev_format_state && (t & 15) == 3)
            t &= ~32;

        // because left adjustment and zero padding are mutually exclusive, disable zero padding if both are used
        if (mod_flag & LEFT_ADJ)
        {
            mod_flag &= ~ZERO_PAD;
        }

        switch (t)
        {
            // n is a modifier that will print out current string position
            // as such we store the current output_count converted into required type
        case 'n':
            switch (prev_format_state)
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
            // pointer address
        case 'p':
            precision = MAX(precision, 2 * sizeof(void *));
            t = 'x';
            mod_flag |= ALT_FORM;
            // fall down because pointer address uses HEX formatting
        case 'x':
        case 'X':
            // format the value as hex
            fmt_start = gob_format_hex(arg.i, fmt_end, t & 32);
            if (arg.i && (mod_flag & ALT_FORM))
            {
                prefix += (t >> 4), pl = 2;
            }
            goto ifmt_tail;
        case 'o':
            fmt_start = gob_format_octal(arg.i, fmt_end);
            if ((mod_flag & ALT_FORM) && precision < fmt_end - fmt_start + 1)
                precision = fmt_end - fmt_start + 1;
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
            fmt_start = gob_format_unsigned(arg.i, fmt_end);
        ifmt_tail:
            if (use_precision && precision < 0)
            {
                errno = EOVERFLOW;
                return -1;
            }
            if (use_precision)
                mod_flag &= ~ZERO_PAD;
            if (!arg.i && !precision)
            {
                fmt_start = fmt_end;
                break;
            }
            precision = MAX(precision, fmt_end - fmt_start + !arg.i);
            break;
        narrow_c:
        case 'c':
            *(fmt_start = fmt_end - (precision = 1)) = arg.i;
            mod_flag &= ~ZERO_PAD;
            break;
        case 'm':
            if (1)
                fmt_start = strerror(errno);
            else
            case 's':
                fmt_start = arg.p ? arg.p : "(null)";
            fmt_end = fmt_start + strnlen(fmt_start, precision < 0 ? INT_MAX : precision);
            if (precision < 0 && *fmt_end)
            {
                errno = EOVERFLOW;
                return -1;
            }
            precision = fmt_end - fmt_start;
            mod_flag &= ~ZERO_PAD;
            break;
        case 'C':
            if (!arg.i)
                goto narrow_c;
            wc[0] = arg.i;
            wc[1] = 0;
            arg.p = wc;
            precision = -1;
        case 'S':
            ws = arg.p;
            for (i = len = 0; i < precision && *ws && (len = wctomb(mb, *ws++)) >= 0 && len <= precision - i; i += len)
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
            precision = i;
            pad(file, ' ', format_width, precision, mod_flag);
            ws = arg.p;
            for (i = 0; i < 0U + precision && *ws && i + (len = wctomb(mb, *ws++)) <= precision; i += len)
            {
                out(file, mb, len);
            }
            pad(file, ' ', format_width, precision, mod_flag ^ LEFT_ADJ);
            len = format_width > precision ? format_width : precision;
            continue;
        case 'e':
        case 'f':
        case 'g':
        case 'a':
        case 'E':
        case 'F':
        case 'G':
        case 'A':
            if (use_precision && precision < 0)
            {
                errno = EOVERFLOW;
                return -1;
            }
            len = gob_format_float(file, arg.f, format_width, precision, mod_flag, t, prev_format_state);
            if (len < 0)
            {
                errno = EOVERFLOW;
                return -1;
            }
            continue;
        }

        if (precision < fmt_end - fmt_start)
            precision = fmt_end - fmt_start;
        if (precision > INT_MAX - pl)
        {
            errno = EOVERFLOW;
            return -1;
        }
        if (format_width < pl + precision)
            format_width = pl + precision;
        if (format_width > INT_MAX - output_count)
        {
            errno = EOVERFLOW;
            return -1;
        }

        pad(file, ' ', format_width, pl + precision, mod_flag);
        out(file, prefix, pl);
        pad(file, '0', format_width, pl + precision, mod_flag ^ ZERO_PAD);
        pad(file, '0', precision, fmt_end - fmt_start, 0);
        out(file, fmt_start, fmt_end - fmt_start);
        pad(file, ' ', format_width, pl + precision, mod_flag ^ LEFT_ADJ);

        len = format_width;
    }

    if (file != NULL)
    {
        return output_count;
    }
    if (!using_position)
    {
        return 0;
    }

    for (i = 1; i <= NL_ARGMAX && nl_type[i]; i++)
    {
        pop_arg(&nl_arg[i], nl_type[i], &args);
    }
    for (; i <= NL_ARGMAX && !nl_type[i]; i++)
        ;
    if (i <= NL_ARGMAX)
    {
        errno = EINVAL;
        return -1;
    }
    return 1;
}

static int gob_format_float(FILE *file, long double value, int width, int p, int fl, int t, int precision)
{
    int bufsize = (precision == BIGLPRE)
                      ? (LDBL_MANT_DIG + 28) / 29 + 1 +                 // mantissa expansion
                            (LDBL_MAX_EXP + LDBL_MANT_DIG + 28 + 8) / 9 // exponent expansion
                      : (DBL_MANT_DIG + 28) / 29 + 1 +
                            (DBL_MAX_EXP + DBL_MANT_DIG + 28 + 8) / 9;
    uint32_t big[bufsize];
    uint32_t *a, *d, *r, *z;
    int e2 = 0, e, i, j, len;
    char buf[9 + LDBL_MANT_DIG / 4];
    char *str_ptr;
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
    {
        prefix++;
        pl = 0;
    }

    if (!isfinite(value))
    {
        char *str = (t & 32) ? "inf" : "INF";
        if (value != value)
            str = (t & 32) ? "nan" : "NAN";
        pad(str, ' ', width, 3 + pl, fl & ~ZERO_PAD);
        out(str, prefix, pl);
        out(str, str, 3);
        pad(str, ' ', width, 3 + pl, fl ^ LEFT_ADJ);
        return MAX(width, 3 + pl);
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

        estr = gob_format_unsigned(e2 < 0 ? -e2 : e2, ebuf);
        if (estr == ebuf)
        {
            *--estr = '0';
        }
        *--estr = (e2 < 0 ? '-' : '+');
        *--estr = t + ('p' - 'a');

        str_ptr = buf;
        do
        {
            int value = value;
            *str_ptr++ = xdigits[value] | (t & 32);
            value = 16 * (value - value);
            if (str_ptr - buf == 1 && (value || p > 0 || (fl & ALT_FORM)))
                *str_ptr++ = '.';
        } while (value);

        if (p > INT_MAX - 2 - (ebuf - estr) - pl)
        {
            return -1;
        }
        if (p && str_ptr - buf - 2 < p)
            len = (p + 2) + (ebuf - estr);
        else
            len = (str_ptr - buf) + (ebuf - estr);

        pad(file, ' ', width, pl + len, fl);
        out(file, prefix, pl);
        pad(file, '0', width, pl + len, fl ^ ZERO_PAD);
        out(file, buf, str_ptr - buf);
        pad(file, '0', len - (ebuf - estr) - (str_ptr - buf), 0, 0);
        out(file, estr, ebuf - estr);
        pad(file, ' ', width, pl + len, fl ^ LEFT_ADJ);
        return MAX(width, pl + len);
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
            uint64_t value = ((uint64_t)*d << sh) + carry;
            *d = value % 1000000000;
            carry = value / 1000000000;
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
        b = (t | 32) == 'f' ? r : a;
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
    j = p - ((t | 32) != 'f') * e - ((t | 32) == 'g' && p);
    if (j < 9 * (z - r - 1))
    {
        uint32_t value;
        /* We avoid C'str broken division of negative numbers */
        d = r + 1 + ((j + 9 * LDBL_MAX_EXP) / 9 - LDBL_MAX_EXP);
        j += 9 * LDBL_MAX_EXP;
        j %= 9;
        for (i = 10, j++; j < 9; i *= 10, j++)
            ;
        value = *d % i;
        /* Are there any significant digits past j? */
        if (value || d + 1 != z)
        {
            long double round = 2 / LDBL_EPSILON;
            long double small;
            if ((*d / i & 1) || (i == 1000000000 && d > a && (d[-1] & 1)))
                round += 2;
            if (value < i / 2)
                small = 0x0.8p0;
            else if (value == i / 2 && d + 1 == z)
                small = 0x1.0p0;
            else
                small = 0x1.8p0;
            if (pl && *prefix == '-')
                round *= -1, small *= -1;
            *d -= value;
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
            if ((t | 32) == 'f')
                p = MIN(p, MAX(0, 9 * (z - r - 1) - j));
            else
                p = MIN(p, MAX(0, 9 * (z - r - 1) + e - j));
        }
    }
    if (p > INT_MAX - 1 - (p || (fl & ALT_FORM)))
    {
        return -1;
    }
    len = 1 + p + (p || (fl & ALT_FORM));
    if ((t | 32) == 'f')
    {
        if (e > INT_MAX - len)
        {
            return -1;
        }
        if (e > 0)
            len += e;
    }
    else
    {
        estr = gob_format_unsigned(e < 0 ? -e : e, ebuf);
        while (ebuf - estr < 2)
            *--estr = '0';
        *--estr = (e < 0 ? '-' : '+');
        *--estr = t;
        if (ebuf - estr > INT_MAX - len)
        {
            return -1;
        }
        len += ebuf - estr;
    }

    if (len > INT_MAX - pl)
    {
        return -1;
    }
    pad(file, ' ', width, pl + len, fl);
    out(file, prefix, pl);
    pad(file, '0', width, pl + len, fl ^ ZERO_PAD);

    if ((t | 32) == 'f')
    {
        if (a > r)
            a = r;
        for (d = a; d <= r; d++)
        {
            char *str = gob_format_unsigned(*d, buf + 9);
            if (d != a)
                while (str > buf)
                    *--str = '0';
            else if (str == buf + 9)
                *--str = '0';
            out(str, str, buf + 9 - str);
        }
        if (p || (fl & ALT_FORM))
            out(file, ".", 1);
        for (; d < z && p > 0; d++, p -= 9)
        {
            char *str = gob_format_unsigned(*d, buf + 9);
            while (str > buf)
                *--str = '0';
            out(str, str, MIN(9, p));
        }
        pad(file, '0', p + 9, 9, 0);
    }
    else
    {
        if (z <= a)
            z = a + 1;
        for (d = a; d < z && p >= 0; d++)
        {
            char *str = gob_format_unsigned(*d, buf + 9);
            if (str == buf + 9)
                *--str = '0';
            if (d != a)
                while (str > buf)
                    *--str = '0';
            else
            {
                out(str, str++, 1);
                if (p > 0 || (fl & ALT_FORM))
                    out(str, ".", 1);
            }
            out(str, str, MIN(buf + 9 - str, p));
            p -= buf + 9 - str;
        }
        pad(file, '0', p + 18, 18, 0);
        out(file, estr, ebuf - estr);
    }

    pad(file, ' ', width, pl + len, fl ^ LEFT_ADJ);

    return MAX(width, pl + len);
}

int gob_vfprintf(FILE *file, const char *fmt, va_list args)
{

    va_list args_copy;

    va_copy(args_copy, args);
    int nl_type[NL_ARGMAX + 1] = {0};
    union arg nl_arg[NL_ARGMAX + 1];
    // calling the function with file = NULL will fill out the arg list
    if (gob_format_core(NULL, fmt, &args_copy, nl_arg, nl_type) < 0)
    {
        va_end(args_copy);
        return -1;
    }

    int result = gob_format_core(file, fmt, &args_copy, nl_arg, nl_type);

    va_end(args_copy);
    return result;
}

int gob_sprintf(char *str, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    FILE str_file;

    // use pseudo file because it's better than just raw string. kinda.
    scan_help_file_from_string(&str_file, str);
    int result = gob_vfprintf(&str_file, fmt, args);
    va_end(args);
    return result;
}