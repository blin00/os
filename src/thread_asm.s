global _thread_switch

_thread_switch:
    mov eax, [esp + 4]  ; from
    mov [eax + 8], esp
    mov [eax + 12], ebx
    mov [eax + 16], esi
    mov [eax + 20], edi
    mov [eax + 24], ebp
    mov eax, [esp + 8]  ; to
    mov esp, [eax + 8]
    mov ebx, [eax + 12]
    mov esi, [eax + 16]
    mov edi, [eax + 20]
    mov ebp, [eax + 24]
    ret
