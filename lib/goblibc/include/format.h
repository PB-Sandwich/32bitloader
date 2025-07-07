/**
 * @file format.h
 * @author Sofia "MetalPizzaCat"
 * @brief Set of helper functions used to help with formatting
 * @version 0.1
 * @date 2025-07-07
 *
 * @copyright Copyright (c) 2025
 *
 */
#pragma once
#include <file.h>
#include <stdarg.h>

static int gob_format_float(FILE *file, long double value, int width, int p, int fl, int t, int precision);

/// @brief Helper function that writes an int value as unsigned into the provided string. 
/// This writes to string in reverse, so requires pointer to be far enough to fit the string
/// @param value Value to write
/// @param str Destination
/// @return Pointer to the string
static char *gob_format_unsigned(uintmax_t value, char *str);

/// @brief Helper function that writes an int value as an octal into the provided string. 
/// This writes to string in reverse, so requires pointer to be far enough to fit the string
/// @param x Value to write
/// @param s Destination
/// @return Pointer to the string
static char *gob_format_octal(uintmax_t x, char *s);

/// @brief Helper function that writes an int value as an hex into the provided string. 
/// This writes to string in reverse, so requires pointer to be far enough to fit the string
/// @param x Value to write
/// @param s Destination
/// @return Pointer to the string
/// @param lower Whether to write as lowercase or uppercase. 32 for lowercase, 0 for uppercase. This uses `ascii | lowercase` operation
/// @return 
static char *gob_format_hex(uintmax_t x, char *s, int lower);