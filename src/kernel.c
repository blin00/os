#include <stdint.h>

#include "multiboot.h"
#include "io.h"
#include "util.h"
#include "string.h"
#include "interrupt.h"
#include "memory.h"
#include "random.h"

void kmain(uint32_t magic, uint32_t info, uint32_t kernel_end) {
    fb_init(80, 25);
    if (magic != 0x2BADB002) {
        printf("error: kernel not loaded by multiboot.\n");
        return;
    }
    rand_init();
    multiboot_info_t* mb_info = (multiboot_info_t*) info;
    if (mb_info->flags & (1 << 9)) {
        char* name = (char*) mb_info->boot_loader_name;
        size_t len = strlen(name);
        if (len > 0) {
            rand_add_random_event((uint8_t*) name, len > 32 ? 32 : len, 255, 0);
            printf("kernel booted by %s\n", name);
        }
    }
    if (!(mb_info->flags & (1 << 6))) {
        printf("error: no memory map.\n");
        return;
    }
    uint32_t free_start = kernel_end, free_end = 0;
    printf("memory map:\n");
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
    if (!free_end) {
        printf("error: unable to detect usable memory.\n");
        return;
    }
    printf("using mem: 0x%x-0x%x\n", free_start, free_end);
    init_mem((void*) free_start, free_end - free_start + 1);
    setup_int();
    printf("init done - press ESC to reboot\n");
    BOCHS_BREAK;
    while (1) {
        asm volatile("hlt");
    }
}
