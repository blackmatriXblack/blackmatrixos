@echo off
chcp 65001 >nul 2>&1
title BlackMatrix Toolkit - System Tools Launcher
color 0A
setlocal enabledelayedexpansion

:MENU
cls
echo.
echo  ╔══════════════════════════════════════════════════════════════╗
echo  ║           BlackMatrix Toolkit v2.0                          ║
echo  ║           Windows + Linux System Tools                      ║
echo  ╚══════════════════════════════════════════════════════════════╝
echo.
echo  ═══ SYSTEM INFO ══════════════════════════════════════════════
echo   [1]   systeminfo      - Full system information
echo   [2]   hostname        - Computer name
echo   [3]   whoami          - Current user
echo   [4]   ver             - Windows version
echo   [5]   winver          - Windows version GUI
echo   [6]   msinfo32        - System Info GUI
echo   [7]   dxdiag          - DirectX Diagnostic
echo   [8]   tasklist        - Running processes
echo   [9]   taskmgr         - Task Manager GUI
echo  [10]   wmic cpu        - CPU information
echo  [11]   wmic bios       - BIOS information
echo  [12]   wmic memorychip - RAM information
echo  [13]   wmic diskdrive  - Disk drive info
echo  [14]   wmic nic        - Network adapters
echo  [15]   driverquery     - Installed drivers
echo  [16]   powercfg        - Power configuration
echo.
echo  ═══ NETWORK ══════════════════════════════════════════════════
echo  [20]   ipconfig        - IP configuration
echo  [21]   ipconfig /all   - Full IP config
echo  [22]   ping            - Ping a host
echo  [23]   tracert         - Trace route
echo  [24]   nslookup        - DNS lookup
echo  [25]   netstat         - Network connections
echo  [26]   netstat -an     - All connections
echo  [27]   arp -a          - ARP table
echo  [28]   route print     - Routing table
echo  [29]   nbtstat         - NetBIOS stats
echo  [30]   getmac          - MAC addresses
echo  [31]   netsh           - Network shell
echo  [32]   net user        - User accounts
echo  [33]   net share       - Shared folders
echo  [34]   net session     - Active sessions
echo  [35]   net start       - Running services
echo  [36]   curl            - HTTP client
echo  [37]   wget            - Download file
echo  [38]   ssh             - SSH client
echo.
echo  ═══ DISK / FILESYSTEM ════════════════════════════════════════
echo  [40]   dir             - List directory
echo  [41]   tree            - Directory tree
echo  [42]   chkdsk          - Check disk
echo  [43]   diskpart        - Disk partition tool
echo  [44]   fsutil          - File system utility
echo  [45]   defrag          - Defragment disk
echo  [46]   cleanmgr        - Disk cleanup GUI
echo  [47]   vol             - Volume label
echo  [48]   mountvol        - Mount points
echo  [49]   sfc /scannow    - System file checker
echo  [50]   DISM            - Deployment Image Servicing
echo.
echo  ═══ PROCESS / SERVICE ════════════════════════════════════════
echo  [55]   sc query        - List services
echo  [56]   sc start        - Start a service
echo  [57]   sc stop         - Stop a service
echo  [58]   taskkill        - Kill a process
echo  [59]   wmic process    - Process details
echo  [60]   wmic service    - Service details
echo  [61]   schtasks        - Scheduled tasks
echo  [62]   reg query       - Registry query
echo  [63]   reg add         - Registry add
echo  [64]   reg delete      - Registry delete
echo  [65]   gpupdate        - Group policy update
echo  [66]   gpresult        - Group policy result
echo.
echo  ═══ SECURITY ═════════════════════════════════════════════════
echo  [70]   firewall        - Windows Firewall
echo  [71]   netsh advfirewall - Advanced firewall
echo  [72]   cipher          - EFS encryption
echo  [73]   certutil        - Certificate utility
echo  [74]   secpol          - Security Policy
echo  [75]   gpedit          - Group Policy Editor
echo  [76]   compmgmt        - Computer Management
echo  [77]   lusrmgr         - Local Users/Groups
echo.
echo  ═══ SYSTEM TOOLS ═════════════════════════════════════════════
echo  [80]   cmd             - Command Prompt
echo  [81]   powershell      - PowerShell
echo  [82]   wsl             - Windows Subsystem Linux
echo  [83]   regedit         - Registry Editor
echo  [84]   devmgmt         - Device Manager
echo  [85]   control         - Control Panel
echo  [86]   appwiz.cpl      - Programs and Features
echo  [87]   ncpa.cpl        - Network Connections
echo  [88]   firewall.cpl    - Firewall Settings
echo  [89]   sysdm.cpl       - System Properties
echo  [90]   mstsc           - Remote Desktop
echo  [91]   resmon          - Resource Monitor
echo  [92]   perfmon         - Performance Monitor
echo  [93]   eventvwr        - Event Viewer
echo  [94]   mmc             - Microsoft Management Console
echo  [95]   msconfig        - System Configuration
echo  [96]   msra            - Remote Assistance
echo  [97]   osk             - On-Screen Keyboard
echo  [98]   magnify         - Magnifier
echo  [99]   narrator        - Screen Reader
echo.
echo  ═══ DEVELOPER TOOLS ══════════════════════════════════════════
echo [110]   python          - Python interpreter
echo [111]   node            - Node.js
echo [112]   npm             - Node package manager
echo [113]   git             - Git version control
echo [114]   gcc             - GNU C compiler
echo [115]   make            - GNU Make
echo [116]   cmake           - CMake build
echo [117]   java            - Java runtime
echo [118]   javac           - Java compiler
echo [119]   dotnet          - .NET CLI
echo [120]   choco           - Chocolatey
echo [121]   winget          - Windows Package Manager
echo [122]   scoop           - Scoop package manager
echo [123]   docker          - Docker containers
echo [124]   vim / nano      - Text editors
echo [125]   grep            - Search text
echo [126]   findstr         - Windows findstr
echo [127]   awk             - Text processing
echo [128]   sed             - Stream editor
echo [129]   sort            - Sort lines
echo [130]   tee             - Pipe to file
echo.
echo  ═══ MEDIA / UTILITIES ════════════════════════════════════════
echo [140]   notepad         - Notepad editor
echo [141]   wordpad         - WordPad editor
echo [142]   mspaint         - Paint
echo [143]   calc            - Calculator
echo [144]   snippingtool    - Screen capture
echo [145]   explorer        - File Explorer
echo [146]   clip            - Copy to clipboard
echo [147]   shutdown        - Shutdown computer
echo [148]   restart         - Restart computer
echo [149]   logoff          - Log off
echo [150]   lock            - Lock workstation
echo.
echo  ═══ LINUX TOOLS (WSL/Git Bash) ══════════════════════════════
echo [160]   ls -la          - List files
echo [161]   cat             - Display file
echo [162]   grep            - Search pattern
echo [163]   find            - Find files
echo [164]   chmod           - Change permissions
echo [165]   chown           - Change owner
echo [166]   tar             - Archive files
echo [167]   gzip/gunzip     - Compress/decompress
echo [168]   wget            - Download files
echo [169]   curl            - Transfer data
echo [170]   ssh             - Secure shell
echo [171]   scp             - Secure copy
echo [172]   rsync           - Remote sync
echo [173]   df -h           - Disk usage
echo [174]   du -sh          - Directory size
echo [175]   top / htop      - Process monitor
echo [176]   ps aux          - Process list
echo [177]   kill            - Kill process
echo [178]   free -h         - Memory info
echo [179]   uname -a        - System info
echo [180]   lscpu           - CPU info
echo.
echo  ═══ QUICK ACTIONS ════════════════════════════════════════════
echo [200]   Run custom command
echo [201]   Ping Google (8.8.8.8)
echo [202]   Flush DNS cache
echo [203]   Release / Renew IP
echo [204]   Show open ports
echo [205]   Show WiFi profiles
echo [206]   Show installed programs
echo [207]   Show environment vars
echo [208]   Clear temp files
echo [209]   System health check
echo [210]   Network speed test
echo.
echo  [0]    Exit
echo.
echo  ════════════════════════════════════════════════════════════════
set /p choice="Enter number: "

