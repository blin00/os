#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

#include "io.h"
#include "util.h"
#include "string.h"

static size_t WIDTH;
static size_t HEIGHT;

static volatile uint16_t* fb = (uint16_t*) 0xb8000;
static size_t row;
static size_t col;
static uint16_t port;

static char num_to_hex_lower[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
static char num_to_hex_upper[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

static void set_cursor(size_t loc);
static void update_cursor(void);
static void _putc(size_t loc, char c);

static void set_cursor(size_t loc) {
    outb(port, 14);
    outb(port + 1, (uint8_t) (loc >> 8));       // high byte
    outb(port, 15);
    outb(port + 1, (uint8_t) loc);              // low byte
}

static void update_cursor(void) {
    set_cursor(row * WIDTH + col);
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
        } else if (c == 8) {
            if (row || col) {
                if (col) col--;
                else {
                    col = WIDTH - 1;
                    row--;
                }
            }
            _putc(row * WIDTH + col, ' ');
        } else if (c == '\t') {
            putc(' ');
        } else {
            _putc(row * WIDTH + col, c);
            if (++col >= WIDTH) {
                row++;
                col = 0;
            }
        }
        if (row >= HEIGHT) {
            row = HEIGHT - 1;
            fb_scroll(WIDTH);
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

void putbytes(void* ptr, size_t num) {
    uint8_t* buf = (uint8_t*) ptr;
    for (size_t i = 0; i < num; i++) {
        putc(num_to_hex_lower[(buf[i] >> 4) & 0xf]);
        putc(num_to_hex_lower[buf[i] & 0xf]);
    }
}

void printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size_t idx = 0;
    while (1) {
        if (fmt[idx] == '%' || !fmt[idx]) {
            // print buffered stuff
            write(fmt, idx);
            if (!fmt[idx]) break;
            fmt += idx;
            idx = 0;
            size_t length = 4;  // # of bytes in the integer
            char type;
look_at_type:
            type = (++fmt)[idx];
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
            fmt++;
        } else idx++;
    }
    va_end(args);
}

void fb_init(size_t width, size_t height) {
    port = *(uint16_t*) 0x0463;
    WIDTH = width;
    HEIGHT = height;
    fb_clear();
}

void fb_clear(void) {
    for (size_t i = 0; i < WIDTH * HEIGHT; i++) _putc(i, ' ');
    row = col = 0;
    set_cursor(0);
}

void fb_scroll(size_t amt) {
    size_t i;
    size_t debug = 0;
    for (i = 0; i < WIDTH * HEIGHT - amt; i++, debug++) {
        fb[i] = fb[i + amt];
    }
    for(; i < WIDTH * HEIGHT; i++) {
        _putc(i, ' ');
    }
}
