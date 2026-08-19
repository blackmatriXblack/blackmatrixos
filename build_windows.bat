@echo off
REM CosmosOS Build Script for Windows
REM Uses TDM-GCC (32-bit capable) for C, NASM (win32 output) for asm,
REM and MSYS2 binutils for linking. Verified working end-to-end.

set TDM=D:\TDM-GCC
set MSYS=C:\msys64
set CC=%TDM%\bin\gcc.exe -std=c17 -m32 -ffreestanding -fno-pie -fno-stack-protector -mno-stack-arg-probe -fno-builtin -nostdlib -nostdinc -Wall -Wextra -O2 -Iinclude
set NASM=%MSYS%\mingw64\bin\nasm.exe
set LD=%MSYS%\mingw64\bin\ld.exe
set OBJCOPY=%MSYS%\mingw64\bin\objcopy.exe

echo ====================================
echo  CosmosOS Build System
echo ====================================
echo.

REM Clean old build
echo [1/6] Cleaning...
del /Q *.o kernel\*.o drivers\*.o mm\*.o fs\*.o shell\*.o gui\*.o net\*.o lib\*.o cosmos.bin cosmos.elf cosmosos.img boot\boot.bin 2>nul

REM Assemble NASM files (COFF/win32, symbols match TDM-GCC underscore convention)
echo [2/6] Assembling...
%NASM% -f win32 kernel\entry.asm -o kernel\entry.o
if errorlevel 1 goto :error
%NASM% -f win32 kernel\gdt_flush.asm -o kernel\gdt_flush.o
if errorlevel 1 goto :error
%NASM% -f win32 kernel\idt_flush.asm -o kernel\idt_flush.o
if errorlevel 1 goto :error
%NASM% -f win32 kernel\isr.asm -o kernel\isr.o
if errorlevel 1 goto :error
%NASM% -f win32 kernel\context.asm -o kernel\context.o
if errorlevel 1 goto :error

REM Compile C files
echo [3/6] Compiling...
%CC% -c kernel\main.c -o kernel\main.o
if errorlevel 1 goto :error
%CC% -c kernel\gdt.c -o kernel\gdt.o
if errorlevel 1 goto :error
%CC% -c kernel\idt.c -o kernel\idt.o
if errorlevel 1 goto :error
%CC% -c kernel\process.c -o kernel\process.o
if errorlevel 1 goto :error
%CC% -c drivers\vga.c -o drivers\vga.o
if errorlevel 1 goto :error
%CC% -c drivers\pic.c -o drivers\pic.o
if errorlevel 1 goto :error
%CC% -c drivers\keyboard.c -o drivers\keyboard.o
if errorlevel 1 goto :error
%CC% -c drivers\timer.c -o drivers\timer.o
if errorlevel 1 goto :error
%CC% -c mm\memory.c -o mm\memory.o
if errorlevel 1 goto :error
%CC% -c fs\fs.c -o fs\fs.o
if errorlevel 1 goto :error
%CC% -c shell\shell.c -o shell\shell.o
if errorlevel 1 goto :error
%CC% -c gui\gui.c -o gui\gui.o
if errorlevel 1 goto :error
%CC% -c net\net.c -o net\net.o
if errorlevel 1 goto :error
%CC% -c lib\string.c -o lib\string.o
if errorlevel 1 goto :error
%CC% -c lib\stdio.c -o lib\stdio.o
if errorlevel 1 goto :error
%CC% -c lib\div64.c -o lib\div64.o
if errorlevel 1 goto :error

REM Link
echo [4/6] Linking...
set OBJS=kernel\entry.o kernel\main.o kernel\gdt.o kernel\gdt_flush.o kernel\idt.o kernel\idt_flush.o kernel\isr.o kernel\process.o kernel\context.o
set OBJS=%OBJS% drivers\vga.o drivers\pic.o drivers\keyboard.o drivers\timer.o
set OBJS=%OBJS% mm\memory.o fs\fs.o shell\shell.o gui\gui.o net\net.o lib\string.o lib\stdio.o lib\div64.o

%LD% -m i386pe -T scripts\linker_pe.ld -o cosmos.elf %OBJS%
if errorlevel 1 goto :error
%OBJCOPY% -O binary cosmos.elf cosmos.bin
if errorlevel 1 goto :error

REM Create disk image
echo [5/6] Creating disk image...
%NASM% -f bin boot\boot.asm -o boot\boot.bin
if errorlevel 1 goto :error

copy /Y boot\boot.bin cosmosos.img >nul
copy /B cosmosos.img + cosmos.bin cosmosos.img >nul

REM Pad to 1.44MB
echo [6/6] Padding image...
for %%A in (cosmosos.img) do set SIZE=%%~zA
if %SIZE% LSS 1474560 (
    powershell -Command "$pad = 1474560 - %SIZE%; [System.IO.File]::WriteAllBytes('pad.tmp', [byte[]]::new($pad))" 2>nul
    copy /B cosmosos.img + pad.tmp cosmosos.img >nul
    del pad.tmp 2>nul
)

echo.
echo ====================================
echo  Build successful!
echo ====================================
echo  Kernel:   cosmos.bin
echo  Disk:     cosmosos.img
echo.
echo  To run:   run_cosmosos.bat
echo  Or:       qemu-system-i386 -drive format=raw,file=cosmosos.img -m 128M
echo ====================================
goto :eof

:error
echo.
echo BUILD FAILED!
exit /b 1