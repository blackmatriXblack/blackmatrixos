[BITS 32]
[ORG 0x10000]
VGA equ 0xB8000
C80 equ 160
section .text
global _start
_start:
    mov esp,0x90000
    call kb_flush
    call cls
    call title_bar
    mov dword [cur],VGA+C80*2
    jmp shell

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

title_bar:
    mov edi,VGA
    mov ecx,80
    mov ax,0x1F20
.tl:mov [edi],ax
    add edi,2
    dec ecx
    jnz .tl
    mov esi,t_str
    mov edi,VGA
    mov ah,0x1F
.tp:lodsb
    or al,al
    jz .td
    mov [edi],ax
    add edi,2
    jmp .tp
.td:ret

p:  push edi
    mov edi,[cur]
    mov ah,[clr]
.pl:lodsb
    or al,al
    jz .pd
    cmp al,10
    je .pn
    mov [edi],ax
    add edi,2
    jmp .pl
.pn:sub edi,VGA
    mov eax,edi
    xor edx,edx
    mov ecx,160
    div ecx
    inc eax
    mul ecx
    add eax,VGA
    mov edi,eax
    jmp .pl
.pd:mov [cur],edi
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
    jmp .pd
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
.pd:pop edx ecx ebx
    ret

phex:
    push eax
    shr al,4
    call .n
    pop eax
    push eax
    and al,0x0F
    call .n
    pop eax
    ret
.n: cmp al,10
    jb .d
    add al,7
.d: add al,'0'
    call pc
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
    cmp al,0x3A
    jae wa
    jmp s2a
.f1:call gui_tui
    call cls
    call title_bar
    call kb_flush
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

shell:
    mov dword [cur],VGA+C80*2
.lp:call prompt
    mov dword [il],0
    mov byte [inp],0
.rd:call wa
    or al,al
    jz .rd
    cmp al,27
    je .ht
    cmp al,10
    je .ex
    cmp al,8
    je .bk
    mov ecx,[il]
    cmp ecx,254
    jge .rd
    mov [inp+ecx],al
    inc dword [il]
    mov byte [inp+ecx+1],0
    call pc
    jmp .rd
.bk:mov ecx,[il]
    or ecx,ecx
    jz .rd
    dec dword [il]
    push edi
    mov edi,[cur]
    sub edi,2
    mov word [edi],0x0720
    mov [cur],edi
    pop edi
    jmp .rd
.ex:call nl
    mov ecx,[il]
    mov byte [inp+ecx],0
    call dispatch
    ; Scroll if needed
    mov eax,[cur]
    sub eax,VGA
    xor edx,edx
    mov ecx,160
    div ecx
    cmp eax,23
    jl .ns
    call scroll_up
    mov dword [cur],VGA+C80*22
.ns:jmp shell
.ht:cli
    hlt

scroll_up:
    push esi edi ecx
    mov esi,VGA+C80*2
    mov edi,VGA+C80*1
    mov ecx,80*22
    rep movsw
    mov edi,VGA+C80*23
    mov ecx,80
    mov ax,0x0720
    rep stosw
    pop ecx edi esi
    ret

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
    mov edi,stbl2
.sx:cmp byte [edi],0
    je .cr
    push esi edi
    call scmp
    pop edi esi
    je .sr
    push esi
    call slen
    pop esi
    add edi,eax
    inc edi
    add edi,4
    jmp .sx
.sr:push esi
    call slen
    pop esi
    add edi,eax
    inc edi
    mov eax,[edi]
    call eax
    ret
.cr:mov esi,inp
    mov edi,rtbl
.rx:cmp byte [edi],0
    je .uk
    push esi edi
    call scmp
    pop edi esi
    je .rf
    push esi
    call slen
    pop esi
    add edi,eax
    inc edi
    add edi,4
    jmp .rx
.rf:push esi
    call slen
    pop esi
    add edi,eax
    inc edi
    mov esi,[edi]
    mov byte [clr],0x07
    call p
    call nl
    ret
.uk:mov byte [clr],0x0C
    mov esi,u1
    call p
    mov byte [clr],0x07
.r: ret

; ============ SPECIAL COMMAND TABLE ============
stbl2:
    db "help",0
    dd cmd_help
    db "clear",0
    dd cmd_cls
    db "cls",0
    dd cmd_cls
    db "gui",0
    dd cmd_gui
    db "matrix",0
    dd cmd_mat
    db "cmatrix",0
    dd cmd_mat
    db "reboot",0
    dd cmd_reboot
    db "halt",0
    dd cmd_halt
    db "shutdown",0
    dd cmd_halt
    db "poweroff",0
    dd cmd_halt
    db "echo",0
    dd cmd_echo
    db "calc",0
    dd cmd_calc
    db "seq",0
    dd cmd_seq
    db "color",0
    dd cmd_color
    db "yes",0
    dd cmd_yes
    db "printf",0
    dd cmd_printf
    db "hex",0
    dd cmd_hex
    db "dec",0
    dd cmd_dec
    db "factor",0
    dd cmd_factor
    db "expr",0
    dd cmd_expr
    db "exit",0
    dd cmd_exit
    db "quit",0
    dd cmd_exit
    db "logout",0
    dd cmd_exit
    db "banner",0
    dd cmd_banner
    db "cowsay",0
    dd cmd_cow
    db "neofetch",0
    dd cmd_neo
    db "fortune",0
    dd cmd_fort
    db "sl",0
    dd cmd_sl
    db "figlet",0
    dd cmd_fig
    db "cal",0
    dd cmd_cal
    db "date",0
    dd cmd_date
    db 0

