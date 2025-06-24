
#pragma once

#include <stdint.h>

void* memcpy(void* dest, void* source, uint32_t size);
int memcmp(void* buf1, void* buf2, uint32_t size);

char* strcpy(char* str_dest, char* str_source);
int strcmp(char* str1, char* str2);
char* strstr(char* str, char* substr);

int strlen(char* str);
