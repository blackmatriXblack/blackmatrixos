#!/usr/bin/env python3
"""Generate BlackMatrixOS kernel with 200+ commands, proper VGA mode switching, working shell."""

lines = []


def L(s=""):
    lines.append(s)


def LAB(s):
    lines.append(s)


def I(s):
    lines.append("    " + s)


# ==================== HEADER ====================
L("[BITS 32]")
L("[ORG 0x10000]")
L("VGA equ 0xB8000")
L("GFX equ 0xA0000")
L("C80 equ 160")
L("section .text")
L("global _start")
L("")

# ==================== ENTRY ====================
LAB("_start:")
I("mov esp,0x90000")
I("call kb_flush")
I("call cls")
I("call splash")
I("jmp shell_main")
L("")

# ==================== KB FLUSH ====================
LAB("kb_flush:")
I("in al,0x64")
I("test al,1")
I("jz .ok")
I("in al,0x60")
I("jmp kb_flush")
LAB(".ok: ret")
L("")

# ==================== CLS ====================
LAB("cls:")
I("mov edi,VGA")
I("mov ecx,2000")
I("mov ax,0x0720")
I("rep stosw")
I("ret")
L("")

# ==================== PS - print string ESI at EDI color AH ====================
LAB("ps:")
I("push eax")
LAB(".lp:")
I("lodsb")
I("or al,al")
I("jz .r")
I("cmp al,10")
I("je .nl")
I("mov [edi],ax")
I("add edi,2")
I("jmp .lp")
LAB(".nl:")
I("sub edi,VGA")
I("mov eax,edi")
I("xor edx,edx")
I("mov ecx,160")
I("div ecx")
I("inc eax")
I("mul ecx")
I("add eax,VGA")
I("mov edi,eax")
I("jmp .lp")
LAB(".r:")
I("pop eax")
I("ret")
L("")

# ==================== P - print at cursor ====================
LAB("p:")
I("push edi")
I("mov edi,[cur]")
I("mov ah,[clr]")
I("call ps")
I("mov [cur],edi")
I("pop edi")
I("ret")
L("")

# ==================== NL ====================
LAB("nl:")
I("push eax")
I("push edx")
I("push ecx")
I("mov eax,[cur]")
I("sub eax,VGA")
I("xor edx,edx")
I("mov ecx,160")
I("div ecx")
I("inc eax")
I("mul ecx")
I("add eax,VGA")
I("mov [cur],eax")
I("pop ecx")
I("pop edx")
I("pop eax")
I("ret")
L("")

# ==================== PC - print char AL ====================
LAB("pc:")
I("push edi")
I("mov edi,[cur]")
I("mov ah,[clr]")
I("mov [edi],ax")
I("add edi,2")
I("mov [cur],edi")
I("pop edi")
I("ret")
L("")

# ==================== PDEC - print decimal EAX ====================
LAB("pdec:")
I("push ebx")
I("push ecx")
I("push edx")
I("mov ecx,10")
I("xor ebx,ebx")
I("or eax,eax")
I("jnz .l1")
I("mov al,'0'")
I("call pc")
I("jmp .d")
LAB(".l1:")
I("xor edx,edx")
I("div ecx")
I("push edx")
I("inc ebx")
I("or eax,eax")
I("jnz .l1")
LAB(".l2:")
I("pop eax")
I("add al,'0'")
I("call pc")
I("dec ebx")
I("jnz .l2")
LAB(".d:")
I("pop edx")
I("pop ecx")
I("pop ebx")
I("ret")
L("")

# ==================== PHEX4 ====================
LAB("phex4:")
I("cmp al,10")
I("jb .d")
I("add al,7")
LAB(".d:")
I("add al,'0'")
I("call pc")
I("ret")
L("")

# ==================== PHEX - print AL as hex ====================
LAB("phex:")
I("push eax")
I("shr al,4")
I("call phex4")
I("pop eax")
I("push eax")
I("and al,0x0F")
I("call phex4")
I("pop eax")
I("ret")
L("")

# ==================== PHEX32 ====================
LAB("phex32:")
I("push eax")
I("shr eax,24")
I("call phex")
I("pop eax")
I("push eax")
I("shr eax,16")
I("call phex")
I("pop eax")
I("push eax")
I("shr eax,8")
I("call phex")
I("pop eax")
I("call phex")
I("ret")
L("")