; ============ HANDLERS ============
cmd_help:
    mov byte [clr],0x0B
    mov esi,s_help
    call p
    mov byte [clr],0x07
    call nl
    ret
cmd_cls:
    mov edi,VGA+C80
    mov ecx,80*23
    mov ax,0x0720
    rep stosw
    mov dword [cur],VGA+C80*2
    ret
cmd_gui:
    call gui_tui
    call cls
    call title_bar
    ret
cmd_reboot:
    mov al,0xFE
    out 0x64,al
    hlt
cmd_halt:
    cli
    hlt
cmd_exit:
    cli
    hlt
cmd_echo:
    mov esi,inp+5
    mov byte [clr],0x0E
    call p
    mov byte [clr],0x07
    call nl
    ret
cmd_calc:
    mov byte [clr],0x0A
    mov esi,s_calc
    call p
    mov byte [clr],0x07
    call nl
    ret
cmd_seq:
    mov byte [clr],0x07
    mov eax,1
    mov ecx,10
.sl:push eax ecx
    call pdec
    call nl
    pop ecx eax
    inc eax
    dec ecx
    jnz .sl
    ret
cmd_color:
    mov edi,[cur]
    xor ecx,ecx
.cl:mov al,219
    mov ah,cl
    mov [edi],ax
    add edi,2
    inc ecx
    cmp ecx,16
    jl .cl
    mov [cur],edi
    call nl
    ret
cmd_yes:
    mov byte [clr],0x0E
.yl:mov esi,s_yes
    call p
    in al,0x64
    test al,1
    jz .yl
    in al,0x60
    test al,0x80
    jz .ye
    jmp .yl
.ye:in al,0x60
    mov byte [clr],0x07
    ret
cmd_printf:
    mov esi,inp+7
    mov byte [clr],0x0E
    call p
    mov byte [clr],0x07
    call nl
    ret
cmd_hex:
    mov byte [clr],0x0A
    mov esi,s_hex
    call p
    mov byte [clr],0x07
    call nl
    ret
cmd_dec:
    mov byte [clr],0x0A
    mov esi,s_dec
    call p
    mov byte [clr],0x07
    call nl
    ret
cmd_factor:
    mov byte [clr],0x0B
    mov esi,s_fac
    call p
    mov byte [clr],0x07
    call nl
    ret
cmd_expr:
    mov byte [clr],0x0A
    mov esi,s_expr
    call p
    mov byte [clr],0x07
    call nl
    ret
cmd_banner:
    mov byte [clr],0x0A
    mov esi,s_ban
    call p
    mov byte [clr],0x07
    ret
cmd_cow:
    mov byte [clr],0x0E
    mov esi,s_cow
    call p
    mov byte [clr],0x07
    ret
cmd_neo:
    mov byte [clr],0x0A
    mov esi,s_neo
    call p
    mov byte [clr],0x07
    ret
cmd_fort:
    mov byte [clr],0x0E
    mov esi,s_fort
    call p
    mov byte [clr],0x07
    call nl
    ret
cmd_sl:
    mov byte [clr],0x0A
    mov esi,s_sl
    call p
    mov byte [clr],0x07
    ret
cmd_fig:
    mov byte [clr],0x0A
    mov esi,s_fig
    call p
    mov byte [clr],0x07
    ret
cmd_cal:
    mov byte [clr],0x07
    mov esi,s_cal
    call p
    ret
cmd_date:
    mov byte [clr],0x0B
    mov esi,s_dt
    call p
    mov byte [clr],0x07
    call nl
    ret
cmd_mat:
    mov byte [clr],0x0A
    mov esi,s_mat
    call p
    mov byte [clr],0x07
    ret

; ============ GUI TUI MODE ============
gui_tui:
    mov edi,VGA
    mov ecx,2000
    mov ax,0x1F20
    rep stosw
    ; Title
    mov esi,g_ttl
    mov edi,VGA
    mov ah,0x4F
.gt:lodsb
    or al,al
    jz .gtd
    mov [edi],ax
    add edi,2
    jmp .gt
.gtd:
    ; Window 1 title
    mov esi,g_w1t
    mov edi,VGA+C80*2+2
    mov ah,0x3F
.g1:lodsb
    or al,al
    jz .g1d
    mov [edi],ax
    add edi,2
    jmp .g1
.g1d:
    ; Window 1 content
    mov esi,g_w1c
    mov edi,VGA+C80*4+4
    mov ah,0x1F
.g1c:lodsb
    or al,al
    jz .g1cd
    cmp al,10
    je .g1cn
    mov [edi],ax
    add edi,2
    jmp .g1c
.g1cn:
    add edi,C80-4
    sub edi,4
    jmp .g1c
.g1cd:
    ; Window 2 title
    mov esi,g_w2t
    mov edi,VGA+C80*12+42
    mov ah,0x2F
.g2:lodsb
    or al,al
    jz .g2d
    mov [edi],ax
    add edi,2
    jmp .g2
.g2d:
    ; Window 2 content
    mov esi,g_w2c
    mov edi,VGA+C80*14+44
    mov ah,0x2F
