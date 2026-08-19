; BlackMatrixOS Bootloader - 512 bytes
; Loads kernel from disk at 0x10000 and jumps to protected mode
; Uses EDD (BIOS int 0x13 AH=0x42) to read the kernel in 64KB chunks

[BITS 16]
[ORG 0x7C00]

KERNEL_ADDR equ 0x10000
KERNEL_SECTORS equ 512          ; Max kernel size 256 KB (increase if kernel grows)
SECTORS_PER_CHUNK equ 128       ; 128 sectors = 64 KB (one segment)

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    mov [drv], dl

    ; Load kernel via EDD, 64KB chunk per segment (0x1000, 0x2000, ...)
    mov cx, KERNEL_SECTORS / SECTORS_PER_CHUNK
    mov word [dap_seg], 0x1000
    mov dword [dap_lba], 1      ; Kernel starts at LBA 1 (after boot sector)

.load_chunk:
    mov word [dap_count], SECTORS_PER_CHUNK
    mov word [dap_off], 0
    mov ah, 0x42
    mov dl, [drv]
    lea si, [dap]
    int 0x13
    jc .err
    add word [dap_seg], SECTORS_PER_CHUNK * 512 / 16
    add dword [dap_lba], SECTORS_PER_CHUNK
    loop .load_chunk

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
dap:
    db 0x10             ; DAP size
    db 0                ; Reserved
dap_count: dw 0
dap_off:   dw 0
dap_seg:   dw 0x1000
dap_lba:   dq 1

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