#include <stddef.h>

#include "string.h"

size_t strlen(const char* str) {
    size_t i = 0;
    while (str[i]) i++;
    return i;
}

int strcmp(const char* s1, const char* s2) {
    size_t i = 0;
    while (s1[i] || s2[i]) {
        if (s1[i] < s2[i]) return -1;
        else if (s1[i] > s2[i]) return 1;
        i++;
    }
    return 0;
}

void* memset(void* ptr, int value, size_t size) {
    unsigned char* buf = (unsigned char*) ptr;
    for (size_t i = 0; i < size; i++) {
        buf[i] = (unsigned char) value;
    }
    return ptr;
}

int memcmp(const void* p1, const void* p2, size_t size) {
    const unsigned char* s1 = (const unsigned char*) p1;
    const unsigned char* s2 = (const unsigned char*) p2;
    for (size_t i = 0; i < size; i++) {
        if (s1[i] < s2[i]) return -1;
        else if (s1[i] > s2[i]) return 1;
    }
    return 0;
}

void* memcpy(void* restrict dst, const void* src, size_t size) {
    unsigned char* restrict _dst = (unsigned char* restrict) dst;
    const unsigned char* _src = (const unsigned char*) src;
    for (size_t i = 0; i < size; i++) {
        _dst[i] = _src[i];
    }
    return dst;
}

void* memmove(void* dst, const void* src, size_t size) {
    unsigned char* _dst = (unsigned char*) dst;
    const unsigned char* _src = (const unsigned char*) src;
    if (_dst < _src) {
        for (size_t i = 0; i < size; i++)
            _dst[i] = _src[i];
    } else {
        for (size_t i = size; i > 0; i--)
            _dst[i - 1] = _src[i - 1];
    }
    return dst;
}