.g2c:lodsb
    or al,al
    jz .g2cd
    cmp al,10
    je .g2cn
    mov [edi],ax
    add edi,2
    jmp .g2c
.g2cn:
    add edi,C80-44
    sub edi,44
    jmp .g2c
.g2cd:
    ; Icons
    mov esi,g_i1
    mov edi,VGA+C80*4+60
    mov ah,0x1E
.gi:lodsb
    or al,al
    jz .gid
    mov [edi],ax
    add edi,2
    jmp .gi
.gid:
    mov esi,g_i2
    mov edi,VGA+C80*7+60
    mov ah,0x1A
.gi2:lodsb
    or al,al
    jz .gi2d
    mov [edi],ax
    add edi,2
    jmp .gi2
.gi2d:
    ; Taskbar
    mov edi,VGA+C80*24
    mov ecx,80
    mov ax,0x7020
.tbl:mov [edi],ax
    add edi,2
    dec ecx
    jnz .tbl
    mov esi,g_st
    mov edi,VGA+C80*24+1
    mov ah,0x7A
.gs:lodsb
    or al,al
    jz .gsd
    mov [edi],ax
    add edi,2
    jmp .gs
.gsd:
    mov esi,g_ck
    mov edi,VGA+C80*24+68
    mov ah,0x70
.gc:lodsb
    or al,al
    jz .gcd
    mov [edi],ax
    add edi,2
    jmp .gc
.gcd:
    ; Wait ESC
.gw:in al,0x64
    test al,1
    jz .gw
    in al,0x60
    test al,0x80
    jnz .gw
    cmp al,0x01
    jne .gw
    ret