if "%choice%"=="1" goto :SYSINFO
if "%choice%"=="2" goto :HOSTNAME
if "%choice%"=="3" goto :WHOAMI
if "%choice%"=="4" goto :VER
if "%choice%"=="5" goto :WINVER
if "%choice%"=="6" goto :MSINFO32
if "%choice%"=="7" goto :DXDIAG
if "%choice%"=="8" goto :TASKLIST
if "%choice%"=="9" goto :TASKMGR
if "%choice%"=="10" goto :WMIC_CPU
if "%choice%"=="11" goto :WMIC_BIOS
if "%choice%"=="12" goto :WMIC_MEM
if "%choice%"=="13" goto :WMIC_DISK
if "%choice%"=="14" goto :WMIC_NIC
if "%choice%"=="15" goto :DRIVERQUERY
if "%choice%"=="16" goto :POWERCFG

if "%choice%"=="20" goto :IPCONFIG
if "%choice%"=="21" goto :IPCONFIG_ALL
if "%choice%"=="22" goto :PING
if "%choice%"=="23" goto :TRACERT
if "%choice%"=="24" goto :NSLOOKUP
if "%choice%"=="25" goto :NETSTAT
if "%choice%"=="26" goto :NETSTAT_ALL
if "%choice%"=="27" goto :ARP
if "%choice%"=="28" goto :ROUTE
if "%choice%"=="29" goto :NBTSTAT
if "%choice%"=="30" goto :GETMAC
if "%choice%"=="31" goto :NETSH
if "%choice%"=="32" goto :NET_USER
if "%choice%"=="33" goto :NET_SHARE
if "%choice%"=="34" goto :NET_SESSION
if "%choice%"=="35" goto :NET_START
if "%choice%"=="36" goto :CURL
if "%choice%"=="37" goto :WGET
if "%choice%"=="38" goto :SSH

