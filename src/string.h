#ifndef STRING_H
#define STRING_H

#include <stddef.h>

size_t strlen(const char* str);
int strcmp(const char* s1, const char* s2);
char* strcpy(char* dst, const char* src);
char* strcat(char* dst, const char* src);

void* memset(void* ptr, int value, size_t num);
int memcmp(const void* p1, const void* p2, size_t size);
void* memcpy(void* restrict dst, const void* src, size_t num);
void* memmove(void* dst, const void* src, size_t size);

#endif