; ============ RESPONSE TABLE (300+ commands) ============
rtbl:
    db "ver",0
    dd r_ver
    db "uname",0
    dd r_un
    db "hostname",0
    dd r_hn
    db "whoami",0
    dd r_wa
    db "id",0
    dd r_id
    db "uptime",0
    dd r_up
    db "mem",0
    dd r_mem
    db "free",0
    dd r_mem
    db "lscpu",0
    dd r_cpu
    db "lspci",0
    dd r_pci
    db "lsusb",0
    dd r_usb
    db "lsblk",0
    dd r_blk
    db "dmesg",0
    dd r_dm
    db "env",0
    dd r_ev
    db "set",0
    dd r_ev
    db "who",0
    dd r_who
    db "w",0
    dd r_who
    db "last",0
    dd r_lg
    db "lastlog",0
    dd r_lg
    db "finger",0
    dd r_fng
    db "groups",0
    dd r_grp
    db "locale",0
    dd r_loc
    db "timedatectl",0
    dd r_dt
    db "hostnamectl",0
    dd r_hn
    db "localectl",0
    dd r_loc
    db "arch",0
    dd r_arch
    db "nproc",0
    dd r_npr
    db "getconf",0
    dd r_gcf
    db "ulimit",0
    dd r_ulm
    db "umask",0
    dd r_ums
    db "stty",0
    dd r_sty
    db "tty",0
    dd r_tty
    db "tput",0
    dd r_tpt
    db "ps",0
    dd r_ps
    db "top",0
    dd r_top
    db "htop",0
    dd r_top
    db "kill",0
    dd r_kil
    db "killall",0
    dd r_kil
    db "pkill",0
    dd r_kil
    db "nice",0
    dd r_nic
    db "renice",0
    dd r_nic
    db "nohup",0
    dd r_nhp
    db "timeout",0
    dd r_tmo
    db "jobs",0
    dd r_job
    db "bg",0
    dd r_bg
    db "fg",0
    dd r_fg
    db "wait",0
    dd r_wat
    db "vmstat",0
    dd r_vms
    db "iostat",0
    dd r_ios
    db "mpstat",0
    dd r_mps
    db "sar",0
    dd r_sar
    db "slabtop",0
    dd r_slb
    db "iftop",0
    dd r_ift
    db "ls",0
    dd r_ls
    db "dir",0
    dd r_ls
    db "cd",0
    dd r_cd
    db "pwd",0
    dd r_pwd
    db "cat",0
    dd r_cat
    db "tac",0
    dd r_tac
    db "head",0
    dd r_hed
    db "tail",0
    dd r_tal
    db "less",0
    dd r_les
    db "more",0
    dd r_les
    db "grep",0
    dd r_grp2
    db "egrep",0
    dd r_grp2
    db "fgrep",0
    dd r_grp2
    db "rg",0
    dd r_grp2
    db "ag",0
    dd r_grp2
    db "awk",0
    dd r_awk
    db "sed",0
    dd r_sed
    db "cut",0
    dd r_cut
    db "sort",0
    dd r_srt
    db "uniq",0
    dd r_unq
    db "wc",0
    dd r_wc
    db "nl",0
    dd r_nl
    db "rev",0
    dd r_rev
    db "tr",0
    dd r_tr
    db "tee",0
    dd r_tee
    db "xargs",0
    dd r_xrg
    db "paste",0
    dd r_pst
    db "join",0
    dd r_jon
    db "comm",0
    dd r_com
    db "diff",0
    dd r_dif
    db "patch",0
    dd r_pth
    db "fold",0
    dd r_fld
    db "expand",0
    dd r_exp
    db "unexpand",0
    dd r_uex
    db "colrm",0
    dd r_crm
    db "column",0
    dd r_col
    db "pr",0
    dd r_pr
    db "od",0
    dd r_hexd
    db "hexdump",0
    dd r_hexd
    db "xxd",0
    dd r_hexd
    db "strings",0
    dd r_str
    db "cmp",0
    dd r_cmp
    db "cksum",0
    dd r_ck
    db "sum",0
    dd r_ck
    db "md5sum",0
    dd r_md5
    db "sha1sum",0
    dd r_s1
    db "sha256sum",0
    dd r_s256
    db "sha512sum",0
    dd r_s512
    db "base64",0
    dd r_b64
    db "basename",0
    dd r_bas
    db "dirname",0
    dd r_dir
    db "realpath",0
    dd r_rlp
    db "readlink",0
    dd r_rlp
    db "file",0
    dd r_fil
    db "stat",0
    dd r_sta
    db "touch",0
    dd r_tch
    db "mkdir",0
    dd r_mkd
    db "rmdir",0
    dd r_rmd
    db "rm",0
    dd r_rm
    db "cp",0
    dd r_cp
    db "mv",0
    dd r_mv
    db "ln",0
    dd r_ln
    db "chmod",0
    dd r_chm
    db "chown",0
    dd r_cho
    db "chgrp",0
    dd r_chg
    db "dd",0
    dd r_dd
    db "truncate",0
    dd r_trc
    db "split",0
    dd r_spl
    db "csplit",0
    dd r_csp
    db "shred",0
    dd r_shd
    db "sync",0
    dd r_syn
    db "find",0
    dd r_fnd
    db "locate",0
    dd r_fnd
    db "tree",0
    dd r_tre
    db "df",0
    dd r_df
    db "du",0
    dd r_du
    db "mount",0
    dd r_mnt
    db "umount",0
    dd r_umt
    db "findmnt",0
    dd r_mnt
    db "blkid",0
    dd r_bki
    db "fdisk",0
    dd r_fdk
    db "mkfs",0
    dd r_mkf
    db "fsck",0
    dd r_fsc
    db "tune2fs",0
    dd r_tun
    db "e2label",0
    dd r_tun
    db "gzip",0
    dd r_gz
    db "gunzip",0
    dd r_gz
    db "bzip2",0
    dd r_gz
    db "bunzip2",0
    dd r_gz
    db "xz",0
    dd r_gz
    db "tar",0
    dd r_tar
    db "zip",0
    dd r_zip
    db "unzip",0
    dd r_zip
    db "cpio",0
    dd r_cpo
    db "ar",0
    dd r_ar
    db "7z",0
    dd r_7z
    db "rar",0
    dd r_rar
    db "ifconfig",0
    dd r_ifc
    db "ip",0
    dd r_ifc
    db "route",0
    dd r_rou
    db "arp",0
    dd r_arp
    db "ping",0
    dd r_png
    db "traceroute",0
    dd r_trt
    db "tracepath",0
    dd r_trt
    db "mtr",0
    dd r_trt
    db "netstat",0
    dd r_nst
    db "ss",0
    dd r_nst
    db "lsof",0
    dd r_lsf
    db "nslookup",0
    dd r_nsl
    db "dig",0
    dd r_nsl
    db "host",0
    dd r_nsl
    db "whois",0
    dd r_whi
    db "curl",0
    dd r_crl
    db "wget",0
    dd r_wgt
    db "ssh",0
    dd r_ssh
    db "scp",0
    dd r_scp
    db "rsync",0
    dd r_rsy
    db "nc",0
    dd r_nc
    db "nmap",0
    dd r_nmp
    db "tcpdump",0
    dd r_tdp
    db "iptables",0
    dd r_ipt
    db "nft",0
    dd r_ipt
    db "ufw",0
    dd r_ipt
    db "firewall-cmd",0
    dd r_ipt
    db "nmcli",0
    dd r_nmc
    db "systemctl",0
    dd r_syc
    db "journalctl",0
    dd r_jnl
    db "service",0
    dd r_syc
    db "init",0
    dd r_ini
    db "telinit",0
    dd r_ini
    db "passwd",0
    dd r_psw
    db "useradd",0
    dd r_uad
    db "userdel",0
    dd r_ude
    db "usermod",0
    dd r_uad
    db "groupadd",0
    dd r_gad
    db "groupdel",0
    dd r_ude
    db "su",0
    dd r_su
    db "sudo",0
    dd r_sdo
    db "apt",0
    dd r_apt
    db "apt-get",0
    dd r_apt
    db "yum",0
    dd r_yum
    db "dnf",0
    dd r_yum
    db "pacman",0
    dd r_pac
    db "snap",0
    dd r_snp
    db "flatpak",0
    dd r_snp
    db "pip",0
    dd r_pip
    db "npm",0
    dd r_npm
    db "gem",0
    dd r_gem
    db "cargo",0
    dd r_cgo
    db "gcc",0
    dd r_gcc
    db "g++",0
    dd r_gcc
    db "make",0
    dd r_mak
    db "cmake",0
    dd r_cmk
    db "python",0
    dd r_pyt
    db "python3",0
    dd r_pyt
    db "node",0
    dd r_nod
    db "git",0
    dd r_git
    db "svn",0
    dd r_svn
    db "java",0
    dd r_jav
    db "javac",0
    dd r_jav
    db "dotnet",0
    dd r_dot
    db "rustc",0
    dd r_rst
    db "go",0
    dd r_go
    db "perl",0
    dd r_per
    db "ruby",0
    dd r_rub
    db "php",0
    dd r_php
    db "lua",0
    dd r_lua
    db "sh",0
    dd r_sh
    db "bash",0
    dd r_sh
    db "man",0
    dd r_man
    db "info",0
    dd r_man
    db "apropos",0
    dd r_man
    db "tldr",0
    dd r_man
    db "history",0
    dd r_his
    db "alias",0
    dd r_ali
    db "unalias",0
    dd r_ali
    db "export",0
    dd r_exp2
    db "read",0
    dd r_rdd
    db "test",0
    dd r_tst
    db "which",0
    dd r_wch
    db "type",0
    dd r_wch
    db "whereis",0
    dd r_wch
    db "source",0
    dd r_src
    db ".",0
    dd r_src
    db "screenfetch",0
    dd r_neo
    db "hollywood",0
    dd cmd_mat
    db "about",0
    dd r_abt
    db "shuf",0
    dd r_shf
    db "pushd",0
    dd r_pud
    db "popd",0
    dd r_pud
    db "dirs",0
    dd r_pwd
    db "reset",0
    dd cmd_cls
    db "poweroff",0
    dd cmd_halt
    db "reboot",0
    dd cmd_reboot
    db 0

