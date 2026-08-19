; ==================== COMMAND HANDLERS ====================
cmd_help:
    mov byte [clr],0x07
    mov esi,h_t
    call p
    mov byte [clr],0x0B
    mov esi,h1
    call p
    mov byte [clr],0x07
    mov esi,h2
    call p
    mov esi,h3
    call p
    mov esi,h4
    call p
    mov esi,h5
    call p
    mov esi,h6
    call p
    mov esi,h7
    call p
    mov esi,h8
    call p
    mov esi,h9
    call p
    mov esi,h10
    call p
    mov esi,h11
    call p
    mov esi,h12
    call p
    ret
cmd_cls:
    call cls
    mov dword [cur],VGA+C80*3
    ret
cmd_ver:
    mov byte [clr],0x0B
    mov esi,v1
    call p
    mov byte [clr],0x07
    mov esi,v2
    call p
    ret
cmd_uname:
    mov byte [clr],0x0B
    mov esi,un1
    call p
    mov byte [clr],0x07
    ret
cmd_host:
    mov byte [clr],0x0A
    mov esi,hn1
    call p
    mov byte [clr],0x07
    ret
cmd_who:
    mov byte [clr],0x0A
    mov esi,wa1
    call p
    mov byte [clr],0x07
    ret
cmd_id:
    mov byte [clr],0x07
    mov esi,id1
    call p
    ret
cmd_date:
    mov byte [clr],0x0B
    mov esi,dt1
    call p
    mov byte [clr],0x07
    ret
cmd_uptime:
    mov byte [clr],0x07
    mov esi,up1
    call p
    ret
cmd_echo:
    mov esi,inp
.lp:lodsb
    or al,al
    jz .d
    cmp al,' '
    je .pr
    jmp .lp
.pr:mov byte [clr],0x0E
    call p
    mov byte [clr],0x07
    call nl
    ret
.d: call nl
    ret
cmd_mem:
    mov byte [clr],0x07
    mov esi,mm1
    call p
    ret
cmd_lscpu:
    mov byte [clr],0x07
    mov esi,cpu1
    call p
    ret
cmd_lspci:
    mov byte [clr],0x07
    mov esi,pci1
    call p
    ret
cmd_lsusb:
    mov byte [clr],0x07
    mov esi,usb1
    call p
    ret
cmd_lsblk:
    mov byte [clr],0x07
    mov esi,blk1
    call p
    ret
cmd_dmesg:
    mov byte [clr],0x07
    mov esi,dm1
    call p
    ret
cmd_env:
    mov byte [clr],0x07
    mov ev1,0
    call p
    ret
cmd_color:
    mov edi,[cur]
    xor ecx,ecx
.cl:mov al,'#'
    mov ah,cl
    mov [edi],ax
    add edi,2
    inc ecx
    cmp ecx,16
    jl .cl
    mov [cur],edi
    call nl
    ret
cmd_calc:
    mov byte [clr],0x0A
    mov esi,ca1
    call p
    mov byte [clr],0x07
    ret
cmd_hex:
    mov byte [clr],0x0A
    mov esi,hx1
    call p
    mov byte [clr],0x07
    ret
cmd_dec:
    mov byte [clr],0x0A
    mov esi,dc1
    call p
    mov byte [clr],0x07
    ret
cmd_factor:
    mov byte [clr],0x0B
    mov esi,fa1
    call p
    mov byte [clr],0x07
    ret
cmd_seq:
    mov byte [clr],0x07
    xor eax,eax
    mov ecx,10
.sl:inc eax
    push eax ecx
    call pdec
    call nl
    pop ecx eax
    dec ecx
    jnz .sl
    ret
cmd_rev:
    mov byte [clr],0x0B
    mov rv1,0
    call p
    mov byte [clr],0x07
    ret
cmd_yes:
    mov byte [clr],0x0E
.yl:mov esi,y1
    call p
    in al,0x64
    test al,1
    jz .yl
    in al,0x60
    test al,0x80
    jz .ex
    jmp .yl
.ex:in al,0x60
    mov byte [clr],0x07
    ret
cmd_true:
    ret
cmd_false:
    mov byte [clr],0x0C
    mov esi,fl1
    call p
    mov byte [clr],0x07
    ret
cmd_sleep:
    mov byte [clr],0x07
    mov esi,sl1
    call p
    mov ecx,0x00500000
.sl:dec ecx
    jnz .sl
    ret
cmd_ps:
    mov byte [clr],0x07
    mov esi,ps1
    call p
    ret
cmd_top:
    mov byte [clr],0x07
    mov esi,tp1
    call p
    ret
cmd_kill:
    mov byte [clr],0x0C
    mov esi,kl1
    call p
    mov byte [clr],0x07
    ret
cmd_ls:
    mov byte [clr],0x0B
    mov esi,ls1
    call p
    mov byte [clr],0x07
    ret
