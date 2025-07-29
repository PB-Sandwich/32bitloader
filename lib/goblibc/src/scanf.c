#include "stdio.h"
#include <stdlib.h>
#include <ctype.h>
#include <scan_helpers.h>
#include <limits.h>
#include <string.h>
#include <wchar.h>

#define SIZE_hh -2
#define SIZE_h -1
#define SIZE_def 0
#define SIZE_l 1
#define SIZE_L 2
#define SIZE_ll 3

static void store_int(void *dest, int size, unsigned long long i)
{
    if (!dest)
        return;
    switch (size)
    {
    case SIZE_hh:
        *(char *)dest = i;
        break;
    case SIZE_h:
        *(short *)dest = i;
        break;
    case SIZE_def:
        *(int *)dest = i;
        break;
    case SIZE_l:
        *(long *)dest = i;
        break;
    case SIZE_ll:
        *(long long *)dest = i;
        break;
    }
}

/// @brief Get argument at given position in the argument list
/// @param arg_list list of arguments
/// @param n id
/// @return
static void *get_arg_at(va_list arg_list, unsigned int i)
{
    va_list ap2;
    va_copy(ap2, arg_list);
    for (unsigned int j = i; j > 1; j--)
    {
        va_arg(ap2, void *);
    }
    void *p = va_arg(ap2, void *);
    va_end(ap2);
    return p;
}