; ============ STATIC STRINGS ============
t_str: db ' BlackMatrixOS v3.0 - [Command Prompt]                         F1=GUI | ESC=Exit',0
p1: db 'root',0
p2: db '@',0
p3: db 'bmos',0
p4: db '$ ',0
u1: db 'Unknown command. Type "help" for all commands.',10,0
s_yes: db 'y',10,0
s_calc: db 'Calculator: calc <expr> (e.g. calc 2+3)',0
s_hex: db 'Hex: 0xFF = 255 | 0x100 = 256',0
s_dec: db 'Dec: 255 = 0xFF | 256 = 0x100',0
s_fac: db 'Factor: 12 = 2 2 3 | 100 = 2 2 5 5',0
s_expr: db 'Expr: 5+3=8 | 10-3=7 | 6*7=42 | 100/4=25',0
s_dt: db 'Sat Mar 29 12:00:00 UTC 2026',0

; ============ RESPONSE STRINGS ============
r_ver: db 'BlackMatrixOS v3.0 | Build 20260329 | i686',0
r_un: db 'BlackMatrixOS bmos 3.0.0 i686 BlackMatrixOS',0
r_hn: db 'bmos',0
r_wa: db 'root',0
r_id: db 'uid=0(root) gid=0(root) groups=0(root)',0
r_up: db 'up 0 days, 0:42, 1 user, load avg: 0.00 0.00 0.00',0
r_mem: db 'Mem: 65536K total | 32768K used | 32768K free | 4096K shared',0
r_cpu: db 'CPU: BlackMatrix Virtual i686 @ 100MHz | 1 core | 32KB L1 | 256KB L2',0
r_pci: db '00:00.0 Host bridge: BlackMatrix Virtual',0
r_usb: db 'Bus 001 Device 001: ID 0000:0000 Root Hub',0
r_blk: db 'NAME  SIZE TYPE MOUNTPOINT',10,'fd0   1.4M floppy',10,'ram0  64M  ramdisk',0
r_dm: db '[0.000000] BlackMatrixOS kernel loaded at 0x10000',0
r_ev: db 'PATH=/bin:/sbin:/usr/bin',10,'HOME=/root',10,'USER=root',10,'SHELL=/bin/sh',10,'TERM=bmos',0
r_who: db 'root  tty1  2026-03-29 12:00',0
r_lg: db 'root  pts/0  127.0.0.1  Sat Mar 29 12:00 - 12:42 (00:42)',0
r_fng: db 'Login: root  Name: Administrator',10,'Directory: /root  Shell: /bin/sh',0
r_grp: db 'root',0
r_loc: db 'LANG=en_US.UTF-8',10,'LC_ALL=en_US.UTF-8',0
r_dt: db 'Local time: Sat 2026-03-29 12:00:00 UTC',10,'Time zone: UTC (UTC, +0000)',0
r_arch: db 'i686',0
r_npr: db '1',0
r_gcf: db 'PAGESIZE: 4096',10,'CLK_TCK: 100',10,'OPEN_MAX: 1024',0
r_ulm: db 'unlimited',0
r_ums: db '0022',0
r_sty: db 'speed 38400 baud; line = 0;',0
r_tty: db '/dev/tty1',0
r_tpt: db 'Terminal: bmos  Lines: 25  Columns: 80',0
r_ps: db '  PID TTY  TIME CMD',10,'    1 ?  00:00 init',10,'    2 ?  00:00 shell',0
r_top: db 'top - 12:00:00 up 0:42, 1 user',10,'Tasks: 2 total, 2 running',10,'CPU: 0% used',10,'Mem: 65536K total, 32768K free',0
r_kil: db 'Usage: kill <pid>  |  kill -9 <pid>',0
r_nic: db 'Usage: nice -n <priority> <cmd>',0
r_nhp: db 'Usage: nohup <command> &',0
r_tmo: db 'Usage: timeout <duration> <command>',0
r_job: db '[1]+  Running  shell',0
r_bg: db '[1]+ shell &',0
r_fg: db '[1]+ shell',0
r_wat: db 'Waiting for background processes...',0
r_vms: db 'procs  memory  swap  io  system  cpu',10,'  2   32768K  0K  0  0  0 100 0',0
r_ios: db 'Device  tps  kB_read/s  kB_wrtn/s',10,'fd0     0.00  0.00       0.00',0
r_mps: db 'CPU  %usr  %sys  %idle',10,'all  0.00  0.00  100.00',0
r_sar: db '12:00:00  CPU  %user  %system  %idle',10,'12:00:00  all  0.00   0.00     100.00',0
r_slb: db 'Active / Total Objects: 0 / 0',10,'Active / Total Slabs: 0 / 0',0
r_ift: db 'interface: lo   TX: 0b/s  RX: 0b/s',0
r_ls: db 'bin  dev  etc  home  lib  proc  root  sbin  tmp  usr  var',0
r_cd: db 'Changed to /root',0
r_pwd: db '/root',0
r_cat: db 'Usage: cat <filename>',0
r_tac: db 'Usage: tac <filename>',0
r_hed: db 'Usage: head [-n N] <filename>',0
r_tal: db 'Usage: tail [-n N] <filename>',0
r_les: db 'Usage: less <filename>',0
r_grp2: db 'Usage: grep <pattern> <file>',0
r_awk: db 'Usage: awk <program> <file>',0
r_sed: db 'Usage: sed <script> <file>',0
r_cut: db 'Usage: cut -d<delim> -f<fields> <file>',0
r_srt: db 'Usage: sort <file>',0
r_unq: db 'Usage: uniq <file>',0
r_wc: db 'Usage: wc <file>  (lines, words, chars)',0
r_nl: db 'Usage: nl <file>',0
r_rev: db 'Usage: rev <file>',0
r_tr: db 'Usage: tr <set1> <set2>',0
r_tee: db 'Usage: tee <file>',0
r_xrg: db 'Usage: <input> | xargs <command>',0
r_pst: db 'Usage: paste <file1> <file2>',0
r_jon: db 'Usage: join <file1> <file2>',0
r_com: db 'Usage: comm <file1> <file2>',0
r_dif: db 'Usage: diff <file1> <file2>',0
r_pth: db 'Usage: patch < <patchfile>',0
r_fld: db 'Usage: fold -w <width> <file>',0
r_exp: db 'Usage: expand <file>',0
r_uex: db 'Usage: unexpand <file>',0
r_crm: db 'Usage: colrm <start> [<end>]',0
r_col: db 'Usage: column -t <file>',0
r_pr: db 'Usage: pr <file>',0
r_hexd: db 'Usage: hexdump <file>',0
r_str: db 'Usage: strings <file>',0
r_cmp: db 'Usage: cmp <file1> <file2>',0
r_ck: db 'Usage: cksum <file>',0
r_md5: db 'Usage: md5sum <file>',0
r_s1: db 'Usage: sha1sum <file>',0
r_s256: db 'Usage: sha256sum <file>',0
r_s512: db 'Usage: sha512sum <file>',0
r_b64: db 'Usage: base64 -e|-d <file>',0
r_bas: db 'Usage: basename <path>',0
r_dir: db 'Usage: dirname <path>',0
r_rlp: db 'Usage: realpath <path>',0
r_fil: db 'Usage: file <filename>',0
r_sta: db 'Usage: stat <filename>',0
r_tch: db 'Usage: touch <filename>',0
r_mkd: db 'Usage: mkdir <dirname>',0
r_rmd: db 'Usage: rmdir <dirname>',0
r_rm: db 'Usage: rm <file>',0
r_cp: db 'Usage: cp <source> <dest>',0
r_mv: db 'Usage: mv <source> <dest>',0
r_ln: db 'Usage: ln -s <target> <link>',0
r_chm: db 'Usage: chmod <mode> <file>',0
r_cho: db 'Usage: chown <user> <file>',0
r_chg: db 'Usage: chgrp <group> <file>',0
r_dd: db 'Usage: dd if=<in> of=<out> bs=<N>',0
r_trc: db 'Usage: truncate -s <size> <file>',0
r_spl: db 'Usage: split -l <N> <file>',0
r_csp: db 'Usage: csplit <file> <pattern>',0
r_shd: db 'Usage: shred <file>',0
r_syn: db 'Syncing all filesystems...',0
r_fnd: db 'Usage: find <path> -name <pattern>',0
r_tre: db '/root',10,'|-- bin',10,'|-- etc',10,'|-- home',10,'|-- lib',10,'|-- root',10,'`-- var',0
r_df: db 'Filesystem  Size  Used  Avail  Use%  Mounted on',10,'/dev/ram0   64M   32M   32M    50%   /',0
r_du: db 'Usage: du [-sh] <path>',0
r_mnt: db '/dev/ram0 on / type ext2 (rw)',0
r_umt: db 'Usage: umount <device>',0
r_bki: db '/dev/ram0: UUID="0000-0000" TYPE="ext2"',0
r_fdk: db 'Usage: fdisk <device>',0
r_mkf: db 'Usage: mkfs.ext4 <device>',0
r_fsc: db 'Usage: fsck <device>',0
r_tun: db 'Usage: tune2fs <device>',0
r_gz: db 'Usage: gzip <file>',0
r_tar: db 'Usage: tar cf <archive> <files>',0
r_zip: db 'Usage: zip <archive> <files>',0
r_cpo: db 'Usage: cpio -o < filelist',0
r_ar: db 'Usage: ar r <archive> <files>',0
r_7z: db 'Usage: 7z a <archive> <files>',0
r_rar: db 'Usage: rar a <archive> <files>',0
r_ifc: db 'lo: flags=73<UP,LOOPBACK,RUNNING>',10,'  inet 127.0.0.1  netmask 255.0.0.0',0
r_rou: db 'Kernel IP routing table',10,'Destination  Gateway  Genmask  Iface',10,'127.0.0.0    *        lo       lo',0
r_arp: db 'Address  HWtype  HWaddress  Flags Mask  Iface',0
r_png: db 'PING 127.0.0.1 (127.0.0.1): 56 data bytes',10,'64 bytes from 127.0.0.1: time=0.042ms',0
r_trt: db 'traceroute to localhost (127.0.0.1), 30 hops max',10,'1  localhost (127.0.0.1)  0.042ms',0
r_nst: db 'Netid  State  Recv-Q  Send-Q  Local  Peer',10,'tcp    LISTEN 0       0       *:0    *:*',0
r_lsf: db 'COMMAND  PID  USER  FD  TYPE  DEVICE  SIZE  NODE NAME',0
r_nsl: db 'Server:  127.0.0.1',10,'Address: 127.0.0.1#53',10,'Name: localhost',10,'Address: 127.0.0.1',0
r_whi: db 'Domain: localhost',10,'Status: Active',0
r_crl: db 'Usage: curl <url>',0
r_wgt: db 'Usage: wget <url>',0
r_ssh: db 'Usage: ssh <user>@<host>',0
r_scp: db 'Usage: scp <source> <user>@<host>:<dest>',0
r_rsy: db 'Usage: rsync <source> <dest>',0
r_nc: db 'Usage: nc <host> <port>',0
r_nmp: db 'Usage: nmap <target>',0
r_tdp: db 'Usage: tcpdump -i <iface>',0
r_ipt: db 'Chain INPUT (policy ACCEPT)',10,'target  prot  opt  source  destination',0
r_nmc: db 'DEVICE  TYPE  STATE  CONNECTION',10,'lo      loopback  connected  lo',0
r_syc: db 'UNIT  LOAD  ACTIVE  SUB  DESCRIPTION',10,'init.service  loaded  active  running  Init',0
r_jnl: db 'No journal entries found.',0
r_ini: db 'Current runlevel: 3  (multi-user)',0
r_psw: db 'Usage: passwd <username>',0
r_uad: db 'Usage: useradd <username>',0
r_ude: db 'Usage: userdel <username>',0
r_gad: db 'Usage: groupadd <groupname>',0
r_su: db 'Usage: su <username>',0
r_sdo: db 'Usage: sudo <command>',0
r_apt: db 'Usage: apt install|remove|update|upgrade <pkg>',0
r_yum: db 'Usage: yum install|remove|update <pkg>',0
r_pac: db 'Usage: pacman -S|-R|-Syu <pkg>',0
r_snp: db 'Usage: snap install|remove|list <pkg>',0
r_pip: db 'Usage: pip install|uninstall <package>',0
r_npm: db 'Usage: npm install|uninstall <package>',0
r_gem: db 'Usage: gem install|uninstall <gem>',0
r_cgo: db 'Usage: cargo build|run|test',0
r_gcc: db 'Usage: gcc [options] <source.c>',0
r_mak: db 'Usage: make [target]',0
r_cmk: db 'Usage: cmake <path>',0
r_pyt: db 'Python 3.12.0 (BlackMatrixOS)',10,'Type "help" for more info.',0
r_nod: db 'Node.js v22.0.0 (BlackMatrixOS)',0
r_git: db 'git version 2.43.0 (BlackMatrixOS)',0
r_svn: db 'svn, version 1.14.0 (BlackMatrixOS)',0
r_jav: db 'openjdk version "21.0.0" (BlackMatrixOS)',0
r_dot: db '.NET SDK 8.0.100 (BlackMatrixOS)',0
r_rst: db 'rustc 1.75.0 (BlackMatrixOS)',0
r_go: db 'go version go1.22.0 (BlackMatrixOS)',0
r_per: db 'perl 5.38.0 (BlackMatrixOS)',0
r_rub: db 'ruby 3.3.0 (BlackMatrixOS)',0
r_php: db 'PHP 8.3.0 (BlackMatrixOS)',0
r_lua: db 'Lua 5.4.6 (BlackMatrixOS)',0
r_sh: db 'BlackMatrixOS Shell v3.0',0
r_man: db 'Usage: man <command>',10,'No manual entry.',0
r_his: db '    1  help',10,'    2  ver',10,'    3  uname',10,'    4  whoami',0
r_ali: db 'alias ll="ls -la"',10,'alias la="ls -a"',0
r_exp2: db 'Usage: export VAR=value',0
r_rdd: db 'Usage: read <variable>',0
r_tst: db 'Usage: test <expression>',0
r_wch: db '/usr/bin/sh',0
r_src: db 'Usage: source <script>',0
r_abt: db 'BlackMatrixOS v3.0',10,'A hobby operating system kernel.',10,'Written in x86 assembly.',10,'(c) 2026 BlackMatrix Project',0
r_shf: db 'Usage: shuf [-n N] <file>',0
r_pud: db 'Usage: pushd|popd <dir>',0
r_neo: db '       _____ _',10,'      | __  |_|_ _ _ _',10,'      | __ -| | | | | |',10,'      |_____|_|___|___|',10,'  BlackMatrixOS v3.0 i686',10,'  Kernel: 3.0.0',10,'  Uptime: 0:42',10,'  Shell: /bin/sh',10,'  Terminal: bmos',10,'  CPU: BlackMatrix Virtual @ 100MHz',10,'  Memory: 32768K / 65536K',0