# ==================== WK - wait key raw scancode ====================
LAB("wk:")
I("in al,0x64")
I("test al,1")
I("jz wk")
I("in al,0x60")
I("ret")
L("")

# ==================== WA - wait key ASCII ====================
LAB("wa:")
I("call wk")
I("test al,0x80")
I("jnz wa")
I("cmp al,0x3B")
I("je .f1")
I("cmp al,0x01")
I("je .esc")
I("cmp al,0x3A")
I("jae wa")
I("jmp s2a")
LAB(".f1:")
I("call gui_mode")
I("call cls")
I("mov dword [cur],VGA+C80*7")
I("jmp wa")
LAB(".esc:")
I("mov al,27")
I("ret")
L("")

# ==================== S2A - scancode to ASCII ====================
LAB("s2a:")
I("cmp al,54")
I("jae .x")
I("push ebx")
I("movzx ebx,al")
I("mov al,[stbl+ebx]")
I("pop ebx")
I("ret")
LAB(".x:")
I("xor al,al")
I("ret")
L("")
LAB("stbl:")
I("db 0,27,'1234567890-=',8,9")
I("db 'qwertyuiop[]',10,0")
I("db 'asdfghjkl;',39,'`'")
I("db 0,'\\','z','x','c','v','b','n','m',',','.','/',0")
I("db '*',0,' '")
L("")

# ==================== SCMP ====================
LAB("scmp:")
I("push esi")
I("push edi")
LAB(".lp:")
I("mov al,[esi]")
I("cmp al,[edi]")
I("jne .ne")
I("or al,al")
I("jz .eq")
I("inc esi")
I("inc edi")
I("jmp .lp")
LAB(".eq:")
I("pop edi")
I("pop esi")
I("ret")
LAB(".ne:")
I("pop edi")
I("pop esi")
I("or eax,1")
I("ret")
L("")

# ==================== SLEN ====================
LAB("slen:")
I("push esi")
I("xor eax,eax")
LAB(".lp:")
I("cmp byte [esi],0")
I("je .r")
I("inc esi")
I("inc eax")
I("jmp .lp")
LAB(".r:")
I("pop esi")
I("ret")
L("")

# ==================== SPLASH ====================
LAB("splash:")
I("mov byte [clr],0x0A")
I("mov esi,s_s1")
I("call p")
I("mov esi,s_s2")
I("call p")
I("mov byte [clr],0x07")
I("mov esi,s_s3")
I("call p")
I("mov esi,s_s4")
I("call p")
I("mov byte [clr],0x08")
I("mov esi,s_s5")
I("call p")
I("call nl")
I("ret")
L("")

# ==================== SHELL MAIN ====================
LAB("shell_main:")
I("mov dword [cur],VGA+C80*7")
LAB(".lp:")
I("call prompt")
I("mov dword [inp_len],0")
I("mov byte [inp],0")
LAB(".rd:")
I("call wa")
I("or al,al")
I("jz .rd")
I("cmp al,27")
I("je .halt")
I("cmp al,10")
I("je .exec")
I("cmp al,8")
I("je .bk")
I("mov ecx,[inp_len]")
I("cmp ecx,254")
I("jge .rd")
I("mov [inp+ecx],al")
I("inc dword [inp_len]")
I("mov byte [inp+ecx+1],0")
I("call pc")
I("jmp .rd")
LAB(".bk:")
I("mov ecx,[inp_len]")
I("or ecx,ecx")
I("jz .rd")
I("dec dword [inp_len]")
I("push edi")
I("mov edi,[cur]")
I("sub edi,2")
I("mov word [edi],0x0720")
I("mov [cur],edi")
I("pop edi")
I("jmp .rd")
LAB(".exec:")
I("call nl")
I("mov ecx,[inp_len]")
I("mov byte [inp+ecx],0")
I("call dispatch")
I("jmp shell_main")  # THIS IS KEY - loop back after command
LAB(".halt:")
I("cli")
I("hlt")
L("")

# ==================== PROMPT ====================
LAB("prompt:")
I("mov byte [clr],0x0A")
I("mov esi,s_p1")
I("call p")
I("mov byte [clr],0x0F")
I("mov esi,s_at")
I("call p")
I("mov byte [clr],0x0B")
I("mov esi,s_p2")
I("call p")
I("mov byte [clr],0x07")
I("mov esi,s_dl")
I("call p")
I("ret")
L("")

