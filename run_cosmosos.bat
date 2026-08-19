@echo off
REM CosmosOS Launcher
REM Requires QEMU installed and in PATH

echo ====================================
echo  CosmosOS Launcher
echo ====================================
echo.

REM Check if QEMU is available
where qemu-system-i386 >nul 2>&1
if errorlevel 1 (
    echo ERROR: qemu-system-i386 not found in PATH
    echo Please install QEMU from https://www.qemu.org/download/
    echo and add it to your system PATH.
    pause
    exit /b 1
)

echo Starting CosmosOS in QEMU...
echo Press Ctrl+Alt+G to release mouse cursor
echo Close QEMU window to exit
echo.

qemu-system-i386 -drive format=raw,file="%~dp0cosmosos.img" -m 128M -monitor stdio