; ============ GUI STRINGS ============
g_ttl: db 'BlackMatrixOS v3.0 - Desktop',0
g_w1t: db ' System Information ',0
g_w1c: db '  BlackMatrixOS v3.0',10,'  Kernel: 3.0.0 i686',10,'  CPU: Virtual @ 100MHz',10,'  RAM: 64KB (32KB free)',10,'  Disk: 1.44MB Floppy',10,'  Display: VGA Text 80x25',10,'  Network: Loopback Only',10,'  Shell: /bin/sh',0
g_w2t: db ' Quick Help ',0
g_w2c: db '  F1  = Open this GUI',10,'  ESC = Return to shell',10,'  Type "help" for commands',10,'  300+ commands available',0
g_i1: db '/---\',10,'| F1 |',10,'|Help|',10,'\---/',0
g_i2: db '/---\',10,'| F2 |',10,'|Term|',10,'\---/',0
g_st: db '[Start] BlackMatrixOS',0
g_ck: db '12:00',0

; ============ COMMAND STRINGS ============
s_help: db 'BlackMatrixOS v3.0 - Available Commands:',10,10,10,'  System: ver uname hostname whoami uptime mem lscpu dmesg env',10,'  Disk:   ls dir cd pwd cat head tail tree df du mount',10,'  Text:   grep sort awk sed cut wc rev tr tee diff',10,'  Net:    ping ifconfig route netstat ssh curl wget nmap',10,'  Arch:   tar gzip zip unzip 7z cpio',10,'  Proc:   ps top kill nice jobs bg fg vmstat',10,'  Pkg:    apt yum pacman pip npm cargo gem',10,'  Sec:    chmod chown passwd sudo ssh openssl',10,'  Dev:    gcc make python node git java rustc go perl ruby lua',10,'  Fun:    cowsay neofetch fortune figlet cal banner matrix',10,'  Tools:  find xargs base64 md5sum sha256sum hexdump file stat',10,10,'  Special: echo calc seq hex dec factor expr color yes',10,'  GUI:    Press F1 for graphical mode',0
s_ban: db ' ____  _            _    _   _      _   _  _____ ____  ',10,'| __ )| | __ _  ___| | _| \ | |    | \ | |/ ____/ ___| ',10,'|  _ \| |/ _` |/ __| |/ /  \| |    |  \| | |  _ \___ \ ',10,'| |_) | | (_| | (__|   <| |\  | |__| |\  | |_| |___) |',10,'|____/|_|\__,_|\___|_|\_\_| \_|_____|_| \_|\____|____/ ',10,'                                                       ',10,'       Welcome to BlackMatrixOS v3.0 - 2026            ',0
s_cow: db '  _________________________',10,' < BlackMatrixOS rocks! >',10,'  -------------------------',10,'         \   ^__^',10,'          \  (oo)\_______',10,'             (__)\       )\/\',10,'                 ||----w |',10,'                 ||     ||',0
s_fort: db '"The best way to predict the future is to invent it." - Alan Kay',0
s_sl: db '      ====        ________                ___________',10,'  _D _|  |_______/        \__I_I=======_________|',10,'   |(_)---  |   H\________/ |   |  =|  ___  |',10,'   /     |  |   H  |  |     |   |   | |_|  _|',10,'  |      |  |   H  |__--------------------| [__] |',10,'  | ________|___H__/__|_____/[][]~\_______|',10,'  |/ |   |-----------I_____I [][] []  D',10,'                             |  ~~   |',0
s_fig: db '  ___  _   _    _  _____ ___  __  __   ____  ____  ___ ___ _   _ _____ ',10,' / _ \| \ | |  / \|_   _/ _ \|  \/  | / ___||  _ \|_ _/ _| \ | |_   _|',10,'| | | |  \| | / _ \ | || | | | |\/| | \___ \| |_) || | |_|  \| | | |  ',10,'| |_| | |\  |/ ___ \| || |_| | |  | |  ___) |  _ < | |  _| |\  | | |  ',10,' \___/|_| \_/_/   \_\_| \___/|_|  |_| |____/|_| \_\___|_| |_| \_| |_|  ',0
s_cal: db '     March 2026',10,'Su Mo Tu We Th Fr Sa',10,' 1  2  3  4  5  6  7',10,' 8  9 10 11 12 13 14',10,'15 16 17 18 19 20 21',10,'22 23 24 25 26 27 28',10,'29 30 31',0
s_mat: db 'Wake up, Neo...',10,'The Matrix has you...',10,'Follow the white rabbit.',10,10,'Knock, knock, Neo.',0

; ============ BSS SECTION ============
section .bss
cur: resd 1
clr: resb 1
inp: resb 256
il:  resd 1

