; ==============================================================================
; BlackMatrixOS - Single file bootloader + kernel
; Minimalist OS with command line and GUI
; ==============================================================================

[BITS 16]
[ORG 0x7C00]

; ==============================================================================
; STAGE 1: Bootloader (fits in 512 bytes)
; ==============================================================================
boot_start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    mov [boot_drive], dl

    ; Load kernel from disk to 0x0800:0x0000 = 0x80000
    mov ax, 0x0800
    mov es, ax
    xor bx, bx
    mov ah, 0x02
    mov al, 100
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    int 0x13
    jc boot_disk_err

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

boot_disk_err:
    mov si, msg_err
    call bios_print
    jmp $

bios_print:
    pusha
.loop:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp .loop
.done:
    popa
    ret

msg_err: db 'Boot Error!', 0
boot_drive: db 0

align 8
gdt_null: dq 0
gdt_code: dw 0xFFFF,0, 0x9A00, 0x00CF
gdt_data: dw 0xFFFF,0, 0x9200, 0x00CF
gdt_end:

gdt_desc:
    dw gdt_end - gdt_null - 1
    dd gdt_null

times 510-($-$$) db 0
dw 0xAA55

; ==============================================================================
; STAGE 2: 32-bit Kernel (loaded at 0x80000)
; ==============================================================================
[BITS 32]

VGA equ 0xB8000
GFX equ 0xA0000
COL80 equ 160

pm_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Flush keyboard controller buffer
    call kb_flush

    ; Clear screen black
    call clear_screen

    ; Show welcome
    mov esi, s_banner
    mov edi, VGA
    mov ah, 0x0A
    call print_str_color
    mov esi, s_sep
    mov edi, VGA + COL80*2
    mov ah, 0x0A
    call print_str_color
    mov esi, s_welcome
    mov edi, VGA + COL80*3
    mov ah, 0x07
    call print_str_color
    mov esi, s_hint
    mov edi, VGA + COL80*4
    mov ah, 0x07
    call print_str_color
    mov esi, s_ver
    mov edi, VGA + COL80*5
    mov ah, 0x08
    call print_str_color

    mov dword [cursor], VGA + COL80*7
    jmp shell_main

; ==============================================================================
; Flush keyboard buffer
; ==============================================================================
kb_flush:
    in al, 0x64
    test al, 1
    jz .done
    in al, 0x60
    jmp kb_flush
.done:
    ret

; ==============================================================================
; Clear screen to black
; ==============================================================================
clear_screen:
    mov edi, VGA
    mov ecx, 2000
    mov ax, 0x0720
    rep stosw
    ret

; ==============================================================================
; Print null-terminated string at EDI with color AH
; ==============================================================================
print_str_color:
    push eax
    push edi
.loop:
    lodsb
    or al, al
    jz .done
    cmp al, 10
    je .newline
    mov [edi], ax
    add edi, 2
    jmp .loop
.newline:
    sub edi, VGA
    mov eax, edi
    xor edx, edx
    mov ecx, COL80
    div ecx
    inc eax
    mul ecx
    add eax, VGA
    mov edi, eax
    jmp .loop
.done:
    pop edi
    pop eax
    ret

; ==============================================================================
; Print single char in AL at [cursor] with color [color_attr]
; ==============================================================================
print_char:
    push edi
    mov edi, [cursor]
    mov ah, [color_attr]
    mov [edi], ax
    add edi, 2
    mov [cursor], edi
    pop edi
    ret

; ==============================================================================
; Move cursor to start of next line
; ==============================================================================
new_line:
    push eax
    push edx
    push ecx
    mov eax, [cursor]
    sub eax, VGA
    xor edx, edx
    mov ecx, COL80
    div ecx
    inc eax
    mul ecx
    add eax, VGA
    mov [cursor], eax
    pop ecx
    pop edx
    pop eax
    ret

; ==============================================================================
; Print prompt
; ==============================================================================
print_prompt:
    mov esi, s_prompt
    mov edi, [cursor]
    mov ah, 0x0A
    call print_str_color
    mov [cursor], edi
    ret

; ==============================================================================
; Wait for keypress, return ASCII in AL
; ==============================================================================
kb_wait_key:
    in al, 0x64
    test al, 1
    jz kb_wait_key
    in al, 0x60
    test al, 0x80
    jnz kb_wait_key
    cmp al, 0x3B
    je .f1
    cmp al, 0x01
    je .esc_key
    jmp scancode_to_ascii
.f1:
    call enter_gui_mode
    jmp kb_wait_key
.esc_key:
    mov al, 27
    ret

; ==============================================================================
; Scancode to ASCII (US layout)
; AL = scancode, returns ASCII in AL (0 = no mapping)
; ==============================================================================
scancode_to_ascii:
    cmp al, 54
    jae .nomap
    push ebx
    movzx ebx, al
    mov al, [scantbl + ebx]
    pop ebx
    ret
.nomap:
    xor al, al
    ret

scantbl:
    db 0, 27, '1','2','3','4','5','6','7','8','9','0','-','=',8
    db 9, 'q','w','e','r','t','y','u','i','o','p','[',']',10
    db 0, 'a','s','d','f','g','h','j','k','l',';',39,'`'
    db 0, '\','z','x','c','v','b','n','m',',','.','/',0
    db '*', 0, ' '

; ==============================================================================
; Shell main loop
; ==============================================================================
shell_main:
    call print_prompt
    mov dword [input_pos], 0
    mov byte [input_buf], 0

.input_loop:
    call kb_wait_key
    or al, al
    jz .input_loop

    cmp al, 27
    je do_halt
    cmp al, 10
    je .do_enter
    cmp al, 8
    je .do_back

    ; Regular character
    mov ecx, [input_pos]
    cmp ecx, 62
    jge .input_loop
    mov [input_buf + ecx], al
    inc dword [input_pos]
    mov byte [input_buf + ecx + 1], 0
    call print_char
    jmp .input_loop

.do_back:
    mov ecx, [input_pos]
    or ecx, ecx
    jz .input_loop
    dec dword [input_pos]
    mov eax, [cursor]
    sub eax, 2
    mov word [eax], 0x0720
    mov [cursor], eax
    jmp .input_loop

.do_enter:
    call new_line
    mov ecx, [input_pos]
    mov byte [input_buf + ecx], 0
    call execute_cmd
    jmp shell_main

; ==============================================================================
; String compare: ESI=a, EDI=b
; ZF=1 if equal, ZF=0 if not equal
; ==============================================================================
strcmp:
    push esi
    push edi
.loop:
    mov al, [esi]
    cmp al, [edi]
    jne .not_equal
    or al, al
    jz .equal
    inc esi
    inc edi
    jmp .loop
.equal:
    pop edi
    pop esi
    ret
.not_equal:
    pop edi
    pop esi
    or eax, 1
    ret

; ==============================================================================
; strncmp5: compare first 5 chars
; ==============================================================================
strncmp5:
    push esi
    push edi
    mov ecx, 5
.lp:
    mov al, [esi]
    cmp al, [edi]
    jne .ne
    inc esi
    inc edi
    dec ecx
    jnz .lp
    pop edi
    pop esi
    ret
.ne:
    pop edi
    pop esi
    or eax, 1
    ret

; ==============================================================================
; Execute command in input_buf
; ==============================================================================
execute_cmd:
    mov esi, input_buf
    cmp byte [esi], 0
    je .ret

    mov edi, c_help
    call strcmp
    je .do_help

    mov esi, input_buf
    mov edi, c_clear
    call strcmp
    je .do_clear

    mov esi, input_buf
    mov edi, c_ver
    call strcmp
    je .do_ver

    mov esi, input_buf
    mov edi, c_about
    call strcmp
    je .do_about

    mov esi, input_buf
    mov edi, c_gui
    call strcmp
    je .do_gui

    mov esi, input_buf
    mov edi, c_matrix
    call strcmp
    je .do_matrix

    mov esi, input_buf
    mov edi, c_reboot
    call strcmp
    je .do_reboot

    mov esi, input_buf
    mov edi, c_cowsay
    call strcmp
    je .do_cowsay

    mov esi, input_buf
    mov edi, c_whoami
    call strcmp
    je .do_whoami

    mov esi, input_buf
    mov edi, c_hostname
    call strcmp
    je .do_hostname

    mov esi, input_buf
    mov edi, c_uname
    call strcmp
    je .do_uname

    mov esi, input_buf
    mov edi, c_date
    call strcmp
    je .do_date

    mov esi, input_buf
    mov edi, c_lspci
    call strcmp
    je .do_lspci

    mov esi, input_buf
    mov edi, c_color
    call strcmp
    je .do_color

    mov esi, input_buf
    mov edi, c_mem
    call strcmp
    je .do_mem

    mov esi, input_buf
    mov edi, c_uptime
    call strcmp
    je .do_uptime

    mov esi, input_buf
    mov edi, s_echo_pre
    call strncmp5
    je .do_echo

    ; Unknown
    mov esi, s_unknown
    mov edi, [cursor]
    mov ah, 0x0C
    call print_str_color
    mov [cursor], edi
.ret:
    ret

.do_help:
    mov esi, s_help
    mov edi, [cursor]
    mov ah, 0x07
    call print_str_color
    mov [cursor], edi
    ret

.do_clear:
    call clear_screen
    mov dword [cursor], VGA + COL80*3
    ret

.do_ver:
    mov esi, s_ver_out
    mov edi, [cursor]
    mov ah, 0x0B
    call print_str_color
    mov [cursor], edi
    ret

.do_about:
    mov esi, s_about
    mov edi, [cursor]
    mov ah, 0x0B
    call print_str_color
    mov [cursor], edi
    ret

.do_gui:
    call enter_gui_mode
    ret

.do_matrix:
    call do_matrix_rain
    ret

.do_reboot:
    mov al, 0xFE
    out 0x64, al
    hlt

.do_cowsay:
    mov esi, s_cowsay
    mov edi, [cursor]
    mov ah, 0x0E
    call print_str_color
    mov [cursor], edi
    ret

.do_whoami:
    mov esi, s_root
    mov edi, [cursor]
    mov ah, 0x0A
    call print_str_color
    mov [cursor], edi
    ret

.do_hostname:
    mov esi, s_hostname
    mov edi, [cursor]
    mov ah, 0x0A
    call print_str_color
    mov [cursor], edi
    ret

.do_uname:
    mov esi, s_uname
    mov edi, [cursor]
    mov ah, 0x0B
    call print_str_color
    mov [cursor], edi
    ret

.do_date:
    mov esi, s_date
    mov edi, [cursor]
    mov ah, 0x0B
    call print_str_color
    mov [cursor], edi
    ret

.do_lspci:
    mov esi, s_lspci
    mov edi, [cursor]
    mov ah, 0x07
    call print_str_color
    mov [cursor], edi
    ret

.do_color:
    mov edi, [cursor]
    xor ecx, ecx
.cloop:
    mov al, '#'
    mov ah, cl
    mov [edi], ax
    add edi, 2
    inc ecx
    cmp ecx, 16
    jl .cloop
    mov [cursor], edi
    call new_line
    ret

.do_mem:
    mov esi, s_mem
    mov edi, [cursor]
    mov ah, 0x07
    call print_str_color
    mov [cursor], edi
    ret

.do_uptime:
    mov esi, s_uptime
    mov edi, [cursor]
    mov ah, 0x07
    call print_str_color
    mov [cursor], edi
    ret

.do_echo:
    mov esi, input_buf + 5
    mov edi, [cursor]
    mov ah, 0x0E
    call print_str_color
    mov [cursor], edi
    call new_line
    ret

; ==============================================================================
; Halt
; ==============================================================================
do_halt:
    cli
    hlt

