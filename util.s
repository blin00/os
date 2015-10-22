global outb
global inb
global _rdtsc

section .text
; outb - send a byte to an I/O port
; stack: [esp + 8] the data byte
;        [esp + 4] the I/O port
;        [esp    ] return address
outb:
    mov al, [esp + 8]    ; move the data to be sent into the al register
    mov dx, [esp + 4]    ; move the address of the I/O port into the dx register
    out dx, al           ; send the data to the I/O port
    ret                  ; return to the calling function

inb:
	mov dx, [esp + 4]
	in al, dx
	ret

_rdtsc:
    rdtsc
    ;mov ecx, [esp + 4]
    ;test ecx, ecx
    ;jz .ret
    ;mov [ecx], edx
.ret:
    ret