if "%choice%"=="40" goto :DIR
if "%choice%"=="41" goto :TREE
if "%choice%"=="42" goto :CHKDSK
if "%choice%"=="43" goto :DISKPART
if "%choice%"=="44" goto :FSUTIL
if "%choice%"=="45" goto :DEFRAG
if "%choice%"=="46" goto :CLEANMGR
if "%choice%"=="47" goto :VOL
if "%choice%"=="48" goto :MOUNTVOL
if "%choice%"=="49" goto :SFC
if "%choice%"=="50" goto :DISM

if "%choice%"=="55" goto :SC_QUERY
if "%choice%"=="56" goto :SC_START
if "%choice%"=="57" goto :SC_STOP
if "%choice%"=="58" goto :TASKKILL
if "%choice%"=="59" goto :WMIC_PROC
if "%choice%"=="60" goto :WMIC_SVC
if "%choice%"=="61" goto :SCHTASKS
if "%choice%"=="62" goto :REG_QUERY
if "%choice%"=="63" goto :REG_ADD
if "%choice%"=="64" goto :REG_DEL
if "%choice%"=="65" goto :GPUPDATE
if "%choice%"=="66" goto :GPRESULT

if "%choice%"=="70" goto :FIREWALL
if "%choice%"=="71" goto :NETSH_FW
if "%choice%"=="72" goto :CIPHER
if "%choice%"=="73" goto :CERTUTIL
if "%choice%"=="74" goto :SECPOL
if "%choice%"=="75" goto :GPEDIT
if "%choice%"=="76" goto :COMPMGMT
if "%choice%"=="77" goto :LUSRMGR