cmd_cd:
    mov byte [clr],0x0A
    mov esi,cd1
    call p
    mov byte [clr],0x07
    ret
cmd_pwd:
    mov byte [clr],0x0A
    mov esi,pw1
    call p
    mov byte [clr],0x07
    ret
cmd_cat:
    mov byte [clr],0x0B
    mov esi,ct1
    call p
    mov byte [clr],0x07
    ret
cmd_head:
    mov byte [clr],0x07
    mov esi,hd1
    call p
    ret
cmd_tail:
    mov byte [clr],0x07
    mov esi,tl1
    call p
    ret
cmd_wc:
    mov byte [clr],0x07
    mov esi,wc1
    call p
    ret
cmd_grep:
    mov byte [clr],0x07
    mov esi,gr1
    call p
    ret
cmd_sort:
    mov byte [clr],0x07
    mov esi,so1
    call p
    ret
cmd_uniq:
    mov byte [clr],0x07
    mov esi,uq1
    call p
    ret
cmd_tac:
    mov byte [clr],0x07
    mov esi,tc1
    call p
    ret
cmd_nl:
    mov byte [clr],0x07
    mov esi,nl1
    call p
    ret
cmd_find:
    mov byte [clr],0x07
    mov esi,fn1
    call p
    ret
cmd_tree:
    mov byte [clr],0x0B
    mov esi,tr1
    call p
    mov byte [clr],0x07
    ret
cmd_stat:
    mov byte [clr],0x07
    mov esi,st1
    call p
    ret
cmd_touch:
    mov byte [clr],0x0A
    mov esi,tt1
    call p
    mov byte [clr],0x07
    ret
cmd_mkdir:
    mov byte [clr],0x0A
    mov esi,mk1
    call p
    mov byte [clr],0x07
    ret
cmd_rm:
    mov byte [clr],0x0C
    mov esi,rm1
    call p
    mov byte [clr],0x07
    ret
cmd_cp:
    mov byte [clr],0x0A
    mov esi,cp1
    call p
    mov byte [clr],0x07
    ret
cmd_mv:
    mov byte [clr],0x0A
    mov esi,mv1
    call p
    mov byte [clr],0x07
    ret
cmd_df:
    mov byte [clr],0x07
    mov esi,df1
    call p
    ret
cmd_du:
    mov byte [clr],0x07
    mov esi,du1
    call p
    ret
cmd_chmod:
    mov byte [clr],0x0A
    mov esi,ch1
    call p
    mov byte [clr],0x07
    ret
cmd_chown:
    mov byte [clr],0x0A
    mov esi,co1
    call p
    mov byte [clr],0x07
    ret
cmd_ln:
    mov byte [clr],0x0A
    mov esi,ln1
    call p
    mov byte [clr],0x07
    ret
cmd_hexdump:
    mov byte [clr],0x07
    mov esi,hd2
    call p
    ret
cmd_strings:
    mov byte [clr],0x07
    mov esi,str1
    call p
    ret
cmd_cmp:
    mov byte [clr],0x07
    mov esi,cmp1
    call p
    ret
cmd_base:
    mov byte [clr],0x0A
    mov esi,bs1
    call p
    mov byte [clr],0x07
    ret
cmd_dirn:
    mov byte [clr],0x0A
    mov esi,dn1
    call p
    mov byte [clr],0x07
    ret
cmd_ifc:
    mov byte [clr],0x07
    mov esi,if1
    call p
    ret
cmd_ping:
    mov byte [clr],0x07
    mov esi,pn1
    call p
    ret
cmd_netstat:
    mov byte [clr],0x07
    mov esi,ns1
    call p
    ret
cmd_arp:
    mov byte [clr],0x07
    mov esi,ar1
    call p
    ret
cmd_route:
    mov byte [clr],0x07
    mov esi,rt1
    call p
    ret
cmd_ns:
    mov byte [clr],0x07
    mov esi,nk1
    call p
    ret
cmd_wget:
    mov byte [clr],0x0B
    mov esi,wg1
    call p
    mov byte [clr],0x07
    ret
cmd_curl:
    mov byte [clr],0x0B
    mov esi,cu1
    call p
    mov byte [clr],0x07
    ret
cmd_tracert:
    mov byte [clr],0x07
    mov esi,ttr1
    call p
    ret
cmd_ssh:
    mov byte [clr],0x0B
    mov esi,sh1
    call p
    mov byte [clr],0x07
    ret
cmd_scp:
    mov byte [clr],0x0B
    mov esi,sc1
    call p
    mov byte [clr],0x07
    ret
cmd_tar:
    mov byte [clr],0x07
    mov esi,tar1
    call p
    ret
cmd_gzip:
    mov byte [clr],0x07
    mov esi,gz1
    call p
    ret
