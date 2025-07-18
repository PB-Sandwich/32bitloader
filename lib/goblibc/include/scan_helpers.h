/**
 * @file scan_helpers.h
 * @author Sofia "MetalPizzaCat"
 * @brief Set of helper functions used during scan operations
 * @version 0.1
 * @date 2025-07-07
 *
 * @copyright Copyright (c) 2025
 *
 */
#pragma once
#include <file.h>
#include <stdint.h>
#include <bits/alltypes.h>

/// @brief Scan an integer value
/// @param file Source from which to read the value
/// @param base Base of the value
/// @param pok
/// @param lim
/// @return
unsigned long long gob_intscan(FILE *file, unsigned base, int pok, unsigned long long lim);

/// @brief Scan a floating point value
/// @param file Source from which to read the value
/// @param prec Precision
/// @param pok
/// @return
long double gob_floatscan(FILE *file, int prec, int pok);

/// @brief Create a pseudo file from a null terminated string
/// @param file A non-null file pointer to be initialized
/// @param str Null terminated string
void scan_help_file_from_string(FILE *file, const char *str);

void scan_help_lim(FILE *file, off_t lim);

char __scan_help_getc(FILE *file);

/// @brief Get the next character in sequence
/// @param file Pointer to the file
/// @return character or EOF if file ended
char scan_help_getc(FILE *file);

/// @brief Move iterator back by one unit
/// @param file
void scan_help_unget(FILE *file);

/// @brief
/// @param file
/// @return
int scan_help_cnt(FILE *file);

int __uflow(FILE *it_str);

/// @brief Write a string into a file
/// @param str String to write
/// @param len Length of the string
/// @param file Destination file
/// @return
uint32_t scan_help_write_str(const unsigned char *restrict str, uint32_t len, FILE *restrict file);

size_t scan_help_str_read(FILE *, unsigned char *, size_t);

size_t scan_help_str_write(FILE *, const unsigned char *, size_t);