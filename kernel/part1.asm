[BITS 32]
[ORG 0x10000]
VGA equ 0xB8000
GFX equ 0xA0000
C80 equ 160
section .text
global _start
_start:
    mov esp,0x90000
    call kb_flush
    call cls
    call splash
    jmp shell_main
kb_flush:
    in al,0x64
    test al,1
    jz .ok
    in al,0x60
    jmp kb_flush
.ok:ret
cls:mov edi,VGA
    mov ecx,2000
    mov ax,0x0720
    rep stosw
    ret
ps: push eax
.lp:lodsb
    or al,al
    jz .r
    cmp al,10
    je .nl
    mov [edi],ax
    add edi,2
    jmp .lp
.nl:sub edi,VGA
    mov eax,edi
    xor edx,edx
    mov ecx,160
    div ecx
    inc eax
    mul ecx
    add eax,VGA
    mov edi,eax
    jmp .lp
.r: pop eax
    ret
p:  push edi
    mov edi,[cur]
    mov ah,[clr]
    call ps
    mov [cur],edi
    pop edi
    ret
nl: push eax edx ecx
    mov eax,[cur]
    sub eax,VGA
    xor edx,edx
    mov ecx,160
    div ecx
    inc eax
    mul ecx
    add eax,VGA
    mov [cur],eax
    pop ecx edx eax
    ret
pc: push edi
    mov edi,[cur]
    mov ah,[clr]
    mov [edi],ax
    add edi,2
    mov [cur],edi
    pop edi
    ret
pdec:
    push ebx ecx edx
    mov ecx,10
    xor ebx,ebx
    or eax,eax
    jnz .l1
    mov al,'0'
    call pc
    jmp .d
.l1:xor edx,edx
    div ecx
    push edx
    inc ebx
    or eax,eax
    jnz .l1
.l2:pop eax
    add al,'0'
    call pc
    dec ebx
    jnz .l2
.d: pop edx ecx ebx
    ret
phex4:
    cmp al,10
    jb .d
    add al,7
.d: add al,'0'
    call pc
    ret
phex:
    push eax
    shr al,4
    call phex4
    pop eax
    push eax
    and al,0x0F
    call phex4
    pop eax
    ret
phex32:
    push eax
    shr eax,24
    call phex
    pop eax
    push eax
    shr eax,16
    call phex
    pop eax
    push eax
    shr eax,8
    call phex
    pop eax
    call phex
    ret
wk: in al,0x64
    test al,1
    jz wk
    in al,0x60
    ret
wa: call wk
    test al,0x80
    jnz wa
    cmp al,0x3B
    je .f1
    cmp al,0x01
    je .esc
    jmp s2a
.f1:call gui_mode
    jmp wa
.esc:mov al,27
    ret
s2a:cmp al,54
    jae .x
    push ebx
    movzx ebx,al
    mov al,[stbl+ebx]
    pop ebx
    ret
.x: xor al,al
    ret
stbl:db 0,27,'1234567890-=',8,9
    db 'qwertyuiop[]',10,0
    db 'asdfghjkl;',39,'`'
    db 0,'\','z','x','c','v','b','n','m',',','.','/',0
    db '*',0,' '
scmp:
    push esi edi
.lp:mov al,[esi]
    cmp al,[edi]
    jne .ne
    or al,al
    jz .eq
    inc esi
    inc edi
    jmp .lp
.eq:pop edi esi
    ret
.ne:pop edi esi
    or eax,1
    ret
slen:
    push esi
    xor eax,eax
.lp:cmp byte [esi],0
    je .r
    inc esi
    inc eax
    jmp .lp
.r: pop esi
    ret
scpy:
    push esi edi
.lp:mov al,[esi]
    mov [edi],al
    inc esi
    inc edi
    or al,al
    jnz .lp
    pop edi esi
    ret
splash:
    mov byte [clr],0x0A
    mov esi,s1
    call p
    mov esi,s2
    call p
    mov esi,s3
    call p
    mov byte [clr],0x07
    mov esi,s4
    call p
    mov esi,s5
    call p
    mov esi,s6
    call p
    mov byte [clr],0x08
    mov esi,s7
    call p
    call nl
    ret
shell_main:
    mov dword [cur],VGA+C80*7
.lp:call prompt
    mov dword [inp_len],0
    mov byte [inp],0
.rd:call wa
    or al,al
    jz .rd
    cmp al,27
    je .halt
    cmp al,10
    je .exec
    cmp al,8
    je .bk
    mov ecx,[inp_len]
    cmp ecx,255
    jge .rd
    mov [inp+ecx],al
    inc dword [inp_len]
    mov byte [inp+ecx+1],0
    call pc
    jmp .rd
.bk:mov ecx,[inp_len]
    or ecx,ecx
    jz .rd
    dec dword [inp_len]
    push edi
    mov edi,[cur]
    sub edi,2
    mov word [edi],0x0720
    mov [cur],edi
    pop edi
    jmp .rd
.exec:
    call nl
    mov ecx,[inp_len]
    mov byte [inp+ecx],0
    call dispatch
    jmp shell_main
.halt:
    cli
    hlt
prompt:
    mov byte [clr],0x0A
    mov esi,p1
    call p
    mov byte [clr],0x0F
    mov esi,p2
    call p
    mov byte [clr],0x0B
    mov esi,p3
    call p
    mov byte [clr],0x07
    mov esi,p4
    call p
    ret
dispatch:
    mov esi,inp
    cmp byte [esi],0
    je .r
    mov edi,cmdtbl
.nx:cmp byte [edi],0
    je .unk
    push esi edi
    call scmp
    pop edi esi
    je .run
    push esi
    call slen
    pop esi
    add edi,eax
    inc edi
    add edi,4
    jmp .nx
.run:
    push esi
    call slen
    pop esi
    add edi,eax
    inc edi
    mov eax,[edi]
    call eax
    ret
.unk:
    mov byte [clr],0x0C
    mov esi,u1
    call p
    mov byte [clr],0x07
.r: ret
