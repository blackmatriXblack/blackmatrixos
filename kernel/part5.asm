; ==================== DATA ====================
s1: db '========================================',10
    db '  BlackMatrixOS v2.0 - Command Line OS',10
    db '========================================',10,0
s2: db '  120+ built-in tools | gui or F1 for GUI',10,0
s3: db '  Type "help" for all commands',10,0
s4: db 0
s5: db 0
s6: db 0
s7: db 0
p1: db 'root',0
p2: db '@',0
p3: db 'bmos',0
p4: db '$ ',0
u1: db 'Unknown command. Type "help".',10,0
h_t:db 10,'BlackMatrixOS v2.0 - 120+ Commands:',10,0
h1: db 10,'  --- SYSTEM INFO ---',10,0
h2: db '  help ver uname hostname whoami id date uptime lscpu lspci',10
    db '  lsusb lsblk dmesg mem free env set color neofetch about',10,0
h3: db 10,'  --- DISK / FILE ---',10
    db '  ls dir cd pwd cat head tail mkdir rm touch cp mv',10
    db '  find tree stat df du chmod chown ln hexdump od strings',10
    db '  cmp basename dirname grep sort uniq wc tac nl rev tee',10,0
h4: db 10,'  --- PROCESS ---',10
    db '  ps top kill nice jobs bg fg nohup timeout renice',10,0
h5: db 10,'  --- NETWORK ---',10
    db '  ifconfig ip ping traceroute netstat ss arp route',10
    db '  nslookup dig host wget curl ssh scp nmap tcpdump',10,0
h6: db 10,'  --- ARCHIVE ---',10
    db '  tar gzip gunzip zip unzip',10,0
h7: db 10,'  --- SYSTEM MGMT ---',10
    db '  systemctl journalctl service mount umount fdisk blkid',10
    db '  reboot halt shutdown',10,0
h8: db 10,'  --- SECURITY ---',10
    db '  passwd useradd userdel su sudo chmod chown groupadd iptables',10,0
h9: db 10,'  --- PACKAGE ---',10
    db '  apt yum dnf pacman snap pip npm',10,0
h10:db 10,'  --- DEVELOPER ---',10
    db '  gcc make python node git java dotnet',10,0
h11:db 10,'  --- TEXT ---',10
    db '  tr cut paste fold expand comm diff cksum md5sum xargs',10
    db '  printf expr test read export alias source sh bash man',10,0
h12:db 10,'  --- FUN ---',10
    db '  cowsay matrix banner fortune sl figlet figlet seq hex dec factor',10,0
v1: db 'BlackMatrixOS v2.0.0',10,0
v2: db 'Build: 2026-03-29 | x86 i686 | Monolithic',10,0
un1:db 'BlackMatrixOS bmos 2.0.0 x86 i686 GNU',10,0
hn1:db 'blackmatrix-os',10,0
wa1:db 'root',10,0
id1:db 'uid=0(root) gid=0(root) groups=0(root)',10,0
dt1:db '2026-03-29 00:00:00 UTC',10,0
up1:db ' 00:00:00 up 0 min, 1 user, load: 0.00 0.00 0.00',10,0
mm1:db '              total       used       free     shared    buffers',10
    db 'Mem:      131072 kB    8192 kB  122880 kB      0 kB    2048 kB',10
    db 'Swap:          0 kB        0 kB       0 kB',10,0
cpu1:db 'CPU: QEMU Virtual CPU @ 2.40GHz',10
    db 'Cores: 1 | Threads: 1',10
    db 'Arch: x86 (i686) 32-bit',10
    db 'Cache: 256 KB L2',10
    db 'Flags: fpu vme pse tsc msr pae cx8 apic sep',10,0
pci1:db '00:00.0 Host bridge: Intel 440FX',10
    db '00:01.0 ISA bridge: Intel PIIX3',10
    db '00:01.1 IDE: Intel PIIX3 IDE',10
    db '00:02.0 VGA: Bochs/QEMU VGA',10
    db '00:03.0 Ethernet: Realtek RTL-8029',10
    db '00:04.0 Audio: Sound Blaster 16',10,0