if "%choice%"=="80" goto :CMD_OPEN
if "%choice%"=="81" goto :POWERSHELL
if "%choice%"=="82" goto :WSL
if "%choice%"=="83" goto :REGEDIT
if "%choice%"=="84" goto :DEVMGMT
if "%choice%"=="85" goto :CONTROL
if "%choice%"=="86" goto :APPWIZ
if "%choice%"=="87" goto :NCPA
if "%choice%"=="88" goto :FW_CPL
if "%choice%"=="89" goto :SYSDM
if "%choice%"=="90" goto :MSTSC
if "%choice%"=="91" goto :RESMON
if "%choice%"=="92" goto :PERFMON
if "%choice%"=="93" goto :EVENTVWR
if "%choice%"=="94" goto :MMC
if "%choice%"=="95" goto :MSCONFIG
if "%choice%"=="96" goto :MSRA
if "%choice%"=="97" goto :OSK
if "%choice%"=="98" goto :MAGNIFY
if "%choice%"=="99" goto :NARRATOR

if "%choice%"=="110" goto :PYTHON
if "%choice%"=="111" goto :NODE
if "%choice%"=="112" goto :NPM
if "%choice%"=="113" goto :GIT
if "%choice%"=="114" goto :GCC
if "%choice%"=="115" goto :MAKE
if "%choice%"=="116" goto :CMAKE
if "%choice%"=="117" goto :JAVA
if "%choice%"=="118" goto :JAVAC
if "%choice%"=="119" goto :DOTNET
if "%choice%"=="120" goto :CHOCO
if "%choice%"=="121" goto :WINGET
if "%choice%"=="122" goto :SCOOP
if "%choice%"=="123" goto :DOCKER
if "%choice%"=="124" goto :VIM
if "%choice%"=="125" goto :GREP
if "%choice%"=="126" goto :FINDSTR
if "%choice%"=="127" goto :AWK
if "%choice%"=="128" goto :SED
if "%choice%"=="129" goto :SORT
if "%choice%"=="130" goto :TEE

if "%choice%"=="140" goto :NOTEPAD
if "%choice%"=="141" goto :WORDPAD
if "%choice%"=="142" goto :MSPAINT
if "%choice%"=="143" goto :CALC
if "%choice%"=="144" goto :SNIPPING
if "%choice%"=="145" goto :EXPLORER
if "%choice%"=="146" goto :CLIP
if "%choice%"=="147" goto :SHUTDOWN
if "%choice%"=="148" goto :RESTART
if "%choice%"=="149" goto :LOGOFF
if "%choice%"=="150" goto :LOCK

if "%choice%"=="160" goto :LINUX_LS
if "%choice%"=="161" goto :LINUX_CAT
if "%choice%"=="162" goto :LINUX_GREP
if "%choice%"=="163" goto :LINUX_FIND
if "%choice%"=="164" goto :LINUX_CHMOD
if "%choice%"=="165" goto :LINUX_CHOWN
if "%choice%"=="166" goto :LINUX_TAR
if "%choice%"=="167" goto :LINUX_GZIP
if "%choice%"=="168" goto :LINUX_WGET
if "%choice%"=="169" goto :LINUX_CURL
if "%choice%"=="170" goto :LINUX_SSH
if "%choice%"=="171" goto :LINUX_SCP
if "%choice%"=="172" goto :LINUX_RSYNC
if "%choice%"=="173" goto :LINUX_DF
if "%choice%"=="174" goto :LINUX_DU
if "%choice%"=="175" goto :LINUX_TOP
if "%choice%"=="176" goto :LINUX_PS
if "%choice%"=="177" goto :LINUX_KILL
if "%choice%"=="178" goto :LINUX_FREE
if "%choice%"=="179" goto :LINUX_UNAME
if "%choice%"=="180" goto :LINUX_LSCPU

if "%choice%"=="200" goto :CUSTOM
if "%choice%"=="201" goto :PING_GOOGLE
if "%choice%"=="202" goto :FLUSH_DNS
if "%choice%"=="203" goto :RENEW_IP
if "%choice%"=="204" goto :OPEN_PORTS
if "%choice%"=="205" goto :WIFI_PROFILES
if "%choice%"=="206" goto :INSTALLED
if "%choice%"=="207" goto :ENVVARS
if "%choice%"=="208" goto :CLRTEMP
if "%choice%"=="209" goto :HEALTH
if "%choice%"=="210" goto :SPEEDTEST
if "%choice%"=="0" goto :EXIT
goto :MENU

