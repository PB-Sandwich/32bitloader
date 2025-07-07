#pragma once
#include <file.h>
#include <stdarg.h>

int gob_vfprintf(FILE *file, const char *fmt, va_list args);

/// @brief Format a string using sprintf formatting system
/// @param str Target string
/// @param fmt Formating options
/// @param
/// @return -1 on failure
int gob_sprintf(char *str, const char *fmt, ...);

static int gob_format_float(FILE *file, long double value, int width, int p, int fl, int t, int precision);