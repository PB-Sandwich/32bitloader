#pragma once
#include <file.h>

int gob_format(char *str, const char *fmt, ...);

static int gob_format_float(char *str, long double value, int width, int p, int fl, int t, int precision);