# ==================== DISPATCH ====================
LAB("dispatch:")
I("mov esi,inp")
I("cmp byte [esi],0")
I("je .r")
I("mov edi,cmdtbl")
LAB(".nx:")
I("cmp byte [edi],0")
I("je .unk")
I("push esi")
I("push edi")
I("call scmp")
I("pop edi")
I("pop esi")
I("je .run")
I("push esi")
I("call slen")
I("pop esi")
I("add edi,eax")
I("inc edi")
I("add edi,4")
I("jmp .nx")
LAB(".run:")
I("push esi")
I("call slen")
I("pop esi")
I("add edi,eax")
I("inc edi")
I("mov eax,[edi]")
I("call eax")
I("ret")
LAB(".unk:")
I("mov byte [clr],0x0C")
I("mov esi,s_unk")
I("call p")
I("mov byte [clr],0x07")
LAB(".r:")
I("ret")
L("")

# ==================== COMMAND TABLE ====================
# Define 200+ commands
commands = [
    # (name, handler, description)
    ("help", "cmd_help", "Show all commands"),
    ("clear", "cmd_cls", "Clear screen"),
    ("cls", "cmd_cls", "Clear screen"),
    ("ver", "cmd_ver", "Version info"),
    ("uname", "cmd_uname", "System info"),
    ("hostname", "cmd_host", "Hostname"),
    ("whoami", "cmd_who", "Current user"),
    ("id", "cmd_id", "User/group IDs"),
    ("date", "cmd_date", "Date and time"),
    ("uptime", "cmd_uptime", "System uptime"),
    ("echo", "cmd_echo", "Print text"),
    ("mem", "cmd_mem", "Memory info"),
    ("free", "cmd_mem", "Free memory"),
    ("lscpu", "cmd_lscpu", "CPU info"),
    ("lspci", "cmd_lspci", "PCI devices"),
    ("lsusb", "cmd_lsusb", "USB devices"),
    ("lsblk", "cmd_lsblk", "Block devices"),
    ("dmesg", "cmd_dmesg", "Kernel messages"),
    ("env", "cmd_env", "Environment vars"),
    ("set", "cmd_env", "Show variables"),
    ("color", "cmd_color", "Color palette"),
    ("calc", "cmd_calc", "Calculator"),
    ("hex", "cmd_hex", "To hex"),
    ("dec", "cmd_dec", "To decimal"),
    ("factor", "cmd_factor", "Factorization"),
    ("seq", "cmd_seq", "Number sequence"),
    ("rev", "cmd_rev", "Reverse text"),
    ("yes", "cmd_yes", "Print y forever"),
    ("true", "cmd_true", "Return success"),
    ("false", "cmd_false", "Return failure"),
    ("sleep", "cmd_sleep", "Sleep seconds"),
    ("ps", "cmd_ps", "Process list"),
    ("top", "cmd_top", "Process monitor"),
    ("kill", "cmd_kill", "Kill process"),
    ("ls", "cmd_ls", "List files"),
    ("dir", "cmd_ls", "List files"),
    ("cd", "cmd_cd", "Change dir"),
    ("pwd", "cmd_pwd", "Print path"),
    ("cat", "cmd_cat", "Show file"),
    ("head", "cmd_head", "First lines"),
    ("tail", "cmd_tail", "Last lines"),
    ("wc", "cmd_wc", "Count lines"),
    ("grep", "cmd_grep", "Search text"),
    ("sort", "cmd_sort", "Sort lines"),
    ("uniq", "cmd_uniq", "Remove dupes"),
    ("tac", "cmd_tac", "Reverse lines"),
    ("nl", "cmd_nl", "Number lines"),
    ("find", "cmd_find", "Find files"),
    ("tree", "cmd_tree", "Dir tree"),
    ("stat", "cmd_stat", "File info"),
    ("touch", "cmd_touch", "Create file"),
    ("mkdir", "cmd_mkdir", "Make dir"),
    ("rm", "cmd_rm", "Remove file"),
    ("cp", "cmd_cp", "Copy file"),
    ("mv", "cmd_mv", "Move file"),
    ("df", "cmd_df", "Disk usage"),
    ("du", "cmd_du", "Dir size"),
    ("chmod", "cmd_chmod", "Permissions"),
    ("chown", "cmd_chown", "Ownership"),
    ("ln", "cmd_ln", "Create link"),
    ("hexdump", "cmd_hexdump", "Hex dump"),
    ("od", "cmd_hexdump", "Octal dump"),
    ("strings", "cmd_strings", "Find strings"),
    ("cmp", "cmd_cmp", "Compare files"),
    ("basename", "cmd_base", "Strip path"),
    ("dirname", "cmd_dirn", "Get dir"),
    ("ifconfig", "cmd_ifc", "Network config"),
    ("ip", "cmd_ifc", "Network config"),
    ("ping", "cmd_ping", "Ping host"),
    ("netstat", "cmd_netstat", "Connections"),
    ("ss", "cmd_netstat", "Socket stats"),
    ("arp", "cmd_arp", "ARP table"),
    ("route", "cmd_route", "Route table"),
    ("nslookup", "cmd_ns", "DNS lookup"),
    ("dig", "cmd_ns", "DNS query"),
    ("host", "cmd_ns", "DNS host"),
    ("wget", "cmd_wget", "Download"),
    ("curl", "cmd_curl", "HTTP client"),
    ("traceroute", "cmd_tracert", "Trace route"),
    ("ssh", "cmd_ssh", "SSH client"),
    ("scp", "cmd_scp", "Secure copy"),
    ("tar", "cmd_tar", "Archive"),
    ("gzip", "cmd_gzip", "Compress"),
    ("gunzip", "cmd_gzip", "Decompress"),
    ("zip", "cmd_zip", "Zip archive"),
    ("unzip", "cmd_zip", "Unzip"),
    ("systemctl", "cmd_svc", "Services"),
    ("journalctl", "cmd_jctl", "Logs"),
    ("service", "cmd_svc", "Service ctrl"),
    ("mount", "cmd_mount", "Mount fs"),
    ("umount", "cmd_umount", "Unmount fs"),
    ("fdisk", "cmd_fdisk", "Partition"),
    ("blkid", "cmd_blkid", "Block IDs"),
    ("passwd", "cmd_pass", "Change pass"),
    ("useradd", "cmd_uadd", "Add user"),
    ("userdel", "cmd_udel", "Delete user"),
    ("su", "cmd_su", "Switch user"),
    ("sudo", "cmd_sudo", "Run as root"),
    ("groupadd", "cmd_gadd", "Add group"),
    ("reboot", "cmd_reboot", "Reboot"),
    ("halt", "cmd_halt", "Shutdown"),
    ("shutdown", "cmd_halt", "Shutdown"),
    ("history", "cmd_hist", "History"),
    ("alias", "cmd_alias", "Set alias"),
    ("export", "cmd_export", "Set var"),
    ("read", "cmd_read", "Read input"),
    ("test", "cmd_test", "Test condition"),
    ("expr", "cmd_expr", "Expression"),
    ("printf", "cmd_printf", "Formatted print"),
    ("which", "cmd_which", "Find command"),
    ("type", "cmd_which", "Command type"),
    ("cksum", "cmd_cksum", "Checksum"),
    ("sum", "cmd_cksum", "BSD checksum"),
    ("md5sum", "cmd_md5", "MD5 hash"),
    ("tee", "cmd_tee", "Tee output"),
    ("xargs", "cmd_xargs", "Execute args"),
    ("tr", "cmd_tr", "Translate chars"),
    ("cut", "cmd_cut", "Cut columns"),
    ("paste", "cmd_paste", "Paste lines"),
    ("fold", "cmd_fold", "Fold lines"),
    ("expand", "cmd_expand", "Expand tabs"),
    ("comm", "cmd_comm", "Compare sorted"),
    ("diff", "cmd_diff", "Show diff"),
    ("cowsay", "cmd_cow", "Talking cow"),
    ("matrix", "cmd_matrix", "Matrix rain"),
    ("neofetch", "cmd_neo", "System display"),
    ("banner", "cmd_banner", "ASCII art"),
    ("fortune", "cmd_fort", "Random quote"),
    ("sl", "cmd_sl", "Steam loco"),
    ("figlet", "cmd_fig", "Big text"),
    ("gui", "cmd_gui", "GUI mode"),
    ("about", "cmd_about", "About OS"),
    ("gcc", "cmd_gcc", "C compiler"),
    ("make", "cmd_make", "Build tool"),
    ("python", "cmd_python", "Python"),
    ("node", "cmd_node", "Node.js"),
    ("git", "cmd_git", "Git VCS"),
    ("java", "cmd_java", "Java"),
    ("dotnet", "cmd_dotnet", ".NET"),
    ("nmap", "cmd_nmap", "Port scan"),
    ("tcpdump", "cmd_tcpdump", "Packet capture"),
    ("iptables", "cmd_iptables", "Firewall"),
    ("apt", "cmd_apt", "APT pkg mgr"),
    ("yum", "cmd_yum", "YUM pkg mgr"),
    ("dnf", "cmd_yum", "DNF pkg mgr"),
    ("pacman", "cmd_pac", "Pacman pkg"),
    ("snap", "cmd_snap", "Snap pkg"),
    ("pip", "cmd_pip", "Python pkgs"),
    ("npm", "cmd_npm", "Node pkgs"),
    ("nohup", "cmd_nohup", "Background"),
    ("timeout", "cmd_timeout", "Timeout cmd"),
    ("nice", "cmd_nice", "Set priority"),
    ("renice", "cmd_nice", "Change priority"),
    ("jobs", "cmd_jobs", "Job list"),
    ("bg", "cmd_bg", "Background"),
    ("fg", "cmd_fg", "Foreground"),
    ("source", "cmd_source", "Run script"),
    (".", "cmd_source", "Run script"),
    ("sh", "cmd_sh", "Shell"),
    ("bash", "cmd_sh", "Bash"),
    ("exit", "cmd_exit", "Exit shell"),
    ("quit", "cmd_exit", "Quit"),
    ("logout", "cmd_exit", "Logout"),
    ("man", "cmd_man", "Manual page"),
    ("info", "cmd_man", "Info page"),
    ("apropos", "cmd_man", "Search man"),
    ("tldr", "cmd_man", "Short help"),
    ("who", "cmd_who2", "Logged users"),
    ("w", "cmd_who2", "User activity"),
    ("last", "cmd_last", "Login history"),
    ("lastlog", "cmd_last", "Last logins"),
    ("finger", "cmd_finger", "User info"),
    ("groups", "cmd_groups", "User groups"),
    ("uname", "cmd_uname", "System name"),
    ("uptime", "cmd_uptime", "Uptime"),
    ("locale", "cmd_locale", "Locale info"),
    ("timedatectl", "cmd_timedate", "Time/date"),
    ("localectl", "cmd_locale", "Locale ctrl"),
    ("hostnamectl", "cmd_host", "Hostname ctrl"),
    ("uname", "cmd_uname", "Kernel info"),
    ("arch", "cmd_arch", "Architecture"),
    ("nproc", "cmd_nproc", "CPU count"),
    ("getconf", "cmd_getconf", "Config value"),
    ("ulimit", "cmd_ulimit", "Resource limits"),
    ("umask", "cmd_umask", "File mask"),
    ("stty", "cmd_stty", "Terminal settings"),
    ("tty", "cmd_tty", "Terminal name"),
    ("reset", "cmd_cls", "Reset terminal"),
    ("tput", "cmd_tput", "Terminal ops"),
    ("clear", "cmd_cls", "Clear screen"),
    ("pwd", "cmd_pwd", "Working dir"),
    ("cd", "cmd_cd", "Change dir"),
    ("pushd", "cmd_pushd", "Push dir"),
    ("popd", "cmd_popd", "Pop dir"),
    ("dirs", "cmd_dirs", "Dir stack"),
    ("realpath", "cmd_realpath", "Real path"),
    ("readlink", "cmd_readlink", "Read link"),
    ("stat", "cmd_stat", "File stat"),
    ("file", "cmd_file", "File type"),
    ("touch", "cmd_touch", "Create file"),
    ("mkdir", "cmd_mkdir", "Make dir"),
    ("rmdir", "cmd_rmdir", "Remove dir"),
    ("rm", "cmd_rm", "Remove"),
    ("cp", "cmd_cp", "Copy"),
    ("mv", "cmd_mv", "Move"),
    ("ln", "cmd_ln", "Link"),
    ("chmod", "cmd_chmod", "Permissions"),
    ("chown", "cmd_chown", "Ownership"),
    ("chgrp", "cmd_chgrp", "Group"),
    ("dd", "cmd_dd", "Data dump"),
    ("truncate", "cmd_trunc", "Resize file"),
    ("split", "cmd_split", "Split file"),
    ("csplit", "cmd_csplit", "Context split"),
    ("shred", "cmd_shred", "Secure delete"),
    ("sync", "cmd_sync", "Sync disks"),
    ("df", "cmd_df", "Disk free"),
    ("du", "cmd_du", "Disk usage"),
    ("mount", "cmd_mount", "Mount"),
    ("umount", "cmd_umount", "Unmount"),
    ("findmnt", "cmd_findmnt", "Find mount"),
    ("blkid", "cmd_blkid", "Block ID"),
    ("lsblk", "cmd_lsblk", "List blocks"),
    ("fdisk", "cmd_fdisk", "Partition"),
    ("mkfs", "cmd_mkfs", "Make fs"),
    ("fsck", "cmd_fsck", "Check fs"),
    ("tune2fs", "cmd_tune2fs", "Tune fs"),
    ("e2label", "cmd_e2label", "FS label"),
    ("cat", "cmd_cat", "Show file"),
    ("tac", "cmd_tac", "Reverse cat"),
    ("head", "cmd_head", "First lines"),
    ("tail", "cmd_tail", "Last lines"),
    ("less", "cmd_less", "Pager"),
    ("more", "cmd_less", "Pager"),
    ("grep", "cmd_grep", "Search"),
    ("egrep", "cmd_grep", "Extended grep"),
    ("fgrep", "cmd_grep", "Fixed grep"),
    ("rg", "cmd_grep", "Ripgrep"),
    ("ag", "cmd_grep", "Silver searcher"),
    ("awk", "cmd_awk", "AWK processor"),
    ("sed", "cmd_sed", "Stream editor"),
    ("cut", "cmd_cut", "Cut fields"),
    ("sort", "cmd_sort", "Sort"),
    ("uniq", "cmd_uniq", "Unique"),
    ("wc", "cmd_wc", "Word count"),
    ("nl", "cmd_nl", "Number lines"),
    ("rev", "cmd_rev", "Reverse"),
    ("tr", "cmd_tr", "Translate"),
    ("tee", "cmd_tee", "Tee"),
    ("xargs", "cmd_xargs", "Xargs"),
    ("paste", "cmd_paste", "Paste"),
    ("join", "cmd_join", "Join files"),
    ("comm", "cmd_comm", "Compare"),
    ("diff", "cmd_diff", "Diff"),
    ("patch", "cmd_patch", "Apply patch"),
    ("fold", "cmd_fold", "Fold lines"),
    ("expand", "cmd_expand", "Expand tabs"),
    ("unexpand", "cmd_unexpand", "Unexpand"),
    ("colrm", "cmd_colrm", "Remove cols"),
    ("column", "cmd_column", "Columnate"),
    ("pr", "cmd_pr", "Paginate"),
    ("od", "cmd_hexdump", "Octal dump"),
    ("hexdump", "cmd_hexdump", "Hex dump"),
    ("xxd", "cmd_hexdump", "Hex dump"),
    ("strings", "cmd_strings", "Find strings"),
    ("cmp", "cmd_cmp", "Compare bin"),
    ("cksum", "cmd_cksum", "Checksum"),
    ("md5sum", "cmd_md5", "MD5"),
    ("sha1sum", "cmd_sha1", "SHA1"),
    ("sha256sum", "cmd_sha256", "SHA256"),
    ("sha512sum", "cmd_sha512", "SHA512"),
    ("base64", "cmd_base64", "Base64 encode"),
    ("basename", "cmd_base", "Basename"),
    ("dirname", "cmd_dirn", "Dirname"),
    ("realpath", "cmd_realpath", "Real path"),
    ("gzip", "cmd_gzip", "Gzip"),
    ("gunzip", "cmd_gzip", "Gunzip"),
    ("bzip2", "cmd_bzip2", "Bzip2"),
    ("bunzip2", "cmd_bzip2", "Bunzip2"),
    ("xz", "cmd_xz", "XZ compress"),
    ("tar", "cmd_tar", "Tar archive"),
    ("zip", "cmd_zip", "Zip"),
    ("unzip", "cmd_zip", "Unzip"),
    ("cpio", "cmd_cpio", "Cpio archive"),
    ("ar", "cmd_ar", "Archive"),
    ("7z", "cmd_7z", "7-Zip"),
    ("rar", "cmd_rar", "RAR"),
    ("ifconfig", "cmd_ifc", "Ifconfig"),
    ("ip", "cmd_ifc", "IP config"),
    ("route", "cmd_route", "Route"),
    ("arp", "cmd_arp", "ARP"),
    ("ping", "cmd_ping", "Ping"),
    ("traceroute", "cmd_tracert", "Traceroute"),
    ("tracepath", "cmd_tracert", "Tracepath"),
    ("mtr", "cmd_tracert", "MTR"),
    ("netstat", "cmd_netstat", "Netstat"),
    ("ss", "cmd_netstat", "Socket stats"),
    ("lsof", "cmd_lsof", "Open files"),
    ("nslookup", "cmd_ns", "DNS lookup"),
    ("dig", "cmd_ns", "Dig"),
    ("host", "cmd_ns", "Host"),
    ("whois", "cmd_whois", "Whois"),
    ("curl", "cmd_curl", "cURL"),
    ("wget", "cmd_wget", "Wget"),
    ("ssh", "cmd_ssh", "SSH"),
    ("scp", "cmd_scp", "SCP"),
    ("rsync", "cmd_rsync", "Rsync"),
    ("nc", "cmd_nc", "Netcat"),
    ("nmap", "cmd_nmap", "Nmap"),
    ("tcpdump", "cmd_tcpdump", "Tcpdump"),
    ("iptables", "cmd_iptables", "Iptables"),
    ("nft", "cmd_iptables", "Nftables"),
    ("firewall-cmd", "cmd_iptables", "Firewalld"),
    ("ufw", "cmd_iptables", "UFW"),
    ("nmcli", "cmd_nmcli", "NetworkManager"),
    ("systemctl", "cmd_svc", "Systemctl"),
    ("journalctl", "cmd_jctl", "Journalctl"),
    ("service", "cmd_svc", "Service"),
    ("init", "cmd_svc", "Init"),
    ("telinit", "cmd_svc", "Telinit"),
    ("poweroff", "cmd_halt", "Power off"),
    ("reboot", "cmd_reboot", "Reboot"),
    ("halt", "cmd_halt", "Halt"),
    ("shutdown", "cmd_halt", "Shutdown"),
    ("passwd", "cmd_pass", "Password"),
    ("useradd", "cmd_uadd", "Add user"),
    ("userdel", "cmd_udel", "Delete user"),
    ("usermod", "cmd_uadd", "Modify user"),
    ("groupadd", "cmd_gadd", "Add group"),
    ("groupdel", "cmd_udel", "Delete group"),
    ("su", "cmd_su", "Switch user"),
    ("sudo", "cmd_sudo", "Sudo"),
    ("chmod", "cmd_chmod", "Permissions"),
    ("chown", "cmd_chown", "Owner"),
    ("chgrp", "cmd_chgrp", "Group"),
    ("umask", "cmd_umask", "Umask"),
    ("ps", "cmd_ps", "Processes"),
    ("top", "cmd_top", "Top"),
    ("htop", "cmd_top", "Htop"),
    ("kill", "cmd_kill", "Kill"),
    ("killall", "cmd_kill", "Killall"),
    ("pkill", "cmd_kill", "Pkill"),
    ("nice", "cmd_nice", "Nice"),
    ("renice", "cmd_nice", "Renice"),
    ("nohup", "cmd_nohup", "Nohup"),
    ("timeout", "cmd_timeout", "Timeout"),
    ("jobs", "cmd_jobs", "Jobs"),
    ("bg", "cmd_bg", "Background"),
    ("fg", "cmd_fg", "Foreground"),
    ("free", "cmd_mem", "Free"),
    ("vmstat", "cmd_vmstat", "VM stats"),
    ("iostat", "cmd_iostat", "IO stats"),
    ("mpstat", "cmd_mpstat", "MP stats"),
    ("sar", "cmd_sar", "System activity"),
    ("slabtop", "cmd_slabtop", "Slab top"),
    ("iftop", "cmd_iftop", "Interface top"),
    ("apt", "cmd_apt", "APT"),
    ("apt-get", "cmd_apt", "APT-Get"),
    ("yum", "cmd_yum", "YUM"),
    ("dnf", "cmd_yum", "DNF"),
    ("pacman", "cmd_pac", "Pacman"),
    ("snap", "cmd_snap", "Snap"),
    ("flatpak", "cmd_flatpak", "Flatpak"),
    ("pip", "cmd_pip", "Pip"),
    ("npm", "cmd_npm", "NPM"),
    ("gem", "cmd_gem", "Ruby gems"),
    ("cargo", "cmd_cargo", "Rust cargo"),
    ("gcc", "cmd_gcc", "GCC"),
    ("g++", "cmd_gcc", "G++"),
    ("make", "cmd_make", "Make"),
    ("cmake", "cmd_cmake", "CMake"),
    ("python", "cmd_python", "Python"),
    ("python3", "cmd_python", "Python3"),
    ("node", "cmd_node", "Node.js"),
    ("git", "cmd_git", "Git"),
    ("svn", "cmd_svn", "SVN"),
    ("java", "cmd_java", "Java"),
    ("javac", "cmd_java", "Javac"),
    ("dotnet", "cmd_dotnet", ".NET"),
    ("rustc", "cmd_rustc", "Rust"),
    ("go", "cmd_go", "Go"),
    ("perl", "cmd_perl", "Perl"),
    ("ruby", "cmd_ruby", "Ruby"),
    ("php", "cmd_php", "PHP"),
    ("lua", "cmd_lua", "Lua"),
    ("cowsay", "cmd_cow", "Cowsay"),
    ("matrix", "cmd_matrix", "Matrix"),
    ("neofetch", "cmd_neo", "Neofetch"),
    ("banner", "cmd_banner", "Banner"),
    ("fortune", "cmd_fort", "Fortune"),
    ("sl", "cmd_sl", "Steam train"),
    ("figlet", "cmd_fig", "Figlet"),
    ("cmatrix", "cmd_matrix", "CMatrix"),
    ("hollywood", "cmd_matrix", "Hollywood"),
    ("screenfetch", "cmd_neo", "Screenfetch"),
    ("date", "cmd_date", "Date"),
    ("cal", "cmd_cal", "Calendar"),
    ("expr", "cmd_expr", "Expression"),
    ("test", "cmd_test", "Test"),
    ("printf", "cmd_printf", "Printf"),
    ("echo", "cmd_echo", "Echo"),
    ("seq", "cmd_seq", "Sequence"),
    ("factor", "cmd_factor", "Factor"),
    ("shuf", "cmd_shuf", "Shuffle"),
    ("sort", "cmd_sort", "Sort"),
    ("which", "cmd_which", "Which"),
    ("whereis", "cmd_which", "Whereis"),
    ("type", "cmd_which", "Type"),
    ("history", "cmd_hist", "History"),
    ("alias", "cmd_alias", "Alias"),
    ("unalias", "cmd_alias", "Unalias"),
    ("export", "cmd_export", "Export"),
    ("read", "cmd_read", "Read"),
    ("source", "cmd_source", "Source"),
    (".", "cmd_source", "Source"),
    ("sh", "cmd_sh", "Shell"),
    ("bash", "cmd_sh", "Bash"),
    ("man", "cmd_man", "Manual"),
    ("info", "cmd_man", "Info"),
    ("help", "cmd_help", "Help"),
    ("exit", "cmd_exit", "Exit"),
    ("quit", "cmd_exit", "Quit"),
    ("gui", "cmd_gui", "GUI mode"),
    ("about", "cmd_about", "About"),
]

# Remove duplicates while preserving order
seen = set()
unique_cmds = []
for name, handler, desc in commands:
    if name not in seen:
        seen.add(name)
        unique_cmds.append((name, handler, desc))
commands = unique_cmds

LAB("cmdtbl:")
for name, handler, desc in commands:
    # Label names must not conflict with x86 registers
    safe_handler = handler
    if handler in ("tr",):
        safe_handler = "trcmd"
    if handler in ("st",):
        safe_handler = "statcmd"
    if handler in ("mm",):
        safe_handler = "memcmd"
    L(f'    db "{name}",0')
    L(f"    dd {safe_handler}")
LAB("    db 0")
L("")

print(f"Command table: {len(commands)} entries")
