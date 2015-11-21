#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

int on_scancode(uint8_t sc);
int get_keycode(void);
bool get_shift(void);
bool get_ctrl(void);

extern const char kbd_ascii_map[256];
extern const char kbd_ascii_map_shift[256];

#endif
