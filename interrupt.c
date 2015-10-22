#include "interrupt.h"
#include "util.h"
#include "io.h"

static void ack_irq(uint32_t irq);
static void handle_irq(uint32_t irq);

static void ack_irq(uint32_t irq) {
    if (irq >= 8) {
        outb(PIC2_CMD, PIC_EOI);
    }
    outb(PIC1_CMD, PIC_EOI);
}

static void handle_irq(uint32_t irq) {
    if (irq == 0) {
        //printf("timer tick\n");
        ack_irq(irq);
    } else if (irq == 1) {
        uint8_t sc = inb(0x60);
        printf("key %hhx\n", sc);
        if (sc == 0x01) // ESC
            _triple_fault();
        ack_irq(irq);
    } else if (irq == 8) {
        printf("RTC tick\n");
        ack_irq(irq);
    } else {
        // something else
        printf("unknown IRQ 0x%hhx\n", irq);
    }
}

void setup_int(void) {
    // initialize PIC
    outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
    outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);
    outb(PIC1_DATA, PIC1_OFFSET);                 // ICW2: Master PIC vector offset
    outb(PIC2_DATA, PIC2_OFFSET);                 // ICW2: Slave PIC vector offset
    outb(PIC1_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    outb(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);
    // set PIC masks
    outb(PIC1_DATA, 0b11111000);    // PIT (timer), keyboard, cascade
    outb(PIC2_DATA, 0b11111110);    // CMOS RTC

    build_idt();
    asm volatile("sti");
}

void interrupt_handler(cpu_state_t cpu, uint32_t interrupt, stack_state_t stack) {
    if (interrupt < 32) {
        // uh oh
        if (interrupt != 3) {
            printf("\n***** kernel panic *****\n");
            printf("unexpected exception 0x%hhx\n", interrupt);
            printf("eip: 0x%x\n", stack.eip);
            BOCHS_BREAK;
            asm volatile("jmp _halt");
        } else {
            printf("\n***** kernel int3 *****\n");
            printf("eip: 0x%x\n", stack.eip);
            BOCHS_BREAK;
        }
        return;
    } else if (interrupt >= 0x20 && interrupt <= 0x2f) {
        handle_irq(interrupt - 0x20);
    } else if (interrupt == 0x30) {
        printf("got int 0x30\n");
    } else {
        printf("unknown int 0x%hhx\n", interrupt);
    }
}
