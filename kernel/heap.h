
#pragma once

#include <stdint.h>

#define NULL (void*)0

typedef struct BlockHeader {
    uint32_t size;
    uint8_t free;
    struct BlockHeader* prev;
    struct BlockHeader* next;
} BlockHeader;

void init_heap(uint8_t* heap_buffer, uint32_t heap_size);
void dump_heap();

void* malloc(uint32_t size);
void* realloc(void* ptr, uint32_t size);
void free(void* ptr);
