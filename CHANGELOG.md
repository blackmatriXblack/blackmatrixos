# Changelog

All notable changes to the BlackMatrixOS project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to the README conventions of the repository.

---

## [Unreleased]

### Added
- Nothing yet. Planned items live in the README "Future Roadmap" section:
  - Preemptive multitasking
  - ELF binary loading
  - FAT32 filesystem support
  - AHCI/SATA driver
  - Sound Blaster driver
  - VESA VBE graphics modes
  - Package manager
  - Real HTTP client

---

## [2026-08-16] - Full C Kernel (Latest Build)

Latest compiled state of `cosmosos.img` (build artifacts regenerated: `cosmos.bin`,
`cosmos.elf`, `boot.bin`, and all `.o` object files).

### Added
- Full C kernel (`kernel/main.c`) with all drivers and subsystems
- Interactive shell with 340+ built-in commands, including:
  - File operations, text processing, and scripting (`if/elif/else/fi`,
    `for/while/until/done`, `case`, pipes, redirects, `at` job scheduler)
  - Line-based text editor (`edit`)
  - Games (`2048`, `snake`, `tictactoe` with AI, `hangman`, `quiz`)
  - Math, cipher, animation, and system info commands
- GUI desktop (VGA Mode 13h, 320x200, 256 colors):
  - Window management with drag support
  - Desktop icons, taskbar with Start button and clock
  - Widget toolkit and built-in apps (Calculator, Task Manager, System Monitor, Terminal)
- RAM filesystem with directory support (512 files, 4096 blocks)
- Network stack: Ethernet, ARP, IP, ICMP, UDP, TCP
- Process management and round-robin scheduling
- Physical memory manager with paging and kernel heap allocator

### Fixed
- N/A (no issue tracker in this repository)

### Changed
- Bootloader uses BIOS EDD reads to load kernels up to 256 KB
- Single disk image now contains bootloader + full C kernel

---

## [2026-03-29] - Build Milestone (README Reference Date)

Credits/build date recorded in the README.

### Added
- Three kernel variants:
  1. **Full C Kernel** (`kernel/main.c`) — complete OS with filesystem, process
     management, network stack, and GUI
  2. **Browser Edition** (`kernel/kernel.asm`) — pure assembly standalone kernel
     with built-in browser, matrix rain effect, and 10+ commands
  3. **Single-File OS** (`blackmatrix.asm`) — complete bootloader + kernel in one
     file, boots from a floppy disk image, no external dependencies
- Windows and Linux build scripts:
  - `build_windows.bat` (NASM + TDM-GCC, PE linker script)
  - `build.bat` (MSYS2/MinGW)
  - `Makefile` (Linux, GNU toolchain)
  - `run_cosmosos.bat` (QEMU launcher)
- Driver layer: VGA (text + Mode 13h), PS/2 keyboard, 8259 PIC, PIT timer
- GDT/IDT setup, ISR/IRQ handlers, context switching assembly stubs
- Documentation: comprehensive README with architecture, memory map, shell
  reference, and development guide

### Changed
- N/A (initial documented milestone)

---

## [Initial] - Project Inception

### Added
- Bootloader (`boot/boot.asm`, 512-byte stage 1)
- 32-bit protected mode kernel entry
- Basic VGA text mode driver and printf implementation
- QEMU/Bochs bootable floppy disk image

---

## Legend

- **Added** — new features
- **Changed** — changes to existing functionality
- **Fixed** — bug fixes
- **Removed** — removed features

---

*"Welcome to the Matrix. There is no spoon."*