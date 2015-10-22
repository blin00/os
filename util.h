#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#define BOCHS_BREAK asm volatile("xchg %bx, %bx")

void outb(uint16_t port, uint8_t data);
uint8_t inb(uint16_t port);
uint64_t _rdtsc(void);

#endif
