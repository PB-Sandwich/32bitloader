/**
 * @file stdlib.h
 * @author Sofia "MetalPizzaCat"
 * @brief Basic reimplementation of stdlib.h from libc, with additional functions like atoi/itoa
 * @version 0.1
 * @date 2025-06-30
 *
 * @copyright Copyright (c) 2025
 *
 */
#pragma once

#include <stdint.h>

#define NULL (void *)0

typedef struct BlockHeader
{
    uint32_t size;
    uint8_t free;
    struct BlockHeader *prev;
    struct BlockHeader *next;
} BlockHeader;

void init_heap(uint8_t *heap_buffer, uint32_t heap_size);

void *malloc(uint32_t size);
void *realloc(void *ptr, uint32_t size);
void free(void *ptr);

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

int atoi(char *buffer);

/// @brief Convert integer value to a string. Implementation of a somewhat non standard function
/// @param value Value to convert
/// @param buffer Buffer in which to store the value. Has to be big enough to fit the result + null terminator
/// @param radix Base of the number
/// @return Pointer to the buffer
char *itoa(int value, char *buffer, int radix);

/// @brief Convert integer value to a string.
/// @param value Value to convert
/// @param buffer Buffer in which to store the value. Has to be big enough to fit the result + null terminator
/// @param radix Base of the number
/// @param len How long was the number that was written in the buffer
/// @return
char *int_to_str(int32_t value, char *buffer, int32_t radix, uint32_t *len);

/// @brief Convert a string input into a floating point value. This is a nonstandard function so might behave out of spec
/// @param
/// @return
double atof(const char *);

/// @brief Convert a string into a floating point
/// @param start Start of the string
/// @param end End of the string
/// @return Parsed value
float strtof(const char *start, char **end);

/// @brief Convert a string into a floating point double precision value
/// @param start Start of the string
/// @param end End of the string
/// @return Parsed value
double strtod(const char *start, char **end);

/// @brief Convert a string into a floating point long double precision value
/// @param start Start of the string
/// @param end End of the string
/// @return Parsed value
long double strtold(const char *start, char **end);

/// @brief Convert a string into a floating point
/// @param s Start of the string
/// @param p End of the string
/// @param prec Precision
/// @return Parsed value
static long double strtox(const char *s, char **p, int prec);

long long stroll(const char *start, char **end, int base);