usb1:db 'Bus 001 Device 001: ID 1d6b:0002 USB 2.0 Root Hub',10
    db 'Bus 001 Device 002: ID 0627:0001 QEMU USB Tablet',10,0
blk1:db 'NAME  SIZE  TYPE    MOUNTPOINT',10
    db 'sda   128M  disk    /',10
    db 'sda1  128M  part    /',10,0
dm1:db '[    0.000] BlackMatrixOS Kernel starting...',10
    db '[    0.001] VGA: Text mode 80x25 initialized',10
    db '[    0.002] GDT: Global Descriptor Table loaded',10
    db '[    0.003] IDT: Interrupt Descriptor Table loaded',10
    db '[    0.004] PIC: 8259 remapped',10
    db '[    0.005] PIT: Timer at 100 Hz',10
    db '[    0.006] KB:  Keyboard driver initialized',10
    db '[    0.007] PMM: 128 MB physical memory ready',10
    db '[    0.008] FS:  RAM filesystem mounted',10
    db '[    0.009] NET: Network stack initialized',10
    db '[    0.010] SHELL: 120+ commands loaded',10,0
ev1:db 'HOME=/root',10,'PATH=/bin:/sbin:/usr/bin',10
    db 'SHELL=/bin/bash',10,'USER=root',10
    db 'TERM=linux',10,'LANG=en_US.UTF-8',10
    db 'HOSTNAME=blackmatrix-os',10,'PWD=/root',10,0
ca1:db 'calc <expr> - Calculator (e.g. calc 2+3)',10,0
hx1:db 'Usage: hex <decimal>',10,0
dc1:db 'Usage: dec <hex>',10,0
fa1:db 'Usage: factor <number>',10,0
rv1:db 'Usage: rev <file>',10,0
y1: db 'y',10,0
fl1:db 'false',10,0
sl1:db 'Sleeping 1 second...',10,0
ps1:db '  PID USER     CPU%  MEM%  COMMAND',10
    db '    1 root      0.0   0.5  kernel',10
    db '    2 root      0.1   0.3  shell',10
    db '    3 root      0.0   0.2  timer',10
    db '    4 root      0.0   0.1  keyboard',10
    db '    5 root      0.0   0.1  netd',10,0
tp1:db 'PID  USER  CPU% MEM% COMMAND',10
    db '  1  root  0.0  0.5  kernel',10
    db '  2  root  0.1  0.3  shell',10
    db '  3  root  0.0  0.2  timer',10
    db '  4  root  0.0  0.1  keyboard',10
    db 'Press ESC to exit...',10,0
kl1:db 'kill: process terminated',10,0
ls1:db 'bin/   etc/   home/  proc/  sys/   tmp/   var/   usr/',10
    db 'dev/   lib/   mnt/   opt/   root/  sbin/  srv/   run/',10,0
cd1:db 'Changed directory',10,0
pw1:db '/root',10,0
ct1:db 'cat: display file contents (simulated)',10,0
hd1:db 'head: show first lines',10,0
tl1:db 'tail: show last lines',10,0
wc1:db 'wc: 0 lines, 0 words, 0 chars',10,0
gr1:db 'grep: search pattern in file',10,0
so1:db 'sort: sort lines',10,0
uq1:db 'uniq: remove duplicates',10,0
tc1:db 'tac: reverse line order',10,0
nl1:db 'nl: number lines',10,0
fn1:db 'find: search files',10,0
tr1:db '.',10,'|-- bin/',10,'|-- etc/',10,'|   `-- hostname',10
    db '|-- home/',10,'|   `-- user/',10,'|-- proc/',10,'|-- root/',10
    db '|-- tmp/',10,'|-- usr/',10,'`-- var/',10,0
