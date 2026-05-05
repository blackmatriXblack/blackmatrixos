[BITS 32]
[GLOBAL gdt_flush]
[GLOBAL tss_flush]

gdt_flush:
    mov eax, [esp + 4]    ; Get pointer to GDT descriptor
    lgdt [eax]             ; Load GDT

    mov ax, 0x10           ; Kernel data segment offset
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:.flush        ; Far jump to reload CS
.flush:
    ret

tss_flush:
    mov ax, 0x28 | 0x03   ; TSS segment offset (ring 3 selector, but we use it in ring 0)
    ltr ax
    ret
