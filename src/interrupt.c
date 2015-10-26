#include "random.h"
#include "util.h"
#include "io.h"
#include "interrupt.h"

static void ack_irq(uint32_t irq);
static void handle_irq(uint32_t irq);

volatile uint32_t timer_ticks;
volatile uint32_t rtc_ticks;
volatile uint32_t spurious_irq_count;

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
        if (sc == 0x01) // ESC
            _triple_fault();
        else if (sc == 0x39) {  // spacebar
            uint8_t buf[16];
            if (rand_data(buf, 16)) {
                printf("no data\n");
            } else {
                putbytes(buf, 16);
                putc('\n');
            }
            printf("spurious_irq_count: %u\n", spurious_irq_count);
        } else {
            printf("key %hhx\n", sc);
        }
        rand_on_kbd(timer_ticks);
        ack_irq(irq);
    } else if (irq == 8) {
        rtc_ticks++;
        rand_on_rtc(timer_ticks, rtc_ticks);
        outb(0x70, 0x0c);
        inb(0x71);
        ack_irq(irq);
    } else if (irq == 7 || irq == 15) {
        // spurious - acknowledge only master PIC
        if (irq == 15) ack_irq(7);
        spurious_irq_count++;
    } else {
        // something else
        printf("unknown IRQ 0x%hhx\n", irq);
    }
}

void setup_int(void) {
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
    // initialize PIT to ~66288 Hz (~1193182 / 18)
    outb(0x43, 0b00110100);
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
    // make IDT and enable interrupts
    build_idt();
    asm volatile("sti");
}

void interrupt_handler(cpu_state_t cpu, uint32_t interrupt, stack_state_t stack) {
    if (interrupt < 32) {
        // uh oh
        if (interrupt != 3) {
            printf("\n***** kernel panic *****\n");
            printf("unexpected exception 0x%hhx\n", interrupt);
            printf("eip: 0x%x esp: 0x%x\n", stack.eip, cpu.esp);
            BOCHS_BREAK;
            asm volatile("jmp _halt");
        } else {
            printf("\n***** kernel int3 *****\n");
            printf("eip: 0x%x esp: 0x%x\n", stack.eip, cpu.esp);
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
