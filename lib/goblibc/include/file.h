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
