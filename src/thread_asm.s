global _thread_switch
global _thread_entry

extern thread_yield_finish

; thread_t* _thread_switch(thread_t* from, thread_t* to)
_thread_switch:
    mov ecx, [esp + 4]  ; from
    mov eax, ecx        ; return from
    mov [ecx + 8], esp
    mov [ecx + 12], ebx
    mov [ecx + 16], esi
    mov [ecx + 20], edi
    mov [ecx + 24], ebp
    mov ecx, [esp + 8]  ; to
    mov esp, [ecx + 8]
    mov ebx, [ecx + 12]
    mov esi, [ecx + 16]
    mov edi, [ecx + 20]
    mov ebp, [ecx + 24]
    ret

; entry point of all newly created threads
; void _thread_entry(void)
_thread_entry:
    ; this function is only reached through _thread_switch when a new thread is switched to,
    ; so eax always holds return value of _thread_switch: the previous thread
    push eax
    ; pass it to thread_yield_finish
    call thread_yield_finish
    ; interrupts are disabled up until here
    sti
    add esp, 4
    ret
