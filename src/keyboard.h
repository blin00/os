#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

int add_scancode(uint8_t sc);
int get_scancode(void);

extern const char kbd_ascii_map[256];

#endif