int gob_vfscanf(FILE *file, const char *fmt, va_list args)
{
    int width;
    int size;
    int used_allocation = 0;
    int base;
    const unsigned char *fmt_ptr;
    int c, t;
    char *s;
    wchar_t *wide_chars;
    mbstate_t current_state;
    void *dest = NULL;
    int invert;
    int matches = 0;
    unsigned long long x;
    long double y;
    off_t pos = 0;
    unsigned char scanset[257];
    size_t i, k;
    wchar_t current_wide_char;

    // FLOCK(f);

    // if (!file->rpos)
    // {
    //     __toread(file);
    // }
    if (file->rpos == NULL)
    {
        // UNLOCK FILE
        return 0;
    }

    for (fmt_ptr = (const unsigned char *)fmt; *fmt_ptr; fmt_ptr++)
    {

        used_allocation = 0;

        if (isspace(*fmt_ptr))
        {
            for (; isspace(fmt_ptr[1]); fmt_ptr++)
                ;

            scan_help_lim(file, 0);
            while (isspace(scan_help_getc(file)))
                ;
            scan_help_unget(file);
            pos += scan_help_cnt(file);
            continue;
        }
        if (*fmt_ptr != '%' || fmt_ptr[1] == '%')
        {
            scan_help_lim(file, 0);
            if (*fmt_ptr == '%')
            {
                fmt_ptr++;
                while (isspace((c = scan_help_getc(file))))
                    ;
            }
            else
            {
                c = scan_help_getc(file);
            }
            if (c != *fmt_ptr)
            {
                scan_help_unget(file);
                if (c < 0)
                    goto input_fail;
                goto match_fail;
            }
            pos += scan_help_cnt(file);
            continue;
        }

        fmt_ptr++;
        if (*fmt_ptr == '*')
        {
            dest = 0;
            fmt_ptr++;
        }
        else if (isdigit(*fmt_ptr) && fmt_ptr[1] == '$')
        {
            dest = get_arg_at(args, *fmt_ptr - '0');
            fmt_ptr += 2;
        }
        else
        {
            dest = va_arg(args, void *);
        }

        for (width = 0; isdigit(*fmt_ptr); fmt_ptr++)
        {
            width = 10 * width + *fmt_ptr - '0';
        }

        if (*fmt_ptr == 'm')
        {
            wide_chars = 0;
            s = 0;
            used_allocation = !!dest;
            fmt_ptr++;
        }
        else
        {
            used_allocation = 0;
        }

        size = SIZE_def;
        switch (*fmt_ptr++)
        {
        case 'h':
            if (*fmt_ptr == 'h')
                fmt_ptr++, size = SIZE_hh;
            else
                size = SIZE_h;
            break;
        case 'l':
            if (*fmt_ptr == 'l')
                fmt_ptr++, size = SIZE_ll;
            else
                size = SIZE_l;
            break;
        case 'j':
            size = SIZE_ll;
            break;
        case 'z':
        case 't':
            size = SIZE_l;
            break;
        case 'L':
            size = SIZE_L;
            break;
        case 'd':
        case 'i':
        case 'o':
        case 'u':
        case 'x':
        case 'a':
        case 'e':
        case 'f':
        case 'g':
        case 'A':
        case 'E':
        case 'F':
        case 'G':
        case 'X':
        case 's':
        case 'c':
        case '[':
        case 'S':
        case 'C':
        case 'p':
        case 'n':
            fmt_ptr--;
            break;
        default:
            goto fmt_fail;
        }

        t = *fmt_ptr;

        /* C or S */
        if ((t & 0x2f) == 3)
        {
            t |= 32;
            size = SIZE_l;
        }

        switch (t)
        {
        case 'c':
            if (width < 1)
                width = 1;
        case '[':
            break;
        case 'n':
            store_int(dest, size, pos);
            /* do not increment match count, etc! */
            continue;
        default:
            scan_help_lim(file, 0);
            while (isspace(scan_help_getc(file)))
                ;
            scan_help_unget(file);
            pos += scan_help_cnt(file);
        }

        scan_help_lim(file, width);
        if (scan_help_getc(file) < 0)
            goto input_fail;
        scan_help_unget(file);

        switch (t)
        {
        case 's':
        case 'c':
        case '[':
            if (t == 'c' || t == 's')
            {
                memset(scanset, -1, sizeof scanset);
                scanset[0] = 0;
                if (t == 's')
                {
                    scanset[1 + '\t'] = 0;
                    scanset[1 + '\n'] = 0;
                    scanset[1 + '\v'] = 0;
                    scanset[1 + '\f'] = 0;
                    scanset[1 + '\r'] = 0;
                    scanset[1 + ' '] = 0;
                }
            }
            else
            {
                if (*++fmt_ptr == '^')
                    fmt_ptr++, invert = 1;
                else
                    invert = 0;
                memset(scanset, invert, sizeof scanset);
                scanset[0] = 0;
                if (*fmt_ptr == '-')
                    fmt_ptr++, scanset[1 + '-'] = 1 - invert;
                else if (*fmt_ptr == ']')
                    fmt_ptr++, scanset[1 + ']'] = 1 - invert;
                for (; *fmt_ptr != ']'; fmt_ptr++)
                {
                    if (!*fmt_ptr)
                        goto fmt_fail;
                    if (*fmt_ptr == '-' && fmt_ptr[1] && fmt_ptr[1] != ']')
                        for (c = fmt_ptr++ [-1]; c < *fmt_ptr; c++)
                            scanset[1 + c] = 1 - invert;
                    scanset[1 + *fmt_ptr] = 1 - invert;
                }
            }
            wide_chars = 0;
            s = 0;
            i = 0;
            k = t == 'c' ? width + 1U : 31;
            if (size == SIZE_l)
            {
                if (used_allocation)
                {
                    wide_chars = malloc(k * sizeof(wchar_t));
                    if (!wide_chars)
                        goto alloc_fail;
                }
                else
                {
                    wide_chars = dest;
                }
                current_state = (mbstate_t){0};
                while (scanset[(c = scan_help_getc(file)) + 1])
                {
                    switch (mbrtowc(&current_wide_char, &(char){c}, 1, &current_state))
                    {
                    case -1:
                        goto input_fail;
                    case -2:
                        continue;
                    }
                    if (wide_chars)
                        wide_chars[i++] = current_wide_char;
                    if (used_allocation && i == k)
                    {
                        k += k + 1;
                        wchar_t *tmp = realloc(wide_chars, k * sizeof(wchar_t));
                        if (!tmp)
                            goto alloc_fail;
                        wide_chars = tmp;
                    }
                }
                if (!mbsinit(&current_state))
                    goto input_fail;
            }
            else if (used_allocation)
            {
                s = malloc(k);
                if (!s)
                    goto alloc_fail;
                while (scanset[(c = scan_help_getc(file)) + 1])
                {
                    s[i++] = c;
                    if (i == k)
                    {
                        k += k + 1;
                        char *tmp = realloc(s, k);
                        if (!tmp)
                            goto alloc_fail;
                        s = tmp;
                    }
                }
            }
            else if ((s = dest))
            {
                while (scanset[(c = scan_help_getc(file)) + 1])
                    s[i++] = c;
            }
            else
            {
                while (scanset[(c = scan_help_getc(file)) + 1])
                    ;
            }
            scan_help_unget(file);
            if (!scan_help_cnt(file))
                goto match_fail;
            if (t == 'c' && scan_help_cnt(file) != width)
                goto match_fail;
            if (used_allocation)
            {
                if (size == SIZE_l)
                    *(wchar_t **)dest = wide_chars;
                else
                    *(char **)dest = s;
            }
            if (t != 'c')
            {
                if (wide_chars)
                    wide_chars[i] = 0;
                if (s)
                    s[i] = 0;
            }
            break;
        case 'p':
        case 'X':
        case 'x':
            base = 16;
            goto int_common;
        case 'o':
            base = 8;
            goto int_common;
        case 'd':
        case 'u':
            base = 10;
            goto int_common;
        case 'i':
            base = 0;
        int_common:
            x = gob_intscan(file, base, 0, ULLONG_MAX);
            if (!scan_help_cnt(file))
                goto match_fail;
            if (t == 'p' && dest)
                *(void **)dest = (void *)(uintptr_t)x;
            else
                store_int(dest, size, x);
            break;
        case 'a':
        case 'A':
        case 'e':
        case 'E':
        case 'f':
        case 'F':
        case 'g':
        case 'G':
            y = gob_floatscan(file, size, 0);
            if (!scan_help_cnt(file))
                goto match_fail;
            if (dest)
                switch (size)
                {
                case SIZE_def:
                    *(float *)dest = y;
                    break;
                case SIZE_l:
                    *(double *)dest = y;
                    break;
                case SIZE_L:
                    *(long double *)dest = y;
                    break;
                }
            break;
        }

        pos += scan_help_cnt(file);
        if (dest)
            matches++;
    }
    if (0)
    {
    fmt_fail:
    alloc_fail:
    input_fail:
        if (!matches)
            matches--;
    match_fail:
        if (used_allocation)
        {
            // i really dislike this but without using c++ and smart pointers this is kinda the easiest way to clean stuff up
            free(s);
            free(wide_chars);
        }
    }
    // FUNLOCK(f);
    return matches;
}
int gob_sscanf(const char *str, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    FILE file;
    scan_help_file_from_string(&file, str);
    int result = gob_vfscanf(&file, fmt, args);
    va_end(args);

    return result;
}