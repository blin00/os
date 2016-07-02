#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "io.h"
#include "util.h"
#include "string.h"
#include "memory.h"
#include "random.h"
#include "keyboard.h"
#include "interrupt.h"
#include "thread.h"
#include "synch.h"
#include "shell.h"

void shell_prompt(void) {
    printf("[kernel@os]# ");
}

void test(void* arg) {
    printf("hi! %u\n", (uint32_t) arg);
    uint8_t buf[4242];
    while (true) {
        rand_data(buf, 4242);
        thread_yield();
    }
}

void shell_cmd(const char* cmd) {
    if (!strcmp(cmd, "help")) {
        printf("check the source ;)\n");
    } else if (!strcmp(cmd, "reboot")) {
        printf("rebooting...\n");
        _triple_fault();
    } else if (!strcmp(cmd, "heap")) {
        dump_heap();
    } else if (!strcmp(cmd, "rand")) {
        uint8_t buf[16];
        if (!rand_data(buf, 16)) {
            putbytes(buf, 16);
            putc('\n');
        } else {
            printf("not enough entropy\n");
        }
    } else if (!strcmp(cmd, "alloc")) {
        malloc(0x10000);
    } else if (!strcmp(cmd, "page")) {
        printf("trying to map single 4 MiB page...\n");
        test_enable_paging();
        printf("done\n");
    } else if (!strcmp(cmd, "info")) {
        printf("pit ticks: %u\n", pit_ticks);
        printf("rtc ticks: %lu\n", rtc_ticks);
        printf("spurious irq count: %lu\n", spurious_irq_count);
        printf("rdtsc: 0x%lx\n", __builtin_ia32_rdtsc());
    } else if (!strcmp(cmd, "test")) {
        thread_create(test, (void*) 42);
    } else if (!strcmp(cmd, "yield")) {
        thread_yield();
    } else if (strcmp(cmd, "")) {
        // not empty
        printf("unknown cmd \"%s\"\n", cmd);
    }
}

void shell(void) {
    const size_t BUFFER_LEN = 50;
    int sc;
    size_t len = 0;
    char buf[BUFFER_LEN + 1];
    buf[0] = '\0';
    shell_prompt();
    while (true) {
        sc = kbd_get_keycode();
        if (sc != -1) {
            char c = (kbd_get_shift() ? kbd_ascii_map_shift : kbd_ascii_map)[sc];
            if (c == 8) {
                if (len > 0) {
                    len--;
                    buf[len] = '\0';
                    putc(8);
                }
            } else if (c == 27) {
                while (len > 0) {
                    putc(8);
                    len--;
                }
                buf[0] = '\0';
            } else if (c == '\n') {
                putc('\n');
                shell_cmd(buf);
                len = 0;
                buf[0] = '\0';
                shell_prompt();
            } else if (kbd_get_ctrl() && c == 'c') {
                printf("^C\n");
                len = 0;
                buf[0] = '\0';
                shell_prompt();
            } else if (c && len < BUFFER_LEN) {
                buf[len++] = c;
                buf[len] = '\0';
                putc(c);
            }
        }
    }
}