cmd_zip:
    mov byte [clr],0x07
    mov esi,zp1
    call p
    ret
cmd_svc:
    mov byte [clr],0x07
    mov esi,sv1
    call p
    ret
cmd_jctl:
    mov byte [clr],0x07
    mov esi,jc1
    call p
    ret
cmd_mount:
    mov byte [clr],0x07
    mov esi,mt1
    call p
    ret
cmd_umount:
    mov byte [clr],0x0A
    mov esi,um1
    call p
    mov byte [clr],0x07
    ret
cmd_fdisk:
    mov byte [clr],0x07
    mov esi,fd1
    call p
    ret
cmd_blkid:
    mov byte [clr],0x07
    mov esi,bk1
    call p
    ret
cmd_pass:
    mov byte [clr],0x0A
    mov esi,pa1
    call p
    mov byte [clr],0x07
    ret
cmd_uadd:
    mov byte [clr],0x0A
    mov esi,ua1
    call p
    mov byte [clr],0x07
    ret
cmd_udel:
    mov byte [clr],0x0C
    mov esi,ud1
    call p
    mov byte [clr],0x07
    ret
cmd_su:
    mov byte [clr],0x0A
    mov esi,su1
    call p
    mov byte [clr],0x07
    ret
cmd_sudo:
    mov byte [clr],0x0E
    mov esi,sd1
    call p
    mov byte [clr],0x07
    ret
cmd_gadd:
    mov byte [clr],0x0A
    mov esi,ga1
    call p
    mov byte [clr],0x07
    ret
cmd_reboot:
    mov byte [clr],0x0E
    mov esi,rb1
    call p
    mov al,0xFE
    out 0x64,al
    hlt
cmd_halt:
    mov byte [clr],0x0E
    mov esi,ht1
    call p
    cli
    hlt
cmd_hist:
    mov byte [clr],0x07
    mov esi,hs1
    call p
    ret
cmd_alias:
    mov byte [clr],0x0A
    mov esi,al1
    call p
    mov byte [clr],0x07
    ret
cmd_export:
    mov byte [clr],0x0A
    mov esi,ep1
    call p
    mov byte [clr],0x07
    ret
cmd_read:
    mov byte [clr],0x0A
    mov esi,rd1
    call p
    mov byte [clr],0x07
    ret
cmd_test:
    mov byte [clr],0x0A
    mov esi,ts1
    call p
    mov byte [clr],0x07
    ret
cmd_expr:
    mov byte [clr],0x0A
    mov esi,ex1
    call p
    mov byte [clr],0x07
    ret
cmd_printf:
    mov byte [clr],0x0E
    mov esi,pf1
    call p
    mov byte [clr],0x07
    call nl
    ret
cmd_which:
    mov byte [clr],0x0A
    mov esi,wh1
    call p
    mov byte [clr],0x07
    ret
cmd_cksum:
    mov byte [clr],0x07
    mov esi,ck1
    call p
    ret
cmd_md5:
    mov byte [clr],0x07
    mov esi,md1
    call p
    ret
cmd_tee:
    mov byte [clr],0x07
    mov esi,te1
    call p
    ret
cmd_xargs:
    mov byte [clr],0x07
    mov esi,xa1
    call p
    ret
cmd_tr:
    mov byte [clr],0x07
    mov esi,tr2
    call p
    ret
cmd_cut:
    mov byte [clr],0x07
    mov esi,cu2
    call p
    ret
cmd_paste:
    mov byte [clr],0x07
    mov esi,ps2
    call p
    ret
cmd_fold:
    mov byte [clr],0x07
    mov esi,fo1
    call p
    ret
cmd_expand:
    mov byte [clr],0x07
    mov esi,ex2
    call p
    ret
cmd_comm:
    mov byte [clr],0x07
    mov esi,cm1
    call p
    ret
cmd_diff:
    mov byte [clr],0x07
    mov esi,df2
    call p
    ret
cmd_cow:
    mov byte [clr],0x0E
    mov esi,cw1
    call p
    mov byte [clr],0x07
    ret
cmd_matrix:
    call cmd_matrix_run
    ret
cmd_matrix_run:
    call cls
    mov edi,mcols
    mov ecx,80
    xor eax,eax
.mi:mov [edi],eax
    neg dword [edi]
    add eax,3
    add edi,4
    dec ecx
    jnz .mi
.ml:in al,0x64
    test al,1
    jz .mr
    in al,0x60
    cmp al,0x01
    je .mx
.mr:mov esi,mcols
    mov ecx,80
    xor ebx,ebx
.mc:mov eax,[esi]
    or eax,eax
    js .mn
    cmp eax,25
    jge .mr2
    push eax ebx
    imul eax,160
    lea edi,[VGA+eax]
    mov eax,ebx
    shl eax,1
    add edi,eax
    mov eax,ebx
    add eax,[esi]
    and eax,0x3F
    add al,0x21
    mov ah,0x0F
    mov [edi],ax
    sub edi,160
    cmp edi,VGA
    jb .mp
    mov ah,0x0A
    mov [edi],ax
