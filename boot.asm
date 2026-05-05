; BlackMatrixOS Bootloader - 512 bytes
; Loads kernel from disk at 0x10000 and jumps to protected mode

[BITS 16]
[ORG 0x7C00]

KERNEL_ADDR equ 0x10000

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    mov [drv], dl

    ; Load 64 sectors from sector 2 to 0x1000:0x0000 = 0x10000
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    mov ah, 0x02
    mov al, 64
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [drv]
    int 0x13
    jc .err

    ; Enable A20
    in al, 0x92
    or al, 2
    out 0x92, al

    cli
    lgdt [gdt_desc]

    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:pm_entry

.err:
    mov si, msg_err
.prt:
    lodsb
    or al, al
    jz .halt
    mov ah, 0x0E
    int 0x10
    jmp .prt
.halt:
    jmp $

drv: db 0
msg_err: db 'Boot Error!', 0

align 8
gdt_null: dq 0
gdt_code: dw 0xFFFF,0, 0x9A00, 0x00CF
gdt_data: dw 0xFFFF,0, 0x9200, 0x00CF
gdt_end:
gdt_desc:
    dw gdt_end - gdt_null - 1
    dd gdt_null

[BITS 32]
pm_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    jmp KERNEL_ADDR

times 510-($-$$) db 0
dw 0xAA55
