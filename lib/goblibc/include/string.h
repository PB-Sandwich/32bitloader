/**
 * @file string.h
 * @author Sofia "MetalPizzaCat"
 * @brief Implementation of some string functions
 * @version 0.1
 * @date 2025-07-07
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <stdint.h>

void *memcpy(void *dest, void *source, uint32_t size);
int memcmp(void *buf1, void *buf2, uint32_t size);
void *memset(void *buf, int n, uint32_t size);

char *strcpy(char *str_dest, char *str_source);
int strcmp(const char *str1, const char *str2);
int strncmp(const char *p1, const char *p2, int len);
char *strstr(char *str, char *substr);

int strlen(const char *str);
int strnlen(const char *str, int n);
/// @brief Convert error code to a string representation
/// @param  err Error code
/// @return Pointer to a static string containing the error message
char *strerror(int err);