:: ===== SYSTEM INFO =====
:SYSINFO
cls && echo. && systeminfo && pause && goto :MENU
:HOSTNAME
cls && echo. && hostname && pause && goto :MENU
:WHOAMI
cls && echo. && whoami && whoami /priv && whoami /groups && pause && goto :MENU
:VER
cls && echo. && ver && pause && goto :MENU
:WINVER
start winver && goto :MENU
:MSINFO32
start msinfo32 && goto :MENU
:DXDIAG
start dxdiag && goto :MENU
:TASKLIST
cls && echo. && tasklist /v && pause && goto :MENU
:TASKMGR
start taskmgr && goto :MENU
:WMIC_CPU
cls && echo. && wmic cpu get name,numberofcores,numberoflogicalprocessors,maxclockspeed,currentclockspeed && pause && goto :MENU
:WMIC_BIOS
cls && echo. && wmic bios get manufacturer,name,serialnumber,version && pause && goto :MENU
:WMIC_MEM
cls && echo. && wmic memorychip get capacity,speed,manufacturer,partnumber && pause && goto :MENU
:WMIC_DISK
cls && echo. && wmic diskdrive get model,size,mediatype,interfaceType && pause && goto :MENU
:WMIC_NIC
cls && echo. && wmic nic get name,macaddress,netconnectionstatus && pause && goto :MENU
:DRIVERQUERY
cls && echo. && driverquery /v && pause && goto :MENU
:POWERCFG
cls && echo. && powercfg /list && pause && goto :MENU

:: ===== NETWORK =====
:IPCONFIG
cls && echo. && ipconfig && pause && goto :MENU
:IPCONFIG_ALL
cls && echo. && ipconfig /all && pause && goto :MENU
:PING
cls && set /p host="Enter host: " && ping !host! && pause && goto :MENU
:TRACERT
cls && set /p host="Enter host: " && tracert !host! && pause && goto :MENU
:NSLOOKUP
cls && set /p host="Enter host: " && nslookup !host! && pause && goto :MENU
:NETSTAT
cls && echo. && netstat && pause && goto :MENU
:NETSTAT_ALL
cls && echo. && netstat -anob && pause && goto :MENU
:ARP
cls && echo. && arp -a && pause && goto :MENU
:ROUTE
cls && echo. && route print && pause && goto :MENU
:NBTSTAT
cls && echo. && nbtstat -r && pause && goto :MENU
:GETMAC
cls && echo. && getmac /v && pause && goto :MENU
:NETSH
cls && echo. && netsh interface show interface && pause && goto :MENU
:NET_USER
cls && echo. && net user && pause && goto :MENU
:NET_SHARE
cls && echo. && net share && pause && goto :MENU
:NET_SESSION
cls && echo. && net session && pause && goto :MENU
:NET_START
cls && echo. && net start && pause && goto :MENU
:CURL
cls && set /p url="Enter URL: " && curl -v !url! && pause && goto :MENU
:WGET
cls && set /p url="Enter URL: " && curl -LO !url! && pause && goto :MENU
:SSH
cls && set /p target="Enter user@host: " && ssh !target! && goto :MENU

:: ===== DISK =====
:DIR
cls && echo. && dir /a && pause && goto :MENU
:TREE
cls && echo. && tree /F && pause && goto :MENU
:CHKDSK
cls && echo. && chkdsk C: && pause && goto :MENU
:DISKPART
start diskpart && goto :MENU
:FSUTIL
cls && echo. && fsutil fsinfo drives && pause && goto :MENU
:DEFRAG
cls && echo. && defrag C: /O && pause && goto :MENU
:CLEANMGR
start cleanmgr && goto :MENU
:VOL
cls && echo. && vol && pause && goto :MENU
:MOUNTVOL
cls && echo. && mountvol && pause && goto :MENU
:SFC
cls && echo. && sfc /scannow && pause && goto :MENU
:DISM
cls && echo. && DISM /Online /Cleanup-Image /CheckHealth && pause && goto :MENU

