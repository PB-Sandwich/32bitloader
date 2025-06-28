
#pragma once

#include <stdint.h>

#ifndef NULL
#define NULL (void*)0
#endif

void *memcpy(void *dest, void *source, uint32_t size);
int memcmp(void *buf1, void *buf2, uint32_t size);
void *memset(void *buf, int n, uint32_t size);

char *strcpy(char *str_dest, char *str_source);
char *strncpy(char *str_dest, char *str_source, uint32_t len);
int strcmp(const char *str1, const char *str2);
int strncmp(const char *p1, const char *p2, uint32_t len);
char *strstr(char *str, char *substr);
char *strchr(char *str, char c);

int strlen(char *str);
