#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#define BOCHS_BREAK __asm__ __volatile__("xchg %bx, %bx")

void outb(uint16_t port, uint8_t data);
uint8_t inb(uint16_t port);
int _rdrand(uint32_t* rand);
int _rdseed(uint32_t* seed);
uint64_t _rdmsr(uint32_t msr);
void _wrmsr(uint32_t msr, uint32_t edx, uint32_t eax);

#endif
