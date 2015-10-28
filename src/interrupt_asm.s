%macro no_error_code_interrupt_handler 1
interrupt_handler_%1:
    push    dword 0                     ; push 0 as error code
    push    dword %1                    ; push the interrupt number
    jmp     common_interrupt_handler    ; jump to the common handler
%endmacro

%macro error_code_interrupt_handler 1
interrupt_handler_%1:
    push    dword %1                    ; push the interrupt number
    jmp     common_interrupt_handler    ; jump to the common handler
%endmacro

%macro build_idt_entry 1
    mov eax, interrupt_handler_%1
    mov [idt + 8 * %1], ax
    mov word [idt + 8 * %1 + 2], 0x08
    mov word [idt + 8 * %1 + 4], 0x8e00 ; e = interrupt gate, f = trap gate
    shr eax, 16
    mov [idt + %1 * 8 + 6], ax
%endmacro

global build_idt
global idt
global _triple_fault

extern interrupt_handler

section .text
build_idt:
%assign i 0
%rep 48
    build_idt_entry i
%assign i i + 1
%endrep

    build_idt_entry 48
    lidt [idtr]
    ret

%assign i 0
%rep 48
%if i == 8 || (i >= 10 && i <= 14) || i == 17
    error_code_interrupt_handler i
%else
    no_error_code_interrupt_handler i
%endif
%assign i i + 1
%endrep

no_error_code_interrupt_handler 48

common_interrupt_handler:
    cld
    pusha
    mov eax, [esp + 8 * 4]  ; interrupt number
    mov ebx, esp            ; the registers just pushed
    lea ecx, [esp + 9 * 4]  ; things on stack from interrupt
    push ecx
    push ebx
    push eax
    call interrupt_handler
    add esp, 12
    popa
    add esp, 8  ; interrupt number and error code
    iret

_triple_fault:
    mov word [idtr], 0
    lidt [idtr]
    int3
    jmp _triple_fault

section .data
align 4
    dw 0
idtr:
    dw idt.end - idt - 1
    dd idt

section .bss
align 8
idt:
    resq 256
.end:
