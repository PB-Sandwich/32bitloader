#pragma once
#include <features.h>
#include <file.h>
#include <stdarg.h>
/* The value returned by fgetc and similar functions to indicate the
   end of the file.  */
#define EOF (-1)

#define feof(f) ((f)->flags & F_EOF)
#define ferror(f) ((f)->flags & F_ERR)

int format(FILE *__restrict, const char *__restrict, ...);

/// @brief Core of the printf logic that actually formats the input
/// @param f File(or pseudo file) pointer to the destination
/// @param fmt Formatting string
/// @param ap List of arguments
/// @param nl_arg
/// @param nl_type
/// @return
static int printf_core(FILE *f, const char *fmt, va_list *ap, union arg *nl_arg, int *nl_type);

static int fmt_fp(FILE *file, long double value, int w, int p, int fl, int t, int precision);

static void pad(FILE *file, char c, int w, int l, int fl);