; ==============================================================================
; Matrix rain effect
; ==============================================================================
matrix_cols: times 80 dd 0

do_matrix_rain:
    call clear_screen
    mov edi, matrix_cols
    mov ecx, 80
    xor eax, eax
.init:
    mov [edi], eax
    neg dword [edi]
    add eax, 3
    add edi, 4
    dec ecx
    jnz .init

.loop:
    in al, 0x64
    test al, 1
    jz .render
    in al, 0x60
    cmp al, 0x01
    je .exit
.render:
    mov esi, matrix_cols
    mov ecx, 80
    xor ebx, ebx
.col:
    mov eax, [esi]
    or eax, eax
    js .next
    cmp eax, 25
    jge .reset
    push eax
    push ebx
    imul eax, COL80
    lea edi, [VGA + eax]
    mov eax, ebx
    shl eax, 1
    add edi, eax
    mov eax, ebx
    add eax, [esi]
    and eax, 0x3F
    add al, 0x21
    mov ah, 0x0F
    mov [edi], ax
    sub edi, COL80
    cmp edi, VGA
    jb .no_prev
    mov ah, 0x0A
    mov [edi], ax
.no_prev:
    pop ebx
    pop eax
.next:
    inc dword [esi]
    add esi, 4
    inc ebx
    dec ecx
    jnz .col
    mov ecx, 500000
.delay:
    dec ecx
    jnz .delay
    jmp .loop
.reset:
    mov dword [esi], -20
    jmp .next
.exit:
    call clear_screen
    mov dword [cursor], VGA + COL80*3
    ret

; ==============================================================================
; VGA Mode 13h (320x200x256)
; ==============================================================================
set_mode13h:
    mov dx, 0x3C2
    mov al, 0x63
    out dx, al
    mov dx, 0x3D4
    mov ax, 0x0E11
    out dx, ax
    mov ax, 0x0D06
    out dx, ax
    mov ax, 0x3E07
    out dx, ax
    mov ax, 0x4109
    out dx, ax
    mov ax, 0xEA10
    out dx, ax
    mov ax, 0x2C11
    out dx, ax
    mov ax, 0xDF12
    out dx, ax
    mov ax, 0x0014
    out dx, ax
    mov ax, 0xE715
    out dx, ax
    mov ax, 0x0616
    out dx, ax
    mov ax, 0xE317
    out dx, ax
    mov dx, 0x3C4
    mov ax, 0x0101
    out dx, ax
    mov ax, 0x0E04
    out dx, ax
    mov dx, 0x3CE
    mov ax, 0x4005
    out dx, ax
    mov ax, 0x0506
    out dx, ax
    ret

; ==============================================================================
; Restore VGA text mode (via registers, not INT 10h)
; ==============================================================================
set_textmode:
    mov dx, 0x3C2
    mov al, 0x67
    out dx, al
    mov dx, 0x3D4
    mov ax, 0x0E11
    out dx, ax
    mov ax, 0x0D06
    out dx, ax
    mov ax, 0x3E07
    out dx, ax
    mov ax, 0x0009
    out dx, ax
    mov ax, 0xEA10
    out dx, ax
    mov ax, 0x8E11
    out dx, ax
    mov ax, 0xDF12
    out dx, ax
    mov ax, 0x2813
    out dx, ax
    mov ax, 0x0014
    out dx, ax
    mov ax, 0xE715
    out dx, ax
    mov ax, 0x0416
    out dx, ax
    mov ax, 0xE317
    out dx, ax
    mov dx, 0x3C4
    mov ax, 0x0101
    out dx, ax
    mov ax, 0x0302
    out dx, ax
    mov ax, 0x0003
    out dx, ax
    mov ax, 0x0204
    out dx, ax
    mov dx, 0x3CE
    mov ax, 0x1005
    out dx, ax
    mov ax, 0x0E06
    out dx, ax
    mov dx, 0x3C0
    mov al, 0x20
    out dx, al
    ret

