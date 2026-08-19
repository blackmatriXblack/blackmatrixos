# BlackMatrix Toolkit - PowerShell Version
# 200+ System Tools for Windows and Linux (WSL)

function Show-Menu {
    Clear-Host
    Write-Host "  ============================================================" -ForegroundColor Green
    Write-Host "    BlackMatrix Toolkit v2.0 - PowerShell Edition" -ForegroundColor Green
    Write-Host "    200+ Windows + Linux System Tools" -ForegroundColor Green
    Write-Host "  ============================================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "   SYSTEM INFO" -ForegroundColor Yellow
    Write-Host "    1=Get-ComputerInfo  2=hostname  3=whoami  4=ver" -ForegroundColor Gray
    Write-Host "    5=tasklist  6=Get-Process  7=Get-Service  8=Get-HotFix" -ForegroundColor Gray
    Write-Host "    9=Get-CimInstance Win32_BIOS  10=Get-CimInstance Win32_CPU" -ForegroundColor Gray
    Write-Host "   11=Get-PhysicalDisk  12=Get-NetAdapter  13=driverquery" -ForegroundColor Gray
    Write-Host "   14=systeminfo  15=msinfo32  16=dxdiag  17=taskmgr" -ForegroundColor Gray
    Write-Host ""
    Write-Host "   NETWORK" -ForegroundColor Yellow
    Write-Host "   20=ipconfig  21=ipconfig /all  22=Test-Connection  23=tracert" -ForegroundColor Gray
    Write-Host "   24=Resolve-DnsName  25=Get-NetTCPConnection  26=Get-NetIPAddress" -ForegroundColor Gray
    Write-Host "   27=arp -a  28=route print  29=Get-NetRoute  30=getmac" -ForegroundColor Gray
    Write-Host "   31=netstat  32=net user  33=net share  34=Invoke-WebRequest" -ForegroundColor Gray
    Write-Host "   35=Invoke-RestMethod  36=curl  37=ssh  38=scp" -ForegroundColor Gray
    Write-Host ""
    Write-Host "   PROCESS / SERVICE" -ForegroundColor Yellow
    Write-Host "   40=Get-Process  41=Stop-Process  42=Get-Service  43=Start-Service" -ForegroundColor Gray
    Write-Host "   44=Stop-Service  45=Restart-Service  46=Get-ScheduledTask" -ForegroundColor Gray
    Write-Host "   47=Get-EventLog  48=Get-WinEvent  49=Get-ItemProperty(reg)" -ForegroundColor Gray
    Write-Host ""
    Write-Host "   DISK / FILE" -ForegroundColor Yellow
    Write-Host "   50=Get-ChildItem  51=Get-Volume  52=Get-Disk  53=tree" -ForegroundColor Gray
    Write-Host "   54=Get-Content  55=Set-Content  56=Select-String(grep)" -ForegroundColor Gray
    Write-Host "   57=Measure-Object(wc)  58=Sort-Object  59=Compare-Object" -ForegroundColor Gray
    Write-Host "   60=Compress-Archive(zip)  61=Expand-Archive  62=Copy-Item" -ForegroundColor Gray
    Write-Host ""
    Write-Host "   SYSTEM TOOLS" -ForegroundColor Yellow
    Write-Host "   70=regedit  71=devmgmt.msc  72=control  73=appwiz.cpl" -ForegroundColor Gray
    Write-Host "   74=ncpa.cpl  75=sysdm.cpl  76=gpedit.msc  77=msconfig" -ForegroundColor Gray
    Write-Host "   78=compmmgmt.msc  79=eventvwr  80=perfmon  81=resmon" -ForegroundColor Gray
    Write-Host "   82=mstsc  83=notepad  84=calc  85=mspaint" -ForegroundColor Gray
    Write-Host ""
    Write-Host "   SECURITY" -ForegroundColor Yellow
    Write-Host "   90=Get-MpPreference  91=Start-MpScan  92=Get-BitLockerVolume" -ForegroundColor Gray
    Write-Host "   93=Get-FirewallRule  94=Get-LocalUser  95=Get-LocalGroup" -ForegroundColor Gray
    Write-Host "   96=gpupdate  97=gpresult  98=cipher  99=certutil" -ForegroundColor Gray
    Write-Host ""
    Write-Host "   DEVELOPER" -ForegroundColor Yellow
    Write-Host "  100=python  101=node  102=npm  103=git  104=gcc" -ForegroundColor Gray
    Write-Host "  105=docker  106=java  107=dotnet  108=choco  109=winget" -ForegroundColor Gray
    Write-Host "  110=code(VSCode)  111=make  112=cmake  113=scoop" -ForegroundColor Gray
    Write-Host ""
    Write-Host "   WSL / LINUX" -ForegroundColor Yellow
    Write-Host "  120=wsl uname -a  121=wsl ls -la  122=wsl df -h" -ForegroundColor Gray
    Write-Host "  123=wsl free -h  124=wsl top -bn1  125=wsl ps aux" -ForegroundColor Gray
    Write-Host "  126=wsl cat /etc/os-release  127=wsl ip addr" -ForegroundColor Gray
    Write-Host "  128=wsl systemctl  129=wsl journalctl" -ForegroundColor Gray
    Write-Host ""
    Write-Host "   QUICK ACTIONS" -ForegroundColor Yellow
    Write-Host "  200=Custom command  201=Flush DNS  202=Release/Renew IP" -ForegroundColor Gray
    Write-Host "  203=Open ports  204=WiFi profiles  205=Installed programs" -ForegroundColor Gray
    Write-Host "  206=Environment vars  207=Clear temp  208=System health" -ForegroundColor Gray
    Write-Host "  209=Speed test  210=Shutdown  211=Restart  212=Lock" -ForegroundColor Gray
    Write-Host ""
    Write-Host "    0=Exit" -ForegroundColor Red
    Write-Host "  ============================================================" -ForegroundColor Green
}

