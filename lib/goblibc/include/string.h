
#pragma once

#include <stdint.h>

void *memcpy(void *dest, void *source, uint32_t size);
int memcmp(void *buf1, void *buf2, uint32_t size);
void *memset(void *buf, int n, uint32_t size);

char *strcpy(char *str_dest, char *str_source);
int strcmp(const char *str1, const char *str2);
int strncmp(const char *p1, const char *p2, int len);
char *strstr(char *str, char *substr);

int strlen(char *str);