:: ===== PROCESS/SERVICE =====
:SC_QUERY
cls && echo. && sc query type= all state= all && pause && goto :MENU
:SC_START
cls && set /p svc="Service name: " && sc start !svc! && pause && goto :MENU
:SC_STOP
cls && set /p svc="Service name: " && sc stop !svc! && pause && goto :MENU
:TASKKILL
cls && set /p pid="PID or name: " && taskkill /F /IM !pid! && pause && goto :MENU
:WMIC_PROC
cls && echo. && wmic process get name,processid,workingsetsize /format:table | more && pause && goto :MENU
:WMIC_SVC
cls && echo. && wmic service get name,state,startmode /format:table | more && pause && goto :MENU
:SCHTASKS
cls && echo. && schtasks /query /fo TABLE | more && pause && goto :MENU
:REG_QUERY
cls && set /p key="Registry key: " && reg query !key! && pause && goto :MENU
:REG_ADD
cls && set /p key="Key: " && set /p val="Value: " && set /p data="Data: " && reg add !key! /v !val! /d !data! /f && pause && goto :MENU
:REG_DEL
cls && set /p key="Key: " && set /p val="Value: " && reg delete !key! /v !val! /f && pause && goto :MENU
:GPUPDATE
cls && echo. && gpupdate /force && pause && goto :MENU
:GPRESULT
cls && echo. && gpresult /r && pause && goto :MENU

:: ===== SECURITY =====
:FIREWALL
cls && echo. && netsh advfirewall show allprofiles && pause && goto :MENU
:NETSH_FW
cls && echo. && netsh advfirewall firewall show rule name=all | more && pause && goto :MENU
:CIPHER
cls && echo. && cipher /c && pause && goto :MENU
:CERTUTIL
cls && echo. && certutil -store My && pause && goto :MENU
:SECPOL
start secpol.msc && goto :MENU
:GPEDIT
start gpedit.msc && goto :MENU
:COMPMGMT
start compmgmt.msc && goto :MENU
:LUSRMGR
start lusrmgr.msc && goto :MENU

:: ===== SYSTEM TOOLS =====
:CMD_OPEN
start cmd && goto :MENU
:POWERSHELL
start powershell && goto :MENU
:WSL
start wsl && goto :MENU
:REGEDIT
start regedit && goto :MENU
:DEVMGMT
start devmgmt.msc && goto :MENU
:CONTROL
start control && goto :MENU
:APPWIZ
start appwiz.cpl && goto :MENU
:NCPA
start ncpa.cpl && goto :MENU
:FW_CPL
start firewall.cpl && goto :MENU
:SYSDM
start sysdm.cpl && goto :MENU
:MSTSC
start mstsc && goto :MENU
:RESMON
start resmon && goto :MENU
:PERFMON
start perfmon && goto :MENU
:EVENTVWR
start eventvwr && goto :MENU
:MMC
start mmc && goto :MENU
:MSCONFIG
start msconfig && goto :MENU
:MSRA
start msra && goto :MENU
:OSK
start osk && goto :MENU
:MAGNIFY
start magnify && goto :MENU
:NARRATOR
start narrator && goto :MENU

