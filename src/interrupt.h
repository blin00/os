#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

#define ICW1_ICW4 0x01    /* ICW4 (not) needed */
#define ICW1_SINGLE 0x02    /* Single (cascade) mode */
#define ICW1_INTERVAL4 0x04    /* Call address interval 4 (8) */
#define ICW1_LEVEL 0x08    /* Level triggered (edge) mode */
#define ICW1_INIT 0x10    /* Initialization - required! */

#define ICW4_8086 0x01    /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO 0x02    /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE 0x08    /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0c    /* Buffered mode/master */
#define ICW4_SFNM 0x10    /* Special fully nested (not) */

#define PIC1 0x20    /* IO base address for master PIC */
#define PIC2 0xa0    /* IO base address for slave PIC */
#define PIC1_CMD PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_CMD PIC2
#define PIC2_DATA (PIC2 + 1)

#define PIC_EOI 0x20

#define PIC1_OFFSET 0x20
#define PIC2_OFFSET 0x28

struct __attribute__((packed)) idt_entry {
   uint16_t offset_1; // offset bits 0..15
   uint16_t selector; // a code segment selector in GDT or LDT
   uint8_t zero;      // unused, set to 0
   uint8_t type_attr; // type and attributes, see below
   uint16_t offset_2; // offset bits 16..31
};
typedef struct idt_entry idt_entry_t;

struct __attribute__((packed)) cpu_state {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
};
typedef struct cpu_state cpu_state_t;

struct __attribute__((packed)) stack_state {
    uint32_t error_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
};
typedef struct stack_state stack_state_t;

extern idt_entry_t idt[256];
extern volatile uint32_t pit_ticks;
extern volatile uint64_t rtc_ticks;
extern volatile uint64_t spurious_irq_count;

void int_init(void);
void _triple_fault(void);
void disable_apic(void);

#endif
