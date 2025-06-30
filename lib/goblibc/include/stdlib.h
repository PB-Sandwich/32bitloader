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

char *itoa(int value, char *buffer, int radix);