:: ===== DEVELOPER TOOLS =====
:PYTHON
cls && echo. && python --version 2>&1 && python && goto :MENU
:NODE
cls && echo. && node --version 2>&1 && node && goto :MENU
:NPM
cls && echo. && npm --version 2>&1 && set /p cmd="npm command: " && npm !cmd! && pause && goto :MENU
:GIT
cls && echo. && git --version 2>&1 && git status && pause && goto :MENU
:GCC
cls && echo. && gcc --version 2>&1 && pause && goto :MENU
:MAKE
cls && echo. && make --version 2>&1 && pause && goto :MENU
:CMAKE
cls && echo. && cmake --version 2>&1 && pause && goto :MENU
:JAVA
cls && echo. && java -version 2>&1 && pause && goto :MENU
:JAVAC
cls && echo. && javac -version 2>&1 && pause && goto :MENU
:DOTNET
cls && echo. && dotnet --version 2>&1 && dotnet && goto :MENU
:CHOCO
cls && echo. && choco --version 2>&1 && set /p cmd="choco command: " && choco !cmd! && pause && goto :MENU
:WINGET
cls && echo. && winget --version 2>&1 && set /p cmd="winget command: " && winget !cmd! && pause && goto :MENU
:SCOOP
cls && echo. && scoop --version 2>&1 && set /p cmd="scoop command: " && scoop !cmd! && pause && goto :MENU
:DOCKER
cls && echo. && docker --version 2>&1 && docker ps && pause && goto :MENU
:VIM
cls && set /p file="File to edit: " && vim !file! && goto :MENU
:GREP
cls && set /p pat="Pattern: " && set /p file="File: " && grep !pat! !file! && pause && goto :MENU
:FINDSTR
cls && set /p pat="Pattern: " && set /p file="File: " && findstr /i /n !pat! !file! && pause && goto :MENU
:AWK
cls && echo. && awk --version 2>&1 && pause && goto :MENU
:SED
cls && echo. && sed --version 2>&1 && pause && goto :MENU
:SORT
cls && set /p file="File to sort: " && sort !file! && pause && goto :MENU
:TEE
cls && set /p file="Output file: " && echo Type text (Ctrl+Z to end) && tee !file! && pause && goto :MENU

:: ===== MEDIA =====
:NOTEPAD
start notepad && goto :MENU
:WORDPAD
start write && goto :MENU
:MSPAINT
start mspaint && goto :MENU
:CALC
start calc && goto :MENU
:SNIPPING
start snippingtool && goto :MENU
:EXPLORER
start explorer && goto :MENU
:CLIP
cls && set /p text="Text to copy: " && echo !text! | clip && echo Copied to clipboard! && pause && goto :MENU
:SHUTDOWN
shutdown /s /t 10 && goto :MENU
:RESTART
shutdown /r /t 10 && goto :MENU
:LOGOFF
shutdown /l && goto :MENU
:LOCK
rundll32.exe user32.dll,LockWorkStation && goto :MENU

:: ===== LINUX TOOLS (via bash/wsl) =====
:LINUX_LS
cls && echo. && bash -c "ls -la" 2>nul || wsl ls -la 2>nul || echo "WSL/Git Bash not found" && pause && goto :MENU
:LINUX_CAT
cls && set /p file="File: " && bash -c "cat !file!" 2>nul || wsl cat !file! 2>nul || findstr .* !file! && pause && goto :MENU
:LINUX_GREP
cls && set /p pat="Pattern: " && set /p file="File: " && bash -c "grep !pat! !file!" 2>nul || wsl grep !pat! !file! 2>nul || findstr !pat! !file! && pause && goto :MENU
:LINUX_FIND
cls && set /p path="Path: " && set /p name="Name: " && bash -c "find !path! -name '!name!'" 2>nul || wsl find !path! -name '!name!' 2>nul && pause && goto :MENU
:LINUX_CHMOD
cls && set /p mode="Mode: " && set /p file="File: " && bash -c "chmod !mode! !file!" 2>nul || wsl chmod !mode! !file! 2>nul && pause && goto :MENU
:LINUX_CHOWN
cls && set /p owner="Owner: " && set /p file="File: " && bash -c "chown !owner! !file!" 2>nul || wsl chown !owner! !file! 2>nul && pause && goto :MENU
:LINUX_TAR
cls && set /p args="tar args: " && bash -c "tar !args!" 2>nul || wsl tar !args! 2>nul && pause && goto :MENU
:LINUX_GZIP
cls && set /p file="File: " && bash -c "gzip !file!" 2>nul || wsl gzip !file! 2>nul && pause && goto :MENU
:LINUX_WGET
cls && set /p url="URL: " && bash -c "wget !url!" 2>nul || wsl wget !url! 2>nul || curl -LO !url! && pause && goto :MENU
:LINUX_CURL
cls && set /p url="URL: " && bash -c "curl !url!" 2>nul || wsl curl !url! 2>nul || curl !url! && pause && goto :MENU
:LINUX_SSH
cls && set /p target="user@host: " && bash -c "ssh !target!" 2>nul || wsl ssh !target! 2>nul || ssh !target! && goto :MENU
:LINUX_SCP
cls && set /p args="scp args: " && bash -c "scp !args!" 2>nul || wsl scp !args! 2>nul || scp !args! && pause && goto :MENU
:LINUX_RSYNC
cls && set /p args="rsync args: " && bash -c "rsync !args!" 2>nul || wsl rsync !args! 2>nul && pause && goto :MENU
:LINUX_DF
cls && echo. && bash -c "df -h" 2>nul || wsl df -h 2>nul || wmic logicaldisk get size,freespace,caption && pause && goto :MENU
:LINUX_DU
cls && set /p path="Path: " && bash -c "du -sh !path!" 2>nul || wsl du -sh !path! 2>nul && pause && goto :MENU
:LINUX_TOP
cls && bash -c "top -n 1" 2>nul || wsl top -n 1 2>nul || tasklist /v | more && pause && goto :MENU
:LINUX_PS
cls && bash -c "ps aux" 2>nul || wsl ps aux 2>nul || tasklist /v && pause && goto :MENU
:LINUX_KILL
cls && set /p pid="PID: " && bash -c "kill -9 !pid!" 2>nul || wsl kill -9 !pid! 2>nul || taskkill /F /PID !pid! && pause && goto :MENU
:LINUX_FREE
cls && bash -c "free -h" 2>nul || wsl free -h 2>nul || wmic OS get FreePhysicalMemory,TotalVisibleMemorySize && pause && goto :MENU
:LINUX_UNAME
cls && bash -c "uname -a" 2>nul || wsl uname -a 2>nul || ver && pause && goto :MENU
:LINUX_LSCPU
cls && bash -c "lscpu" 2>nul || wsl lscpu 2>nul || wmic cpu get name,numberofcores && pause && goto :MENU

