#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "synch.h"
#include "keyboard.h"

// one slot is always open, so actual capacity is BUF_SIZE - 1
// static const doesn't work...
#define BUF_SIZE 1024

// remap right control
#define KBD_RCTRL 0x53

static int kbd_add_scancode(uint8_t sc);

// last used entry (space) at 0x39

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
static bool lctrl = false;
static bool rctrl = false;

enum state { IDLE, E0, E1_0, E1_1 };

static enum state kbd_state = IDLE;
static semaphore_t kbd_sema;

void kbd_init() {
    sema_init(&kbd_sema, 0);
}

// safe to call from interrupt handler
void kbd_on_scancode(uint8_t sc) {
    if (kbd_state == IDLE) {
        if (sc == 0xe0) {
            kbd_state = E0;
        } else if (sc == 0xe1) {
            kbd_state = E1_0;
        } else {
            kbd_add_scancode(sc);
        }
    } else if (kbd_state == E0) {
        if ((sc & 0x7f) == 0x1d) {  // right control
            kbd_add_scancode(KBD_RCTRL | (sc & 0x80));
        }
        kbd_state = IDLE;
    } else if (kbd_state == E1_0) {
        kbd_state = E1_1;
    } else if (kbd_state == E1_1) {
        kbd_state = IDLE;
    }
}

static int kbd_add_scancode(uint8_t sc) {
    size_t new_end = (end + 1) % BUF_SIZE;
    if (start == new_end) return 1;
    buf[end] = sc;
    end = new_end;
    sema_up(&kbd_sema);
    return 0;
}

int kbd_get_keycode(void) {
    //if (start == end) return -1;
    sema_down(&kbd_sema);
    uint8_t result = buf[start];
    start = (start + 1) % BUF_SIZE;
    if ((result & 0x7f) == 0x2a) lshift = !(result & 0x80);
    else if ((result & 0x7f) == 0x36) rshift = !(result & 0x80);
    else if ((result & 0x7f) == 0x1d) lctrl = !(result & 0x80);
    else if ((result & 0x7f) == KBD_RCTRL) rctrl = !(result & 0x80);
    return result;
}

bool kbd_get_shift(void) {
    return lshift || rshift;
}

bool kbd_get_ctrl(void) {
    return lctrl || rctrl;
}