st1:db '  File: /etc/hostname',10,'  Size: 15 bytes',10
    db '  Type: regular file',10,'  Perms: 0644',10,0
tt1:db 'touch: file created',10,0
mk1:db 'mkdir: directory created',10,0
rm1:db 'rm: file removed',10,0
cp1:db 'cp: file copied',10,0
mv1:db 'mv: file moved',10,0
df1:db 'Filesystem  Size  Used  Avail Use%  Mounted on',10
    db '/dev/sda1   128M   32M    96M  25%  /',10
    db 'tmpfs        64M    0M    64M   0%  /tmp',10,0
du1:db '8K  ./etc',10,'4K  ./home',10,'4K  ./root',10
    db '0K  ./tmp',10,'16K .',10,0
ch1:db 'chmod: permissions changed',10,0
co1:db 'chown: ownership changed',10,0
ln1:db 'ln: link created',10,0
hd2:db '00000000: 7F45 4C46 0101 0100 0000 0000 0000 0000  .ELF............',10
    db '00000010: 0200 0300 0100 0000 0080 0408 3400 0000  ............4...',10
    db '00000020: 0000 0000 0000 0000 3400 2000 0200 2800  ........4. ...(.',10,0
str1:db 'strings: no printable strings found',10,0
cmp1:db 'cmp: files are identical',10,0
bs1:db '/etc/hostname',10,0
dn1:db '/etc',10,0
if1:db 'eth0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>',10
    db '  inet 10.0.2.15  netmask 255.255.255.0  broadcast 10.0.2.255',10
    db '  ether 52:54:00:12:34:56  txqueuelen 1000',10
    db '  RX packets 0  bytes 0',10
    db '  TX packets 0  bytes 0',10
    db 'lo: flags=73<UP,LOOPBACK,RUNNING>',10
    db '  inet 127.0.0.1  netmask 255.0.0.0',10,0
pn1:db 'PING 8.8.8.8: 64 bytes',10
    db '64 bytes from 8.8.8.8: icmp_seq=1 ttl=64 time=1 ms',10
    db '64 bytes from 8.8.8.8: icmp_seq=2 ttl=64 time=1 ms',10
    db '64 bytes from 8.8.8.8: icmp_seq=3 ttl=64 time=1 ms',10
    db '--- 8.8.8.8 ping statistics ---',10
    db '3 packets transmitted, 3 received, 0% packet loss',10,0
ns1:db 'Proto  LocalAddr   ForeignAddr  State',10
    db 'tcp    0.0.0.0:22  0.0.0.0:*    LISTEN',10
    db 'tcp    0.0.0.0:80  0.0.0.0:*    LISTEN',10
    db 'tcp    127.0.0.1:53  0.0.0.0:*  LISTEN',10,0
ar1:db 'Address         HWtype  HWaddress',10
    db '10.0.2.2        ether   52:55:0a:00:02:02',10,0
rt1:db 'Destination     Gateway     Genmask       Iface',10
    db '0.0.0.0         10.0.2.2    0.0.0.0       eth0',10
    db '10.0.2.0        0.0.0.0     255.255.255.0 eth0',10,0
nk1:db 'Server: 10.0.2.3',10,'Address: 10.0.2.3#53',10
    db 'Name: example.com',10,'Address: 93.184.216.34',10,0
wg1:db 'wget: downloading file...',10
    db 'saving to: index.html',10
    db 'download complete (simulated)',10,0
cu1:db 'curl: HTTP/1.1 200 OK',10
    db 'Content-Type: text/html',10
    db 'Content-Length: 1256',10
    db 'Connection: keep-alive',10,0
ttr1:db 'traceroute to host (simulated):',10
    db ' 1  10.0.2.2   1 ms',10
    db ' 2  192.168.1.1  5 ms',10
    db ' 3  * * *',10,0
sh1:db 'ssh: connecting to host (simulated)...',10
    db 'Welcome to BlackMatrixOS SSH',10,0
