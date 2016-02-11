global _thread_switch

_thread_switch:
    mov eax, [esp + 4]  ; from
    mov [eax + 4], esp
    mov [eax + 8], ebx
    mov [eax + 12], esi
    mov [eax + 16], edi
    mov [eax + 20], ebp
    mov eax, [esp + 8]  ; to
    mov esp, [eax + 4]
    mov ebx, [eax + 8]
    mov esi, [eax + 12]
    mov edi, [eax + 16]
    mov ebp, [eax + 20]
    ret
