#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "keyboard.h"

// one slot is always open, so actual capacity is BUF_SIZE - 1
// static const doesn't work...
#define BUF_SIZE 1024

static int add_scancode(uint8_t sc);

const char kbd_ascii_map[256] = {
    0, 27 /*esc*/, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 8 /*bksp*/, '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0, 0, 0, ' ',
};

const char kbd_ascii_map_shift[256] = {
    0, 27 /*esc*/, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 8 /*bksp*/, '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', 0, 0, 0, ' ',
};

static uint8_t buf[BUF_SIZE];

static size_t start = 0;
static size_t end = 0;

static bool lshift = false;
static bool rshift = false;
static bool ctrl = false;

enum state { IDLE, E0, E1_0, E1_1 };

static enum state state = IDLE;

// safe across interrupts
int on_scancode(uint8_t sc) {
    if (state == IDLE) {
        if (sc == 0xe0) {
            state = E0;
        } else {
            return add_scancode(sc);
        }
    } else if (state == E0) {
        state = IDLE;
    } else if (state == E1_0) {
        state = E1_1;
    } else if (state == E1_1) {
        state = IDLE;
    }
    return 2;
}

static int add_scancode(uint8_t sc) {
    size_t new_end = (end + 1) % BUF_SIZE;
    if (start == new_end) return 1;
    buf[end] = sc;
    end = new_end;
    return 0;
}

int get_keycode(void) {
    if (start == end) return -1;
    uint8_t result = buf[start];
    start = (start + 1) % BUF_SIZE;
    if ((result & 0x7f) == 0x2a) lshift = !(result & 0x80);
    else if ((result & 0x7f) == 0x36) rshift = !(result & 0x80);
    else if ((result & 0x7f) == 0x1d) ctrl = !(result & 0x80);
    return result;
}

bool get_shift(void) {
    return lshift || rshift;
}

bool get_ctrl(void) {
    return ctrl;
}