.mp:pop ebx eax
.mn:inc dword [esi]
    add esi,4
    inc ebx
    dec ecx
    jnz .mc
    mov ecx,500000
.md:dec ecx
    jnz .md
    jmp .ml
.mr2:mov dword [esi],-20
    jmp .mn
.mx:call cls
    ret
cmd_neo:
    mov byte [clr],0x0A
    mov esi,ne1
    call p
    mov byte [clr],0x07
    mov esi,ne2
    call p
    ret
cmd_banner:
    mov byte [clr],0x0A
    mov esi,bn1
    call p
    mov byte [clr],0x07
    ret
cmd_fort:
    mov byte [clr],0x0E
    mov esi,ft1
    call p
    mov byte [clr],0x07
    ret
cmd_sl:
    mov byte [clr],0x0A
    mov esi,sl2
    call p
    mov byte [clr],0x07
    ret
cmd_fig:
    mov byte [clr],0x0A
    mov esi,fg1
    call p
    mov byte [clr],0x07
    ret
cmd_gui:
    call gui_mode
    ret
cmd_about:
    mov byte [clr],0x0B
    mov esi,ab1
    call p
    mov byte [clr],0x07
    ret
cmd_gcc:
    mov byte [clr],0x0A
    mov esi,gc1
    call p
    mov byte [clr],0x07
    ret
cmd_make:
    mov byte [clr],0x0A
    mov esi,mk2
    call p
    mov byte [clr],0x07
    ret
cmd_python:
    mov byte [clr],0x0E
    mov esi,py1
    call p
    mov byte [clr],0x07
    ret
cmd_node:
    mov byte [clr],0x0A
    mov esi,nd1
    call p
    mov byte [clr],0x07
    ret
cmd_git:
    mov byte [clr],0x0B
    mov esi,gt1
    call p
    mov byte [clr],0x07
    ret
cmd_java:
    mov byte [clr],0x0A
    mov esi,jv1
    call p
    mov byte [clr],0x07
    ret
cmd_dotnet:
    mov byte [clr],0x0A
    mov esi,dn2
    call p
    mov byte [clr],0x07
    ret
cmd_nmap:
    mov byte [clr],0x07
    mov esi,np1
    call p
    ret
cmd_tcpdump:
    mov byte [clr],0x07
    mov esi,td1
    call p
    ret
cmd_iptables:
    mov byte [clr],0x07
    mov esi,ipt1
    call p
    ret
cmd_apt:
    mov byte [clr],0x0A
    mov esi,ap1
    call p
    mov byte [clr],0x07
    ret
cmd_yum:
    mov byte [clr],0x0A
    mov esi,ym1
    call p
    mov byte [clr],0x07
    ret
cmd_pac:
    mov byte [clr],0x0A
    mov esi,pc1
    call p
    mov byte [clr],0x07
    ret
cmd_snap:
    mov byte [clr],0x0A
    mov esi,sn1
    call p
    mov byte [clr],0x07
    ret
cmd_pip:
    mov byte [clr],0x0A
    mov esi,pp1
    call p
    mov byte [clr],0x07
    ret
cmd_npm:
    mov byte [clr],0x0A
    mov esi,nm1
    call p
    mov byte [clr],0x07
    ret
cmd_nohup:
    mov byte [clr],0x0A
    mov esi,nh1
    call p
    mov byte [clr],0x07
    ret
cmd_timeout:
    mov byte [clr],0x07
    mov esi,to1
    call p
    ret
cmd_nice:
    mov byte [clr],0x0A
    mov esi,nc1
    call p
    mov byte [clr],0x07
    ret
cmd_jobs:
    mov byte [clr],0x07
    mov esi,jb1
    call p
    ret
cmd_bg:
    mov byte [clr],0x0A
    mov esi,bg1
    call p
    mov byte [clr],0x07
    ret
cmd_fg:
    mov byte [clr],0x0A
    mov esi,fg2
    call p
    mov byte [clr],0x07
    ret
cmd_export:
    mov byte [clr],0x0A
    mov esi,ep1
    call p
    mov byte [clr],0x07
    ret
cmd_source:
    mov byte [clr],0x0A
    mov esi,so2
    call p
    mov byte [clr],0x07
    ret
cmd_sh:
    mov byte [clr],0x0B
    mov esi,sh2
    call p
    mov byte [clr],0x07
    ret
cmd_exit:
    mov byte [clr],0x0E
    mov esi,ex3
    call p
    cli
    hlt
cmd_man:
    mov byte [clr],0x07
    mov esi,mn1
    call p
    ret
