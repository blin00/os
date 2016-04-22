#ifndef IO_H
#define IO_H

#include <stddef.h>
#include <stdint.h>

enum vga_color {
    COLOR_BLACK = 0,
    COLOR_BLUE = 1,
    COLOR_GREEN = 2,
    COLOR_CYAN = 3,
    COLOR_RED = 4,
    COLOR_MAGENTA = 5,
    COLOR_BROWN = 6,
    COLOR_LIGHT_GREY = 7,
    COLOR_DARK_GREY = 8,
    COLOR_LIGHT_BLUE = 9,
    COLOR_LIGHT_GREEN = 10,
    COLOR_LIGHT_CYAN = 11,
    COLOR_LIGHT_RED = 12,
    COLOR_LIGHT_MAGENTA = 13,
    COLOR_LIGHT_BROWN = 14,
    COLOR_WHITE = 15,
};

void fb_write(const char* buf, size_t count);
void putc(char c);
void puts(const char* str);
void putu(uint32_t num);
void putlu(uint64_t num);
void putbytes(void* ptr, size_t num);
void printf(const char* fmt, ...);
void fb_init(size_t width, size_t height);
void fb_clear(void);
void fb_scroll(size_t amt);

#endif
