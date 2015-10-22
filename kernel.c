#include <stdint.h>

#include "multiboot.h"
#include "io.h"
#include "util.h"
#include "interrupt.h"

/*
void build_idt(void) {
    idt_entry_t* x = &idt[42];
    uint32_t handler_addr = (uint32_t) interrupt_handler_42;
    x->offset_1 = handler_addr & 0xffff;
    x->selector = 0x08;
    // x->zero = 0; // .bss segment zeroed out by GRUB
    x->type_attr = 0x8f;
    x->offset_2 = (handler_addr >> 16) & 0xffff;
    setup_idt();
}
*/

void kmain(uint32_t magic, uint32_t info, uint32_t kernel_end) {
    fb_init();
    if (magic != 0x2BADB002) {
        printf("error: kernel not loaded by multiboot.\n");
        return;
    }
    multiboot_info_t* mb_info = (multiboot_info_t*) info;
    uint32_t free_start = kernel_end, free_end = 0;
    printf("memory map:\n");
    printf("lower: %u KB\nupper: %u KB\n", mb_info->mem_lower, mb_info->mem_upper);
    uint32_t mmap_ptr = mb_info->mmap_addr;
    while (mmap_ptr < mb_info->mmap_addr + mb_info->mmap_length) {
        multiboot_memory_map_t* mb_mmap = (multiboot_memory_map_t*) mmap_ptr;
        uint64_t temp_end = mb_mmap->addr + mb_mmap->len - 1;
        printf("0x%lx-0x%lx %s", mb_mmap->addr, temp_end, mb_mmap->type == 1 ? "free" : "reserved");
        if (kernel_end >= mb_mmap->addr && kernel_end <= temp_end) {
            if (temp_end > 0xffffffff) free_end = 0xffffffff;
            else free_end = (uint32_t) temp_end;
            printf(" (*)");
        }
        putc('\n');
        mmap_ptr += mb_mmap->size + 4;
    }
    printf("using mem: 0x%x-0x%x\n", free_start, free_end);
    setup_int();
    /*while (1) {
        uint32_t upper;
        uint32_t lower = _rdtsc(&upper);
        printf("0x%x%x\n", upper, lower);
    }*/
    printf("init done - press ESC to reboot\n");
    BOCHS_BREAK;
    while (1) {
        asm volatile("hlt");
    }
}
