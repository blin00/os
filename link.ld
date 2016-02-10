ENTRY(_start)

SECTIONS {
    /* load at 1 MB */
    . = 0x00100000;
    /* align each section to 4 KB */
    _kernel_start = .;
    .text ALIGN(0x1000): {
        *(.multiboot)
        *(.text)
    }

    .rodata ALIGN(0x1000): {
        *(.rodata)
    }

    .data ALIGN(0x1000): {
        *(.data)
    }

    .bss ALIGN(0x1000): {
        *(COMMON)
        *(.bss)
        *(.stack)
    }
    _kernel_end = .;
}