do {
    Show-Menu
    $choice = Read-Host "  Enter number"

    switch ($choice) {
        # System Info
        "1"  { Get-ComputerInfo | Format-List; Pause }
        "2"  { hostname; Pause }
        "3"  { whoami; whoami /priv; Pause }
        "4"  { ver; Pause }
        "5"  { tasklist /v; Pause }
        "6"  { Get-Process | Sort-Object CPU -Descending | Select-Object -First 30 | Format-Table; Pause }
        "7"  { Get-Service | Format-Table Name,Status,StartType; Pause }
        "8"  { Get-HotFix | Format-Table; Pause }
        "9"  { Get-CimInstance Win32_BIOS | Format-List; Pause }
        "10" { Get-CimInstance Win32_Processor | Format-List Name,NumberOfCores,MaxClockSpeed; Pause }
        "11" { Get-PhysicalDisk | Format-Table FriendlyName,Size,MediaType,HealthStatus; Pause }
        "12" { Get-NetAdapter | Format-Table Name,Status,LinkSpeed,MacAddress; Pause }
        "13" { driverquery /v; Pause }
        "14" { systeminfo; Pause }
        "15" { Start-Process msinfo32 }
        "16" { Start-Process dxdiag }
        "17" { Start-Process taskmgr }

        # Network
        "20" { ipconfig; Pause }
        "21" { ipconfig /all; Pause }
        "22" { $h = Read-Host "Host"; Test-Connection $h -Count 4; Pause }
        "23" { $h = Read-Host "Host"; tracert $h; Pause }
        "24" { $h = Read-Host "Host"; Resolve-DnsName $h; Pause }
        "25" { Get-NetTCPConnection | Format-Table LocalPort,RemoteAddress,State; Pause }
        "26" { Get-NetIPAddress | Format-Table InterfaceAlias,IPAddress,PrefixLength; Pause }
        "27" { arp -a; Pause }
        "28" { route print; Pause }
        "29" { Get-NetRoute | Format-Table; Pause }
        "30" { getmac /v; Pause }
        "31" { netstat -an; Pause }
        "32" { net user; Pause }
        "33" { net share; Pause }
        "34" { $u = Read-Host "URL"; Invoke-WebRequest $u | Select-Object StatusCode,Headers; Pause }
        "35" { $u = Read-Host "URL"; Invoke-RestMethod $u; Pause }
        "36" { $u = Read-Host "URL"; curl -v $u; Pause }
        "37" { $t = Read-Host "user@host"; ssh $t }
        "38" { $a = Read-Host "scp args"; scp $a; Pause }

        # Process/Service
        "40" { Get-Process | Format-Table Id,Name,CPU,WorkingSet,StartTime; Pause }
        "41" { $p = Read-Host "Process name/ID"; Stop-Process -Name $p -Force -ErrorAction SilentlyContinue; Pause }
        "42" { Get-Service | Where-Object {$_.Status -eq "Running"} | Format-Table; Pause }
        "43" { $s = Read-Host "Service name"; Start-Service $s; Pause }
        "44" { $s = Read-Host "Service name"; Stop-Service $s; Pause }
        "45" { $s = Read-Host "Service name"; Restart-Service $s; Pause }
        "46" { Get-ScheduledTask | Format-Table TaskName,State,TaskPath; Pause }
        "47" { Get-EventLog -LogName System -Newest 20 | Format-Table TimeGenerated,EntryType,Source,Message -Wrap; Pause }
        "48" { Get-WinEvent -LogName System -MaxEvents 20 | Format-Table TimeCreated,LevelDisplayName,Message -Wrap; Pause }
        "49" { $k = Read-Host "Registry key"; Get-ItemProperty $k; Pause }

        # Disk/File
        "50" { Get-ChildItem -Force | Format-Table Name,Length,LastWriteTime,Mode; Pause }
        "51" { Get-Volume | Format-Table DriveLetter,FileSystemLabel,SizeRemaining,Size; Pause }
        "52" { Get-Disk | Format-Table Number,FriendlyName,Size,PartitionStyle; Pause }
        "53" { $p = Read-Host "Path"; tree $p /F; Pause }
        "54" { $f = Read-Host "File"; Get-Content $f; Pause }
        "55" { $f = Read-Host "File"; $t = Read-Host "Text"; Set-Content $f $t; Pause }
        "56" { $p = Read-Host "Pattern"; $f = Read-Host "File"; Select-String -Path $f -Pattern $p; Pause }
        "57" { $f = Read-Host "File"; Get-Content $f | Measure-Object -Line -Word -Character; Pause }
        "58" { $f = Read-Host "File"; Get-Content $f | Sort-Object; Pause }
        "59" { $a = Read-Host "File1"; $b = Read-Host "File2"; Compare-Object (Get-Content $a) (Get-Content $b); Pause }
        "60" { $s = Read-Host "Source"; $d = Read-Host "Dest zip"; Compress-Archive -Path $s -DestinationPath $d; Pause }
        "61" { $z = Read-Host "Zip file"; $d = Read-Host "Dest"; Expand-Archive -Path $z -DestinationPath $d; Pause }
        "62" { $s = Read-Host "Source"; $d = Read-Host "Dest"; Copy-Item $s $d; Pause }

        # System Tools
        "70" { Start-Process regedit }
        "71" { Start-Process devmgmt.msc }
        "72" { Start-Process control }
        "73" { Start-Process appwiz.cpl }
        "74" { Start-Process ncpa.cpl }
        "75" { Start-Process sysdm.cpl }
        "76" { Start-Process gpedit.msc }
        "77" { Start-Process msconfig }
        "78" { Start-Process compmgmt.msc }
        "79" { Start-Process eventvwr }
        "80" { Start-Process perfmon }
        "81" { Start-Process resmon }
        "82" { Start-Process mstsc }
        "83" { Start-Process notepad }
        "84" { Start-Process calc }
        "85" { Start-Process mspaint }

        # Security
        "90" { Get-MpPreference | Format-List; Pause }
        "91" { Start-MpScan -ScanType QuickScan; Pause }
        "92" { Get-BitLockerVolume; Pause }
        "93" { Get-NetFirewallRule | Where-Object {$_.Enabled -eq 'True'} | Format-Table DisplayName,Direction,Action; Pause }
        "94" { Get-LocalUser | Format-Table Name,Enabled,LastLogon; Pause }
        "95" { Get-LocalGroup | Format-Table Name,Description; Pause }
        "96" { gpupdate /force; Pause }
        "97" { gpresult /r; Pause }
        "98" { cipher /c; Pause }
        "99" { certutil -store My; Pause }

        # Developer
        "100" { python --version 2>&1; python }
        "101" { node --version 2>&1; node }
        "102" { $c = Read-Host "npm cmd"; npm $c; Pause }
        "103" { git --version; git status; Pause }
        "104" { gcc --version 2>&1; Pause }
        "105" { docker ps; Pause }
        "106" { java -version 2>&1; Pause }
        "107" { dotnet --version 2>&1; dotnet; Pause }
        "108" { $c = Read-Host "choco cmd"; choco $c; Pause }
        "109" { $c = Read-Host "winget cmd"; winget $c; Pause }
        "110" { Start-Process code }
        "111" { make --version 2>&1; Pause }
        "112" { cmake --version 2>&1; Pause }
        "113" { $c = Read-Host "scoop cmd"; scoop $c; Pause }

        # WSL/Linux
        "120" { wsl uname -a; Pause }
        "121" { wsl ls -la; Pause }
        "122" { wsl df -h; Pause }
        "123" { wsl free -h; Pause }
        "124" { wsl top -bn1 | head -20; Pause }
        "125" { wsl ps aux; Pause }
        "126" { wsl cat /etc/os-release; Pause }
        "127" { wsl ip addr; Pause }
        "128" { $c = Read-Host "systemctl cmd"; wsl systemctl $c; Pause }
        "129" { wsl journalctl -n 20; Pause }

        # Quick Actions
        "200" { $c = Read-Host "Command"; Invoke-Expression $c; Pause }
        "201" { ipconfig /flushdns; Pause }
        "202" { ipconfig /release; ipconfig /renew; Pause }
        "203" { Get-NetTCPConnection -State Listen | Format-Table LocalPort,OwningProcess | Sort-Object LocalPort; Pause }
        "204" { netsh wlan show profiles; Pause }
        "205" { Get-ItemProperty HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall\* | Select-Object DisplayName,DisplayVersion,Publisher | Sort-Object DisplayName | Format-Table; Pause }
        "206" { Get-ChildItem Env: | Format-Table Name,Value; Pause }
        "207" { Remove-Item -Path $env:TEMP\* -Recurse -Force -ErrorAction SilentlyContinue; Write-Host "Temp cleared!"; Pause }
        "208" {
            Write-Host "`n  === SYSTEM HEALTH ===" -ForegroundColor Green
            Write-Host "  CPU:" -NoNewline; (Get-CimInstance Win32_Processor).Name
            Write-Host "  Load:" -NoNewline; (Get-CimInstance Win32_Processor).LoadPercentage; Write-Host "%"
            Write-Host "  RAM:" -NoNewline; [math]::Round((Get-CimInstance Win32_OperatingSystem).TotalVisibleMemorySize/1MB,2); Write-Host " GB"
            Write-Host "  Free:" -NoNewline; [math]::Round((Get-CimInstance Win32_OperatingSystem).FreePhysicalMemory/1MB,2); Write-Host " GB"
            Write-Host "  Uptime:" -NoNewline; (Get-Date) - (Get-CimInstance Win32_OperatingSystem).LastBootUpTime
            Get-Volume | Where-Object {$_.DriveLetter} | Format-Table DriveLetter,SizeRemaining,Size
            Get-NetAdapter | Where-Object Status -eq "Up" | Format-Table Name,LinkSpeed
            Pause
        }
        "209" { Write-Host "Testing..."; curl -o $null https://speed.cloudflare.com/__down?bytes=10000000 -w "Speed: %{speed_download} bytes/sec`n" 2>&1; Pause }
        "210" { Stop-Computer -Force }
        "211" { Restart-Computer -Force }
        "212" { rundll32.exe user32.dll,LockWorkStation }
        "0"  { Write-Host "Bye!"; return }
    }
} while ($true)
