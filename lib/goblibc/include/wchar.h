#pragma once
#include <bits/alltypes.h>
#ifndef wchar_t
#define wchar_t int
#endif

#define CURRENT_UTF8 0
#define MB_CUR_MAX (CURRENT_UTF8 ? 4 : 1)

int wctomb(char *s, wchar_t wc);

size_t wcrtomb(char *restrict s, wchar_t wc);