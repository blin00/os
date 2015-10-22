global _start                   ; the entry symbol for ELF
global _halt
global gdt

extern kmain
extern _kernel_end

MAGIC_NUMBER        equ 0x1BADB002     ; define the magic number constant
MULTIB00T_FLAGS     equ 11b           ; multiboot flags
CHECKSUM            equ -(MAGIC_NUMBER + MULTIB00T_FLAGS)  ; (magic number + checksum + flags should equal 0)
KERNEL_STACK_SIZE   equ 16384

section .text                   ; start of the text (code) section
align 4                         ; header must be 4 byte aligned
    dd MAGIC_NUMBER
    dd MULTIB00T_FLAGS
    dd CHECKSUM

_start:
; set up our own gdt
    lgdt [gdtr]
    jmp 0x08:.cs
.cs:
    mov cx, 0x10
    mov ds, cx
    mov es, cx
    mov fs, cx
    mov gs, cx
    mov ss, cx
; set up stack and call kmain
    mov esp, kernel_stack_top
    push _kernel_end
    push ebx
    push eax
    call kmain
;   add esp, 12
; loop forever if main exits
_halt:
    cli
    hlt
    jmp _halt

section .data
align 8
gdt:
    dw 0
gdtr:
    dw .end - gdt - 1
    dd gdt

    dq 0x00CF9A000000FFFF       ; flat code
    dq 0x00CF92000000FFFF       ; flat data
.end:

section .bss
alignb 16
kernel_stack_bottom:
    resb KERNEL_STACK_SIZE
kernel_stack_top:
