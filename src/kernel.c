#include <stdint.h>

#include "multiboot.h"
#include "io.h"
#include "util.h"
#include "string.h"
#include "interrupt.h"
#include "memory.h"
#include "random.h"
#include "keyboard.h"
#include "sha256.h"
#include "aes.h"

/*
void test_crypto() {
    uint8_t result[32];
    const char* test = "test string 123";
    const uint8_t key[] = {0x60,0x3d,0xeb,0x10,0x15,0xca,0x71,0xbe,0x2b,0x73,0xae,0xf0,0x85,0x7d,0x77,0x81,0x1f,0x35,0x2c,0x07,0x3b,0x61,0x08,0xd7,0x2d,0x98,0x10,0xa3,0x09,0x14,0xdf,0xf4};
    uint32_t key_exp[60];
    const uint8_t p1[] = {0x6b,0xc1,0xbe,0xe2,0x2e,0x40,0x9f,0x96,0xe9,0x3d,0x7e,0x11,0x73,0x93,0x17,0x2a};

    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, (const uint8_t*) test, strlen(test));
    sha256_update(&ctx, (const uint8_t*) "5", 1);
    sha256_final(&ctx, result);
    putbytes(result, 32);
    putc('\n');

    aes_key_setup(key, key_exp, 256);
    aes_encrypt(p1, result, key_exp, 256);
    putbytes(result, 16);
    putc('\n');
    aes_decrypt(result, result, key_exp, 256);
    putbytes(result, 16);
    putc('\n');
}
*/

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
    asm volatile("sti");
    while (1) {
        int sc;
        asm volatile("hlt");
        sc = get_scancode();
        if (sc != -1) {
            if (sc == 0x01) // ESC
                _triple_fault();
            else {
                char c = kbd_ascii_map[sc];
                if (c) {
                    putc(c);
                    if (c == '\n') puts("hai");
                }
            }
            /*
            else if (sc == 0x39) {  // spacebar
                uint8_t buf[16];
                asm volatile("cli");
                int result = rand_data(buf, 16);
                asm volatile("sti");
                if (result) {
                    printf("no data\n");
                } else {
                    putbytes(buf, 16);
                    putc('\n');
                }
                printf("spurious_irq_count: %u\n", spurious_irq_count);
            } else if (sc != 0xb9) {
                printf("key %hhx\n", sc);
            }
            */
        }
    }
}