sc1:db 'scp: secure copy (simulated)',10,0
tar1:db 'tar: archive operations',10
    db '  tar cvf archive.tar files  - Create',10
    db '  tar xvf archive.tar       - Extract',10
    db '  tar tvf archive.tar       - List',10,0
gz1:db 'gzip: file compressed (simulated)',10,0
zp1:db 'zip: archive created (simulated)',10,0
sv1:db 'UNIT                LOAD   ACTIVE   SUB',10
    db 'sshd.service        loaded active running',10
    db 'networkd.service    loaded active running',10
    db 'cron.service        loaded active running',10,0
jc1:db '-- Logs begin at 2026-03-29 --',10
    db 'Mar 29 00:00:00 bmos kernel: BlackMatrixOS started',10
    db 'Mar 29 00:00:01 bmos sshd: Server listening on port 22',10,0
mt1:db 'Filesystem  Type   Size  Used  Avail  Mounted on',10
    db '/dev/sda1   ext4   128M   32M    96M   /',10
    db 'tmpfs       tmpfs   64M    0M    64M   /tmp',10
    db 'proc        proc     0     0     0    /proc',10
    db 'sysfs       sysfs    0     0     0    /sys',10,0
um1:db 'umount: unmounted',10,0
fd1:db 'Device   Boot  Start   End   Sectors  Size  Id  Type',10
    db '/dev/sda1 *       1     261    261    128M  83  Linux',10,0
bk1:db '/dev/sda1: UUID="a1b2c3d4" TYPE="ext4"',10,0
pa1:db 'passwd: password updated',10,0
ua1:db 'useradd: user created',10,0
ud1:db 'userdel: user removed',10,0
su1:db 'su: switching to root',10,0
sd1:db '[sudo] password for root: ',10
    db 'Command executed with sudo',10,0
ga1:db 'groupadd: group created',10,0
rb1:db 'Rebooting system...',10,0
ht1:db 'Shutting down...',10,0
hs1:db '  1  help',10,'  2  ls',10,'  3  uname -a',10
    db '  4  cat /etc/os-release',10,'  5  ifconfig',10,0
al1:db 'alias set: ll="ls -la"',10,0
ep1:db 'export: variable set',10,0
rd1:db 'read> ',0
ts1:db 'test: condition evaluated',10,0
ex1:db '5 + 3 = 8',10,0
pf1:db 'Hello, BlackMatrixOS!',0
wh1:db '/usr/bin/command',10,0
ck1:db '305419896 1256 filename',10,0
md1:db 'd41d8cd98f00b204e9800998ecf8427e  filename',10,0
te1:db 'tee: writing to file',10,0
xa1:db 'xargs: executing command',10,0
tr2:db 'tr: translate characters',10,0
cu2:db 'cut: extracting columns',10,0
ps2:db 'paste: merging lines',10,0
fo1:db 'fold: wrapping lines',10,0
ex2:db 'expand: tabs to spaces',10,0
cm1:db 'comm: comparing sorted files',10,0
df2:db 'diff: comparing files',10,0
cw1:db ' _________________________________',10
    db '< BlackMatrixOS is udderly great! >',10
    db ' ---------------------------------',10
    db '        \   ^__^',10
    db '         \  (oo)\_______',10
    db '            (__)\       )\/\',10
    db '                ||----w |',10
    db '                ||     ||',10,0
ne1:db '       _____      ',10,0
ne2:db '      /     \     user@blackmatrix-os',10
    db '     / BMOS   \   -----------------',10
    db '    / KERNEL   \  OS: BlackMatrixOS 2.0',10
    db '   /___________\ Kernel: Monolithic x86',10
    db '   |  ||     ||  Shell: bmos-sh',10
    db '                 CPU: QEMU Virtual 1-core',10
    db '                 Mem: 8MB / 128MB',10
    db '                 Uptime: 0 min',10
    db '                 Packages: 120+ built-in',10,0
