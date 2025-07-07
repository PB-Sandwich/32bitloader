/**
 * @file file.h
 * @author Sofia "MetalPizzaCat"
 * @brief An implementation of FILE related operations for use with IO and string operations.
 *  A FILE can be used as a wrapper for a null terminated string.
 * Also contains a set of "scan help" functions which are meant for scan* functions
 * @version 0.1
 * @date 2025-07-06
 *
 * @copyright Copyright (c) 2025
 *
 */
#pragma once
#include <stdint.h>
#include <bits/alltypes.h>

/// @brief Temporary implementation of FILE-like struct, only for use with strings. Contains some iterator like data
struct _IO_FILE
{
    unsigned flags;
    unsigned char *rpos;
    unsigned char *rend;
    unsigned char *wend;
    unsigned char *wpos;
    size_t (*read)(struct IO_FILE_S *, unsigned char *, size_t);
    size_t (*write)(struct IO_FILE_S *, const unsigned char *, size_t);
    off_t (*seek)(struct IO_FILE_S *, off_t, int);
    unsigned char *buf;
    int lbf;
    unsigned char *shend;
    off_t shlim;
    off_t shcnt;
};

typedef struct _IO_FILE FILE;


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
uint32_t __fwritex(const unsigned char *restrict str, uint32_t len, FILE *restrict file);