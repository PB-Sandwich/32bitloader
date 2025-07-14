#pragma once
#include <bits/alltypes.h>
#ifndef wchar_t
#define wchar_t int
#endif

#define CURRENT_UTF8 0
#define MB_CUR_MAX (CURRENT_UTF8 ? 4 : 1)

int wctomb(char *s, wchar_t wc);

/// @brief convert a wide character to a multibyte sequence
/// @param s Sequence
/// @param wc wide character
/// @return number of bytes written to the sequence
size_t wcrtomb(char *restrict s, wchar_t wc);

size_t mbrtowc(wchar_t *restrict wc, const char *restrict src, size_t n, mbstate_t *restrict st);

int mbsinit(const mbstate_t *st);