#include <stdint.h>
#include <stddef.h>
#include "keyboard.h"

// one slot is always open, so actual capacity is BUF_SIZE - 1
// static const doesn't work...
#define BUF_SIZE 1024

const char kbd_ascii_map[256] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 127 /*bksp*/, 0,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0, 0, 0, ' '
};
static uint8_t buf[BUF_SIZE];

static size_t start = 0;
static size_t end = 0;

int add_scancode(uint8_t sc) {
    size_t new_end = (end + 1) % BUF_SIZE;
    if (start == new_end) return 1;
    buf[end] = sc;
    end = new_end;
    return 0;
}

int get_scancode(void) {
    if (start == end) return -1;
    uint8_t result = buf[start];
    start = (start + 1) % BUF_SIZE;
    return result;
}