; ==============================================================================
; GUI Mode
; ==============================================================================
enter_gui_mode:
    call set_mode13h

    ; Background
    mov edi, GFX
    mov ecx, 320*200
    mov al, 1
    rep stosb

    ; Taskbar
    mov edi, GFX + 320*180
    mov ecx, 320*20
    mov al, 8
    rep stosb

    ; Start button
    mov edi, GFX + 320*182 + 4
    mov ecx, 16
.sb_y:
    push ecx
    mov ecx, 50
.sb_x:
    mov byte [edi], 2
    inc edi
    dec ecx
    jnz .sb_x
    add edi, 320-50
    pop ecx
    dec ecx
    jnz .sb_y

    ; Window 1 - Terminal
    mov edi, GFX + 320*20 + 20
    mov ecx, 140
.w1_y:
    push ecx
    mov ecx, 200
.w1_x:
    mov byte [edi], 7
    inc edi
    dec ecx
    jnz .w1_x
    add edi, 320-200
    pop ecx
    dec ecx
    jnz .w1_y

    ; Title bar 1
    mov edi, GFX + 320*20 + 20
    mov ecx, 14
.t1_y:
    push ecx
    mov ecx, 200
.t1_x:
    mov byte [edi], 9
    inc edi
    dec ecx
    jnz .t1_x
    add edi, 320-200
    pop ecx
    dec ecx
    jnz .t1_y

    ; Close button
    mov edi, GFX + 320*22 + 208
    mov ecx, 10
.cb_y:
    push ecx
    mov ecx, 10
.cb_x:
    mov byte [edi], 4
    inc edi
    dec ecx
    jnz .cb_x
    add edi, 320-10
    pop ecx
    dec ecx
    jnz .cb_y

    ; Window 2 - System Info
    mov edi, GFX + 320*50 + 140
    mov ecx, 100
.w2_y:
    push ecx
    mov ecx, 150
.w2_x:
    mov byte [edi], 15
    inc edi
    dec ecx
    jnz .w2_x
    add edi, 320-150
    pop ecx
    dec ecx
    jnz .w2_y

    ; Title bar 2
    mov edi, GFX + 320*50 + 140
    mov ecx, 14
.t2_y:
    push ecx
    mov ecx, 150
.t2_x:
    mov byte [edi], 1
    inc edi
    dec ecx
    jnz .t2_x
    add edi, 320-150
    pop ecx
    dec ecx
    jnz .t2_y

    ; Desktop icon 1
    mov edi, GFX + 320*40 + 270
    mov ecx, 32
.ic1_y:
    push ecx
    mov ecx, 32
.ic1_x:
    mov byte [edi], 8
    inc edi
    dec ecx
    jnz .ic1_x
    add edi, 320-32
    pop ecx
    dec ecx
    jnz .ic1_y

    ; Desktop icon 2
    mov edi, GFX + 320*100 + 270
    mov ecx, 32
.ic2_y:
    push ecx
    mov ecx, 32
.ic2_x:
    mov byte [edi], 8
    inc edi
    dec ecx
    jnz .ic2_x
    add edi, 320-32
    pop ecx
    dec ecx
    jnz .ic2_y

    ; Wait for ESC
.gui_loop:
    in al, 0x64
    test al, 1
    jz .gui_loop
    in al, 0x60
    test al, 0x80
    jnz .gui_loop
    cmp al, 0x01
    jne .gui_loop

    call set_textmode
    call clear_screen
    ret

; ==============================================================================
; Data
; ==============================================================================
c_help:     db 'help', 0
c_clear:    db 'clear', 0
c_ver:      db 'ver', 0
c_about:    db 'about', 0
c_gui:      db 'gui', 0
c_matrix:   db 'matrix', 0
c_reboot:   db 'reboot', 0
c_cowsay:   db 'cowsay', 0
c_whoami:   db 'whoami', 0
c_hostname: db 'hostname', 0
c_uname:    db 'uname', 0
c_date:     db 'date', 0
c_lspci:    db 'lspci', 0
c_color:    db 'color', 0
c_mem:      db 'mem', 0
c_uptime:   db 'uptime', 0
s_echo_pre: db 'echo ', 0

