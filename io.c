#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

#include "io.h"
#include "util.h"
#include "string.h"

static volatile uint16_t* fb = (uint16_t*) 0xb8000;
static size_t row = 0;
static size_t col = 0;
static uint16_t port;

static char num_to_hex_lower[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
static char num_to_hex_upper[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

static void set_cursor(size_t loc);
static void update_cursor(void);
static void _putc(size_t loc, char c);

static void set_cursor(size_t loc) {
    outb(port, 14);
    outb(port + 1, (uint8_t) (loc >> 8));     // high byte
    outb(port, 15);
    outb(port + 1, (uint8_t) loc);             // low byte
}

static void update_cursor(void) {
    set_cursor(row * 80 + col);
}

static void _putc(size_t loc, char c) {
    uint16_t value = (0x07 << 8) | c;
    fb[loc] = value;
}

void write(const char* buf, size_t count) {
    for (size_t i = 0; i < count; i++) {
        char c = buf[i];
        if (c == '\n') {
            row++;
            col = 0;
        } else {
            _putc(row * 80 + col, c);
            if (++col >= 80) {
                row++;
                col = 0;
            }
        }
        if (row >= 25) {
            row = 24;
            fb_scroll(80);
        }
    }
    update_cursor();
}

void putc(char c) {
    write(&c, 1);
}

void puts(const char* str) {
    write(str, strlen(str));
    putc('\n');
}

void putu(uint32_t num) {
    if (num == 0) putc('0');
    else {
        if (num >= 10) putu(num / 10);
        putc('0' + num % 10);
    }
}

void putlu(uint64_t num) {
    if (num == 0) putc('0');
    else {
        if (num >= 10) putlu(num / 10);
        putc('0' + num % 10);
    }
}

void printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    while (*fmt) {
        if (*fmt != '%') {
            putc(*fmt);
        } else {
            size_t length = 4;  // # of bytes in the integer
            char type;
look_at_type:
            type = *++fmt;
            if (!type) break;
            else if (type == 'l') {
                length <<= 1;
                goto look_at_type;
            } else if (type == 'h') {
                length >>= 1;
                goto look_at_type;
            } else if (type == 'x' || type == 'X') {
                char* table = (type == 'x') ? num_to_hex_lower : num_to_hex_upper;
                if (length > 4) {
                    uint64_t num = va_arg(args, uint64_t);
                    for (int i = length * 2 - 1; i >= 0; i--) {
                        putc(table[(num >> (i * 4)) & 0xf]);
                    }
                } else {
                    uint32_t num = va_arg(args, uint32_t);
                    for (int i = length * 2 - 1; i >= 0; i--) {
                        putc(table[(num >> (i * 4)) & 0xf]);
                    }
                }
            } else if (type == 'u') {
                if (length > 4) {
                    putlu(va_arg(args, uint64_t));
                } else {
                    putu(va_arg(args, uint32_t));
                }
            } else if (type == 's') {
                const char* str = va_arg(args, const char*);
                write(str, strlen(str));
            } else if (type == 'c') {
                putc((char) va_arg(args, int));  // apparently %c takes int and turns it into char
            } else if (type == '%') {
                putc('%');
            } else {
                // ignore invalid format stuff
            }
        }
        fmt++;
    }
    va_end(args);
}

void fb_init(void) {
    port = *(uint16_t*) 0x0463;
    fb_clear();
}

void fb_clear(void) {
    for (size_t i = 0; i < 80 * 25; i++) _putc(i, ' ');
    set_cursor(0);
}

void fb_scroll(size_t amt) {
    size_t i;
    for (i = 0; i < 80 * 25 - amt; i++) {
        fb[i] = fb[i + amt];
    }
    for(; i < 80 * 25; i++) {
        _putc(i, ' ');
    }
}