bn1:db '  ____  _            _    _   _      _   _',10
    db ' | __ )| | __ _  ___| | _| | | | ___| |_(_) ___',10
    db ' |  _ \| |/ _` |/ __| |/ / |_| |/ _ \ __| |/ __|',10
    db ' | |_) | | (_| | (__|   <|  _  |  __/ |_| | (__',10
    db ' |____/|_|\__,_|\___|_|\_\_| |_|\___|\__|_|\___|',10,0
ft1:db '  You have new mail.',10,0
sl2:db '      ====        _________',10
    db '  ___/  o  \_____/',10
    db ' /    ====   ==== \',10
    db '/___________________\',10
    db '   OO          OO',10,0
fg1:db '  ___ _ _ __  __',10
    db ' | __(_) |  \/  |',10
    db ' | _|| | | |\/| |',10
    db ' |_| |_|_|_|  |_|',10,0
ab1:db '  BlackMatrixOS v2.0 - The Matrix Operating System',10
    db '  A minimalist command-line OS with 120+ built-in tools',10
    db '  Written in x86 assembly for i686 architecture',10
    db '  GUI Mode: type "gui" or press F1',10
    db '  (c) 2026 BlackMatrix Project',10,0
gc1:db 'gcc (BlackMatrixOS) 12.2.0',10
    db 'Target: i686-elf',10,0
mk2:db 'make: GNU Make 4.4',10,0
py1:db 'Python 3.11.0 (BlackMatrixOS)',10
    db '>>> print("Hello from BlackMatrixOS!")',10
    db 'Hello from BlackMatrixOS!',10,0
nd1:db 'Node.js v18.12.0 (BlackMatrixOS)',10
    db '> console.log("Hello!")',10
    db 'Hello!',10,0
gt1:db 'git version 2.38.1 (BlackMatrixOS)',10
    db 'On branch main',10
    db 'nothing to commit, working tree clean',10,0
jv1:db 'openjdk version "17.0.5" (BlackMatrixOS)',10,0
dn2:db '.NET SDK 7.0.100 (BlackMatrixOS)',10,0
np1:db 'Nmap 7.93 scan report (simulated)',10
    db 'PORT   STATE SERVICE',10
    db '22/tcp open  ssh',10
    db '80/tcp open  http',10,0
td1:db 'tcpdump: listening on eth0 (simulated)',10,0
ipt1:db 'Chain INPUT (policy ACCEPT)',10
    db 'target  prot  opt  source    destination',10
    db 'ACCEPT  all   --   0.0.0.0/0 0.0.0.0/0',10,0
ap1:db 'apt: Package Manager (simulated)',10
    db '  apt update  - Update package lists',10
    db '  apt install - Install packages',10
    db '  apt remove  - Remove packages',10,0
ym1:db 'yum: Package Manager (simulated)',10,0
pc1:db 'pacman: Package Manager (simulated)',10,0
sn1:db 'snap: Package Manager (simulated)',10,0
pp1:db 'pip: Python Package Manager (simulated)',10,0
nm1:db 'npm: Node Package Manager (simulated)',10,0
nh1:db 'nohup: command running in background',10,0
to1:db 'timeout: command timed out',10,0
nc1:db 'nice: priority adjusted',10,0
jb1:db '[1]  running  shell',10
    db '[2]  stopped  vim',10,0
bg1:db 'bg: job resumed in background',10,0
fg2:db 'fg: job resumed in foreground',10,0
so2:db 'source: executing script',10,0
sh2:db 'BlackMatrixOS Shell v2.0',10,0
ex3:db 'logout',10,0
mn1:db 'man: manual page viewer',10
    db 'Usage: man <command>',10
    db '  Shows help for the specified command.',10,0
mcols: times 80 dd 0
; ==================== BSS ====================
section .bss
cur:    resd 1
clr:    resb 1
inp:    resb 256
inp_len:resd 1
        resb 4096
stack_top:
