global outb
global inb
global _rdrand
global _rdseed
global _rdmsr
global _wrmsr

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

_rdrand:
    rdrand ecx
    xor eax, eax
    jnc .ret
    mov edx, [esp + 4]
    mov [edx], ecx
    inc eax
.ret:
    ret

_rdseed:
    rdseed ecx
    xor eax, eax
    jnc .ret
    mov edx, [esp + 4]
    mov [edx], ecx
    inc eax
.ret:
    ret

_rdmsr:
    mov ecx, [esp + 4]
    rdmsr
    ret

_wrmsr:
    mov ecx, [esp + 4]
    mov edx, [esp + 8]
    mov eax, [esp + 12]
    wrmsr
    ret
