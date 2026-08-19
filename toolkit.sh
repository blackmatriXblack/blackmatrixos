#!/bin/bash
# BlackMatrix Toolkit - Linux/WSL Version
# 150+ System Tools

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
GRAY='\033[0;37m'
RED='\033[0;31m'
CYAN='\033[0;36m'
NC='\033[0m'

show_menu() {
    clear
    echo -e "${GREEN}  ============================================================${NC}"
    echo -e "${GREEN}    BlackMatrix Toolkit v2.0 - Linux Edition${NC}"
    echo -e "${GREEN}    150+ System Tools${NC}"
    echo -e "${GREEN}  ============================================================${NC}"
    echo ""
    echo -e "${YELLOW}   SYSTEM INFO${NC}"
    echo -e "${GRAY}    1=uname -a  2=hostname  3=whoami  4=id  5=uptime${NC}"
    echo -e "${GRAY}    6=lsb_release -a  7=cat /etc/os-release  8=lscpu  9=lsmem${NC}"
    echo -e "${GRAY}   10=lsblk  11=lsusb  12=lspci  13=dmidecode  14=hdparm${NC}"
    echo -e "${GRAY}   15=dmesg  16=journalctl  17=cat /proc/cpuinfo  18=cat /proc/meminfo${NC}"
    echo -e "${GRAY}   19=cat /proc/version  20=timedatectl  21=localectl${NC}"
    echo ""
    echo -e "${YELLOW}   PROCESS${NC}"
    echo -e "${GRAY}   25=ps aux  26=ps -ef  27=top  28=htop  29=pgrep${NC}"
    echo -e "${GRAY}   30=pkill  31=killall  32=nice  33=renice  34=pstree${NC}"
    echo -e "${GRAY}   35=vmstat  36=iostat  37=mpstat  38=sar  39=pidstat${NC}"
    echo ""
    echo -e "${YELLOW}   MEMORY${NC}"
    echo -e "${GRAY}   40=free -h  41=cat /proc/meminfo  42=vmstat -s  43=slabtop${NC}"
    echo ""
    echo -e "${YELLOW}   DISK / FILE${NC}"
    echo -e "${GRAY}   45=df -h  46=du -sh  47=ls -la  48=tree  49=find${NC}"
    echo -e "${GRAY}   50=locate  51=which  52=stat  53=file  54=md5sum/sha256sum${NC}"
    echo -e "${GRAY}   55=chmod  56=chown  57=ln  58=mount  59=umount${NC}"
    echo -e "${GRAY}   60=fdisk  61=parted  62=blkid  63=lsblk  64=fstab${NC}"
    echo -e "${GRAY}   65=dd  66=mkfs  67=fsck  68=tune2fs${NC}"
    echo ""
    echo -e "${YELLOW}   TEXT PROCESSING${NC}"
    echo -e "${GRAY}   70=cat  71=tac  72=head  73=tail  74=less/more${NC}"
    echo -e "${GRAY}   75=grep  76=egrep  77=awk  78=sed  79=cut${NC}"
    echo -e "${GRAY}   80=sort  81=uniq  82=wc  83=tr  84=tee${NC}"
    echo -e "${GRAY}   85=diff  86=patch  87=comm  88=paste  89=join${NC}"
    echo -e "${GRAY}   90=rev  91=expand  92=unexpand  93=fold  94=nl${NC}"
    echo -e "${GRAY}   95=od  96=xxd  97=hexdump  98=strings  99=xargs${NC}"
    echo ""
    echo -e "${YELLOW}   NETWORK${NC}"
    echo -e "${GRAY}  100=ip addr  101=ip route  102=ss -tulnp  103=netstat -tulnp${NC}"
    echo -e "${GRAY}  104=ping  105=traceroute  106=nslookup  107=dig  108=host${NC}"
    echo -e "${GRAY}  109=curl  110=wget  111=ssh  112=scp  113=rsync${NC}"
    echo -e "${GRAY}  114=ifconfig  115=ethtool  116=arp  117=nmap  118=nc${NC}"
    echo -e "${GRAY}  119=tcpdump  120=iptables  121=firewall-cmd  122=nmcli${NC}"
    echo ""
    echo -e "${YELLOW}   ARCHIVE${NC}"
    echo -e "${GRAY}  125=tar  126=gzip/gunzip  127=bzip2  128=xz  129=zip/unzip${NC}"
    echo -e "${GRAY}  130=7z  131=cpio  132=ar  133=zcat${NC}"
    echo ""
    echo -e "${YELLOW}   PACKAGE${NC}"
    echo -e "${GRAY}  135=apt  136=yum/dnf  137=pacman  138=snap  139=flatpak${NC}"
    echo ""
    echo -e "${YELLOW}   SERVICE / SYSTEMD${NC}"
    echo -e "${GRAY}  140=systemctl  141=journalctl  142=loginctl  143=timedatectl${NC}"
    echo ""
    echo -e "${YELLOW}   SECURITY${NC}"
    echo -e "${GRAY}  145=passwd  146=useradd  147=userdel  148=groupadd  149=chmod${NC}"
    echo -e "${GRAY}  150=chown  151=setfacl  152=getfacl  153=last  154=lastlog${NC}"
    echo -e "${GRAY}  155=fail2ban  156=chkrootkit  157=rkhunter${NC}"
    echo ""
    echo -e "${YELLOW}   QUICK ACTIONS${NC}"
    echo -e "${GRAY}  200=Custom cmd  201=Flush DNS  202=Open ports  203=Disk usage${NC}"
    echo -e "${GRAY}  204=Top processes  205=System health  206=Speed test${NC}"
    echo -e "${GRAY}  207=Clear cache  208=Update system  209=Watch logs${NC}"
    echo ""
    echo -e "${RED}    0=Exit${NC}"
    echo -e "${GREEN}  ============================================================${NC}"
}

