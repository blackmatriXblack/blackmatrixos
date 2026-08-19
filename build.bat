@echo off
REM CosmosOS Build Script for Windows
REM Requires MSYS2 installed at C:\msys64

set MSYS=C:\msys64
set CC=%MSYS%\mingw64\bin\gcc.exe
set LD=%MSYS%\mingw64\bin\ld.exe
set NASM=%MSYS%\mingw64\bin\nasm.exe
set OBJCOPY=%MSYS%\mingw64\bin\objcopy.exe
set OBJDUMP=%MSYS%\mingw64\bin\objdump.exe

set CFLAGS=-m32 -ffreestanding -fno-pie -fno-stack-protector -fno-builtin -nostdlib -nostdinc -Wall -Wextra -O2 -Iinclude
set NASMFLAGS=-f elf32
set LDFLAGS=-m elf_i386 -T scripts\linker.ld --oformat binary

echo ====================================
echo  CosmosOS Build System
echo ====================================
echo.

REM Clean old build
echo [1/5] Cleaning...
del /Q *.o kernel\*.o drivers\*.o mm\*.o fs\*.o shell\*.o gui\*.o net\*.o lib\*.o cosmos.bin cosmos.elf cosmosos.img boot\boot.bin 2>nul

REM Assemble NASM files
echo [2/5] Assembling...
%NASM% %NASMFLAGS% kernel\entry.asm -o kernel\entry.o
if errorlevel 1 goto :error
%NASM% %NASMFLAGS% kernel\gdt_flush.asm -o kernel\gdt_flush.o
if errorlevel 1 goto :error
%NASM% %NASMFLAGS% kernel\idt_flush.asm -o kernel\idt_flush.o
if errorlevel 1 goto :error
%NASM% %NASMFLAGS% kernel\isr.asm -o kernel\isr.o
if errorlevel 1 goto :error
%NASM% %NASMFLAGS% kernel\context.asm -o kernel\context.o
if errorlevel 1 goto :error

REM Compile C files
echo [3/5] Compiling...
for %%f in (kernel\main.c kernel\gdt.c kernel\idt.c kernel\process.c) do (
    %CC% %CFLAGS% -c %%f -o %%~dpnf.o
    if errorlevel 1 goto :error
)
for %%f in (drivers\vga.c drivers\pic.c drivers\keyboard.c drivers\timer.c) do (
    %CC% %CFLAGS% -c %%f -o %%~dpnf.o
    if errorlevel 1 goto :error
)
for %%f in (mm\memory.c fs\fs.c shell\shell.c gui\gui.c net\net.c lib\string.c lib\stdio.c) do (
    %CC% %CFLAGS% -c %%f -o %%~dpnf.o
    if errorlevel 1 goto :error
)

REM Link
echo [4/5] Linking...
set OBJS=kernel\entry.o kernel\main.o kernel\gdt.o kernel\gdt_flush.o kernel\idt.o kernel\idt_flush.o kernel\isr.o kernel\process.o kernel\context.o
set OBJS=%OBJS% drivers\vga.o drivers\pic.o drivers\keyboard.o drivers\timer.o
set OBJS=%OBJS% mm\memory.o fs\fs.o shell\shell.o gui\gui.o net\net.o lib\string.o lib\stdio.o

%LD% %LDFLAGS% -o cosmos.elf %OBJS%
if errorlevel 1 goto :error
%OBJCOPY% -O binary cosmos.elf cosmos.bin
if errorlevel 1 goto :error

REM Create disk image
echo [5/5] Creating disk image...
%NASM% -f bin boot\boot.asm -o boot\boot.bin
if errorlevel 1 goto :error

copy /Y boot\boot.bin cosmosos.img >nul
copy /B cosmosos.img + cosmos.bin cosmosos.img >nul

REM Pad to 1.44MB
for %%A in (cosmosos.img) do set SIZE=%%~zA
if %SIZE% LSS 1474560 (
    fsutil file createnew pad.tmp 0 >nul
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
echo  To run:   qemu-system-i386 -drive format=raw,file=cosmosos.img -m 128M
echo ====================================
goto :eof

:error
echo.
echo BUILD FAILED!
exit /b 1
