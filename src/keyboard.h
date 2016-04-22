#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

void kbd_on_scancode(uint8_t sc);
int kbd_get_keycode(void);
bool kbd_get_shift(void);
bool kbd_get_ctrl(void);

extern const char kbd_ascii_map[256];
extern const char kbd_ascii_map_shift[256];

#endif