s_banner:
    db '========================================', 10
    db '  BlackMatrixOS v1.0', 10
    db '========================================', 10, 0
s_sep:
    db '----------------------------------------', 10, 0
s_welcome:
    db '  The Matrix Operating System', 10, 0
s_hint:
    db '  Type "help" | "gui" or F1 for GUI', 10, 0
s_ver:
    db '  x86 32-bit | Build 2026-03-29', 10, 0

s_prompt:   db 'root@bmos$ ', 0
s_unknown:  db 'Unknown command. Type "help".', 10, 0

s_root:     db 'root', 10, 0
s_hostname: db 'blackmatrix-os', 10, 0
s_date:     db '2026-03-29 00:00:00 UTC', 10, 0
s_uptime:   db 'System uptime: boot complete', 10, 0

s_ver_out:
    db 'BlackMatrixOS v1.0.0', 10
    db 'Build: 2026-03-29', 10
    db 'Arch: x86 i686 32-bit', 10
    db 'Kernel: Monolithic', 10, 0

s_uname:
    db 'BlackMatrixOS blackmatrix-os 1.0.0 x86 i686 BlackMatrixOS', 10, 0

s_mem:
    db 'Memory: 128 MB total', 10
    db 'Used:  2 MB', 10
    db 'Free:  126 MB', 10, 0

s_about:
    db '  ____  _            _    _   _      _   _         ___  ____ ', 10
    db ' | __ )| | __ _  ___| | _| | | | ___| |_(_) ___   / _ \/ ___|', 10
    db ' |  _ \| |/ _` |/ __| |/ / |_| |/ _ \ __| |/ __| | | | \___ \', 10
    db ' | |_) | | (_| | (__|   <|  _  |  __/ |_| | (__  | |_| |___) |', 10
    db ' |____/|_|\__,_|\___|_|\_\_| |_|\___|\__|_|\___|  \___/|____/ ', 10
    db '', 10
    db '  BlackMatrixOS - The Matrix Operating System', 10
    db '  A minimalist OS with retro aesthetics', 10, 0

s_cowsay:
    db ' _________________________________', 10
    db '< BlackMatrixOS is udderly great! >', 10
    db ' ---------------------------------', 10
    db '        \   ^__^', 10
    db '         \  (oo)\_______', 10
    db '            (__)\       )\/\', 10
    db '                ||----w |', 10
    db '                ||     ||', 10, 0

s_lspci:
    db 'PCI Devices:', 10
    db '  00:00.0 Host bridge: Intel 440FX', 10
    db '  00:01.0 ISA bridge: Intel PIIX3', 10
    db '  00:01.1 IDE interface: Intel PIIX3 IDE', 10
    db '  00:02.0 VGA: Bochs/QEMU VGA', 10
    db '  00:03.0 Ethernet: RTL-8029', 10, 0

s_help:
    db 'BlackMatrixOS Commands:', 10
    db '  help     - Show this help', 10
    db '  clear    - Clear screen', 10
    db '  ver      - Version info', 10
    db '  about    - About BlackMatrixOS', 10
    db '  gui      - Graphical interface (or F1)', 10
    db '  matrix   - Matrix rain effect', 10
    db '  echo     - Print text', 10
    db '  color    - Color palette', 10
    db '  mem      - Memory info', 10
    db '  uptime   - System uptime', 10
    db '  whoami   - Current user', 10
    db '  hostname - System hostname', 10
    db '  uname    - System info', 10
    db '  date     - Date and time', 10
    db '  lspci    - PCI devices', 10
    db '  cowsay   - Talking cow', 10
    db '  reboot   - Restart system', 10, 0

; ==============================================================================
; BSS
; ==============================================================================
cursor:     dd VGA + COL80*7
color_attr: db 0x07
input_buf:  times 64 db 0
input_pos:  dd 0

times 51*1024-($-$$) db 0