run_cmd() {
    echo ""
    eval "$1" 2>&1
    echo ""
    read -p "Press Enter to continue..."
}

while true; do
    show_menu
    read -p "  Enter number: " c

    case $c in
        1) run_cmd "uname -a" ;;
        2) run_cmd "hostname" ;;
        3) run_cmd "whoami; id" ;;
        4) run_cmd "id" ;;
        5) run_cmd "uptime" ;;
        6) run_cmd "lsb_release -a 2>/dev/null || cat /etc/redhat-release 2>/dev/null || cat /etc/os-release" ;;
        7) run_cmd "cat /etc/os-release" ;;
        8) run_cmd "lscpu" ;;
        9) run_cmd "lsmem 2>/dev/null || cat /proc/meminfo | head -5" ;;
        10) run_cmd "lsblk" ;;
        11) run_cmd "lsusb" ;;
        12) run_cmd "lspci" ;;
        13) run_cmd "sudo dmidecode 2>/dev/null || echo 'Need root'" ;;
        14) run_cmd "sudo hdparm -i /dev/sda 2>/dev/null || echo 'Need root'" ;;
        15) run_cmd "dmesg | tail -30" ;;
        16) run_cmd "journalctl -n 20 --no-pager" ;;
        17) run_cmd "cat /proc/cpuinfo | head -30" ;;
        18) run_cmd "cat /proc/meminfo | head -20" ;;
        19) run_cmd "cat /proc/version" ;;
        20) run_cmd "timedatectl" ;;
        21) run_cmd "localectl" ;;
        25) run_cmd "ps aux" ;;
        26) run_cmd "ps -ef" ;;
        27) run_cmd "top -bn1 | head -30" ;;
        28) run_cmd "htop 2>/dev/null || top -bn1 | head -30" ;;
        29) p=read -p "Process name: "; run_cmd "pgrep -l $p" ;;
        30) p=read -p "Process name: "; run_cmd "pkill $p" ;;
        31) p=read -p "Process name: "; run_cmd "killall $p" ;;
        32) run_cmd "nice" ;;
        33) run_cmd "renice" ;;
        34) run_cmd "pstree -p" ;;
        35) run_cmd "vmstat 1 5" ;;
        36) run_cmd "iostat 2>/dev/null || echo 'Install sysstat'" ;;
        37) run_cmd "mpstat 2>/dev/null || echo 'Install sysstat'" ;;
        38) run_cmd "sar 2>/dev/null || echo 'Install sysstat'" ;;
        39) run_cmd "pidstat 2>/dev/null || echo 'Install sysstat'" ;;
        40) run_cmd "free -h" ;;
        41) run_cmd "cat /proc/meminfo" ;;
        42) run_cmd "vmstat -s" ;;
        43) run_cmd "slabtop -o | head -20" ;;
        45) run_cmd "df -h" ;;
        46) run_cmd "du -sh /* 2>/dev/null | sort -rh | head -20" ;;
        47) run_cmd "ls -la" ;;
        48) p=read -p "Path [.]: "; run_cmd "tree ${p:-.} | head -50" ;;
        49) p=read -p "Name pattern: "; run_cmd "find . -name '$p' 2>/dev/null | head -20" ;;
        50) p=read -p "Search: "; run_cmd "locate $p 2>/dev/null | head -20 || find / -name '$p' 2>/dev/null | head -20" ;;
        51) p=read -p "Command: "; run_cmd "which $p" ;;
        52) p=read -p "File: "; run_cmd "stat $p" ;;
        53) p=read -p "File: "; run_cmd "file $p" ;;
        54) p=read -p "File: "; run_cmd "md5sum $p; sha256sum $p" ;;
        55) p=read -p "Mode File: "; run_cmd "chmod $p" ;;
        56) p=read -p "Owner File: "; run_cmd "chown $p" ;;
        57) run_cmd "ln --help | head -5" ;;
        58) run_cmd "mount | head -20" ;;
        59) run_cmd "umount --help | head -3" ;;
        60) run_cmd "sudo fdisk -l 2>/dev/null || lsblk" ;;
        61) run_cmd "sudo parted -l 2>/dev/null || echo 'Need root'" ;;
        62) run_cmd "blkid" ;;
        63) run_cmd "lsblk -f" ;;
        64) run_cmd "cat /etc/fstab" ;;
        65) run_cmd "dd --help | head -3" ;;
        66) run_cmd "mkfs --help | head -3" ;;
        67) run_cmd "fsck --help | head -3" ;;
        68) run_cmd "tune2fs --help | head -3" ;;
        70) p=read -p "File: "; run_cmd "cat $p" ;;
        71) p=read -p "File: "; run_cmd "tac $p" ;;
        72) p=read -p "File: "; run_cmd "head -20 $p" ;;
        73) p=read -p "File: "; run_cmd "tail -20 $p" ;;
        74) p=read -p "File: "; run_cmd "less $p" ;;
        75) p=read -p "Pattern File: "; run_cmd "grep $p" ;;
        76) p=read -p "Pattern File: "; run_cmd "egrep $p" ;;
        77) p=read -p "Program File: "; run_cmd "awk $p" ;;
        78) p=read -p "Expression File: "; run_cmd "sed $p" ;;
        79) p=read -p "Fields File: "; run_cmd "cut $p" ;;
        80) p=read -p "File: "; run_cmd "sort $p" ;;
        81) p=read -p "File: "; run_cmd "uniq -c $p" ;;
        82) p=read -p "File: "; run_cmd "wc $p" ;;
        83) p=read -p "Sets: "; run_cmd "echo 'test input' | tr $p" ;;
        84) run_cmd "echo 'hello world' | tee /tmp/tee_test.txt; cat /tmp/tee_test.txt" ;;
        85) p=read -p "File1 File2: "; run_cmd "diff $p" ;;
        86) run_cmd "patch --help | head -3" ;;
        87) p=read -p "File1 File2: "; run_cmd "comm $p" ;;
        88) p=read -p "File1 File2: "; run_cmd "paste $p" ;;
        89) p=read -p "File1 File2: "; run_cmd "join $p" ;;
        90) p=read -p "File: "; run_cmd "rev $p" ;;
        91) p=read -p "File: "; run_cmd "expand $p | head -20" ;;
        92) p=read -p "File: "; run_cmd "unexpand $p | head -20" ;;
        93) p=read -p "File: "; run_cmd "fold -w 60 $p | head -20" ;;
        94) p=read -p "File: "; run_cmd "nl $p | head -20" ;;
        95) p=read -p "File: "; run_cmd "od -x $p | head -20" ;;
        96) p=read -p "File: "; run_cmd "xxd $p | head -20" ;;
        97) p=read -p "File: "; run_cmd "hexdump -C $p | head -20" ;;
        98) p=read -p "File: "; run_cmd "strings $p | head -20" ;;
        99) run_cmd "echo '1 2 3' | xargs echo" ;;
        100) run_cmd "ip addr" ;;
        101) run_cmd "ip route" ;;
        102) run_cmd "ss -tulnp" ;;
        103) run_cmd "netstat -tulnp 2>/dev/null || ss -tulnp" ;;
        104) p=read -p "Host: "; run_cmd "ping -c 4 $p" ;;
        105) p=read -p "Host: "; run_cmd "traceroute $p 2>/dev/null || tracepath $p" ;;
        106) p=read -p "Host: "; run_cmd "nslookup $p" ;;
        107) p=read -p "Host: "; run_cmd "dig $p" ;;
        108) p=read -p "Host: "; run_cmd "host $p" ;;
        109) p=read -p "URL: "; run_cmd "curl -v $p" ;;
        110) p=read -p "URL: "; run_cmd "wget --spider $p" ;;
        111) p=read -p "user@host: "; ssh $p ;;
        112) p=read -p "scp args: "; run_cmd "scp $p" ;;
        113) p=read -p "rsync args: "; run_cmd "rsync $p" ;;
        114) run_cmd "ifconfig 2>/dev/null || ip addr" ;;
        115) p=read -p "Interface: "; run_cmd "ethtool $p 2>/dev/null || echo 'Need ethtool'" ;;
        116) run_cmd "arp -a 2>/dev/null || ip neigh" ;;
        117) p=read -p "Target: "; run_cmd "nmap $p 2>/dev/null || echo 'Install nmap'" ;;
        118) run_cmd "nc -h 2>&1 | head -5" ;;
        119) run_cmd "sudo tcpdump -c 5 2>/dev/null || echo 'Need root + tcpdump'" ;;
        120) run_cmd "sudo iptables -L -n 2>/dev/null || echo 'Need root'" ;;
        121) run_cmd "firewall-cmd --list-all 2>/dev/null || echo 'firewalld not running'" ;;
        122) run_cmd "nmcli device status" ;;
        125) p=read -p "tar args: "; run_cmd "tar $p" ;;
        126) p=read -p "File: "; run_cmd "gzip -l $p 2>/dev/null || gunzip -l $p" ;;
        127) run_cmd "bzip2 --help | head -3" ;;
        128) run_cmd "xz --help | head -3" ;;
        129) run_cmd "zip --help | head -3" ;;
        130) run_cmd "7z --help 2>/dev/null | head -5 || echo 'Install p7zip'" ;;
        131) run_cmd "cpio --help | head -3" ;;
        132) run_cmd "ar --help | head -3" ;;
        133) run_cmd "zcat --help | head -3" ;;
        135) run_cmd "apt list --installed 2>/dev/null | head -20 || yum list installed 2>/dev/null | head -20" ;;
        136) run_cmd "dnf list installed 2>/dev/null | head -20 || yum list installed 2>/dev/null | head -20" ;;
        137) run_cmd "pacman -Q 2>/dev/null | head -20 || echo 'Not Arch-based'" ;;
        138) run_cmd "snap list 2>/dev/null || echo 'snap not installed'" ;;
        139) run_cmd "flatpak list 2>/dev/null || echo 'flatpak not installed'" ;;
        140) run_cmd "systemctl list-units --type=service --state=running | head -20" ;;
        141) run_cmd "journalctl -n 20 --no-pager" ;;
        142) run_cmd "loginctl list-sessions 2>/dev/null" ;;
        143) run_cmd "timedatectl" ;;
        145) p=read -p "Username: "; run_cmd "sudo passwd $p" ;;
        146) p=read -p "Username: "; run_cmd "sudo useradd $p" ;;
        147) p=read -p "Username: "; run_cmd "sudo userdel $p" ;;
        148) p=read -p "Groupname: "; run_cmd "sudo groupadd $p" ;;
        149) run_cmd "chmod --help | head -3" ;;
        150) run_cmd "chown --help | head -3" ;;
        151) run_cmd "setfacl --help | head -3" ;;
        152) p=read -p "File: "; run_cmd "getfacl $p" ;;
        153) run_cmd "last | head -20" ;;
        154) run_cmd "lastlog | head -20" ;;
        155) run_cmd "fail2ban-client status 2>/dev/null || echo 'fail2ban not running'" ;;
        156) run_cmd "chkrootkit 2>/dev/null || echo 'Install chkrootkit'" ;;
        157) run_cmd "rkhunter --check 2>/dev/null || echo 'Install rkhunter'" ;;
        200) p=read -p "Command: "; run_cmd "$p" ;;
        201) run_cmd "sudo systemd-resolve --flush-caches 2>/dev/null || sudo resolvectl flush-caches 2>/dev/null || echo 'Flushed'" ;;
        202) run_cmd "ss -tulnp" ;;
        203) run_cmd "df -h && echo '---' && du -sh /* 2>/dev/null | sort -rh | head -10" ;;
        204) run_cmd "ps aux --sort=-%mem | head -15" ;;
        205)
            echo -e "${GREEN}  === SYSTEM HEALTH ===${NC}"
            echo "  Kernel: $(uname -r)"
            echo "  Uptime: $(uptime -p)"
            echo "  CPU: $(lscpu | grep 'Model name' | cut -d: -f2 | xargs)"
            echo "  Load: $(cat /proc/loadavg)"
            echo "  RAM: $(free -h | awk '/Mem:/ {print $3 "/" $2}')"
            echo "  Disk: $(df -h / | awk 'NR==2 {print $3 "/" $2}')"
            echo "  Users: $(who | wc -l) logged in"
            read -p "Press Enter..."
            ;;
        206) run_cmd "curl -o /dev/null -w 'Speed: %{speed_download} bytes/sec\n' https://speed.cloudflare.com/__down?bytes=10000000" ;;
        207) run_cmd "sudo sync; sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches' 2>/dev/null && echo 'Cache cleared' || echo 'Need root'" ;;
        208) run_cmd "sudo apt update && sudo apt upgrade -y 2>/dev/null || sudo yum update -y 2>/dev/null || sudo dnf update -y 2>/dev/null" ;;
        209) p=read -p "Log file: "; run_cmd "tail -f ${p:-/var/log/syslog}" ;;
        0) echo "Bye!"; exit 0 ;;
        *) echo "Invalid option"; sleep 1 ;;
    esac
done
