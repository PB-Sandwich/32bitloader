#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <file.h>
#include <ctype.h>
#include <errno.h>
#include <scan_helpers.h>

double atof(const char *s)
{
    strtod(s, 0);
}

static long double strtox(const char *s, char **p, int prec)
{
    FILE file;
    scan_help_file_from_string(&file, s);
    scan_help_lim(&file, 0);
    long double y = gob_floatscan(&file, prec, 1);
    off_t count = scan_help_cnt(&file);
    if (p != NULL)
    {
        *p = count > 0 ? (char *)s + count : (char *)s;
    }
    return y;
}



float strtof(const char *s, char **p)
{
    return strtox(s, p, 0);
}

double strtod(const char *s, char **p)
{
    return strtox(s, p, 1);
}

long double strtold(const char *s, char **p)
{
    return strtox(s, p, 2);
}
