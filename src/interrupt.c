#include "random.h"
#include "util.h"
#include "io.h"
#include "keyboard.h"
#include "interrupt.h"

static void ack_irq(uint32_t irq);
static void handle_irq(uint32_t irq);
void build_idt(void);

volatile uint32_t timer_ticks;
volatile uint32_t rtc_ticks;
volatile uint32_t spurious_irq_count;

static char* exception_msg[] = {
    "divide by zero",
    "debug",
    "non-maskable interrupt",
    "breakpoint",
    "overflow",
    "bound range exceeded",
    "invalid opcode",
    "device not available",
    "double fault",
    "coprocessor segment overrun",
    "invalid tss",
    "segment not present",
    "stack segment fault",
    "general protection fault",
    "page fault",
    "reserved",
    "x87 floating-point exception",
    "alignment check",
    "machine check",
    "simd floating-point exception",
    "virtualization exception",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "security exception",
    "reserved"
};

static void ack_irq(uint32_t irq) {
    if (irq >= 8) {
        outb(PIC2_CMD, PIC_EOI);
    }
    outb(PIC1_CMD, PIC_EOI);
}

static void handle_irq(uint32_t irq) {
    if (irq == 0) {
        timer_ticks++;
        ack_irq(irq);
    } else if (irq == 1) {
        uint8_t sc = inb(0x60);
        on_scancode(sc);
        rand_on_kbd();
        ack_irq(irq);
    } else if (irq == 8) {
        rtc_ticks++;
        rand_on_rtc();
        outb(0x70, 0x0c);
        inb(0x71);
        ack_irq(irq);
    } else if (irq == 7 || irq == 15) {
        // spurious - acknowledge only master PIC
        if (irq == 15) ack_irq(7);
        spurious_irq_count++;
    }
}

void int_init(void) {
    uint8_t temp;
    timer_ticks = 0;
    rtc_ticks = 0;
    spurious_irq_count = 0;
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
    // initialize PIT to ~66288 Hz (1193181.666... / 18)
    outb(0x43, 0b00110110);
    outb(0x40, 18);
    outb(0x40, 0x00);
    // initialize RTC to 64 Hz (32768 >> (10 - 1))
    // also disables NMI (until next RTC interrupt)
    outb(0x70, 0x8a);
    temp = inb(0x71);
    outb(0x70, 0x8a);
    outb(0x71, (temp & 0xf0) | 10); // set last 4 bits
    outb(0x70, 0x8b);
    temp = inb(0x71);
    outb(0x70, 0x8b);
    outb(0x71, temp | 0x40);        // set bit 6
    // make IDT
    build_idt();
}

void interrupt_handler(uint32_t interrupt, cpu_state_t* cpu, stack_state_t* stack) {
    if (interrupt < 32) {
        // uh oh
        if (interrupt != 3) {
            printf("\n***** kernel panic *****\n");
            printf("unexpected exception 0x%hhx (%s)\n", interrupt, exception_msg[interrupt]);
            printf("eip: 0x%x esp: 0x%x\n", stack->eip, cpu->esp);
            BOCHS_BREAK;
            asm volatile("jmp _halt");
        } else {
            printf("\n***** kernel int3 *****\n");
            printf("eip: 0x%x esp: 0x%x\n", stack->eip, cpu->esp);
            BOCHS_BREAK;
        }
        return;
    } else if (interrupt >= 0x20 && interrupt <= 0x2f) {
        handle_irq(interrupt - 0x20);
    } else if (interrupt == 0x30) {
        printf("got int 0x30\n");
    }
}
