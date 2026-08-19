[BITS 32]
section .text
[GLOBAL _context_switch]

; void context_switch(context_t* old_ctx, context_t* new_ctx)
; Save current context into old_ctx, load new_ctx
_context_switch:
    mov eax, [esp + 4]      ; old_ctx pointer
    mov edx, [esp + 8]      ; new_ctx pointer

    ; Save old context
    pushf                    ; Push EFLAGS
    pop ecx
    mov [eax + 36], ecx      ; eflags

    mov [eax + 0],  eax      ; Save registers (will be overwritten)
    mov [eax + 4],  ebx
    mov [eax + 8],  ecx
    mov [eax + 12], edx
    mov [eax + 16], esi
    mov [eax + 20], edi
    mov [eax + 24], ebp

    mov [esp + 4], esp       ; Save current ESP temporarily
    mov ecx, [esp + 4]
    mov [eax + 28], ecx      ; esp

    ; Save return address as EIP
    mov ecx, [esp]
    mov [eax + 32], ecx      ; eip

    ; Save segment registers
    mov cx, ds
    movzx ecx, cx
    mov [eax + 40], ecx      ; cs (approximation)
    mov [eax + 44], ecx      ; ds
    mov [eax + 48], ecx      ; es
    mov [eax + 52], ecx      ; fs
    mov [eax + 56], ecx      ; gs
    mov [eax + 60], ecx      ; ss

    ; Load new context
    mov ebp, [edx + 24]      ; ebp
    mov esp, [edx + 28]      ; esp

    ; Set up stack for iret to load new EIP
    push dword [edx + 60]    ; ss
    push dword [edx + 28]    ; esp
    push dword [edx + 36]    ; eflags
    push dword [edx + 40]    ; cs
    push dword [edx + 32]    ; eip

    ; Load general registers
    mov eax, [edx + 0]
    mov ebx, [edx + 4]
    mov ecx, [edx + 8]
    ; Don't load edx yet (we need it)
    mov esi, [edx + 16]
    mov edi, [edx + 20]

    ; Load segment registers
    push dword [edx + 44]    ; ds
    pop ds
    push dword [edx + 48]    ; es
    pop es
    push dword [edx + 52]    ; fs
    pop fs
    push dword [edx + 56]    ; gs
    pop gs

    ; Load edx last
    mov edx, [edx + 12]

    iret                     ; Jump to new process