:: ===== QUICK ACTIONS =====
:CUSTOM
cls && set /p cmd="Enter command: " && !cmd! && pause && goto :MENU
:PING_GOOGLE
cls && echo. && ping 8.8.8.8 -n 10 && pause && goto :MENU
:FLUSH_DNS
cls && echo. && ipconfig /flushdns && pause && goto :MENU
:RENEW_IP
cls && echo. && ipconfig /release && ipconfig /renew && pause && goto :MENU
:OPEN_PORTS
cls && echo. && netstat -an | findstr LISTENING && pause && goto :MENU
:WIFI_PROFILES
cls && echo. && netsh wlan show profiles && pause && goto :MENU
:INSTALLED
cls && echo. && wmic product get name,version | more && pause && goto :MENU
:ENVVARS
cls && echo. && set && pause && goto :MENU
:CLRTEMP
cls && echo. && del /q /s %TEMP%\* 2>nul && echo Temp files cleared! && pause && goto :MENU
:HEALTH
cls
echo.
echo  ╔══════════════════════════════════════════════╗
echo  ║         SYSTEM HEALTH CHECK                  ║
echo  ╚══════════════════════════════════════════════╝
echo.
echo  --- System Info ---
systeminfo | findstr /B /C:"OS Name" /C:"OS Version" /C:"System Boot Time" /C:"Total Physical Memory" /C:"Available Physical Memory"
echo.
echo  --- CPU Load ---
wmic cpu get loadpercentage /value
echo.
echo  --- Disk Space ---
wmic logicaldisk get caption,size,freespace /format:table
echo.
echo  --- Network ---
ipconfig | findstr IPv4
echo.
echo  --- Running Processes ---
echo  Total: %NUMBER_OF_PROCESSORS% processors
tasklist | find /c /v ""
echo  processes running
echo.
echo  --- Uptime ---
net statistics workstation | findstr "Statistics since"
echo.
pause && goto :MENU
:SPEEDTEST
cls && echo. && echo Testing network speed... && curl -o NUL https://speed.cloudflare.com/__down?bytes=10000000 -w "Download: %%{speed_download} bytes/sec\n" 2>&1 && pause && goto :MENU

:EXIT
echo.
echo  Goodbye!
exit /b 0
