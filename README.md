# BlackMatrixOS

**The Matrix Operating System** - A minimalist x86 operating system with retro aesthetics, command-line shell, and graphical user interface.

```
  ____  _            _    _   _      _   _         ___  ____
 | __ )| | __ _  ___| | _| | | | ___| |_(_) ___   / _ \/ ___|
 |  _ \| |/ _` |/ __| |/ / |_| |/ _ \ __| |/ __| | | | \___ \
 | |_) | | (_| | (__|   <|  _  |  __/ |_| | (__  | |_| |___) |
 |____/|_|\__,_|\___|_|\_\_| |_|\___|\__|_|\___|  \___/|____/
```

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Architecture](#architecture)
- [Building](#building)
- [Running](#running)
- [Shell Commands Reference](#shell-commands-reference)
- [GUI Mode](#gui-mode)
- [Browser Mode](#browser-mode)
- [File System](#file-system)
- [Network Stack](#network-stack)
- [Project Structure](#project-structure)
- [Kernel Variants](#kernel-variants)
- [VGA Graphics](#vga-graphics)
- [Keyboard Driver](#keyboard-driver)
- [Memory Management](#memory-management)
- [Process Management](#process-management)
- [Development](#development)
- [License](#license)

---

## Overview

BlackMatrixOS is a hobby operating system written in x86 assembly and C. It runs on bare metal (or in QEMU/Bochs) and provides:

- **Text-mode shell** with 340+ built-in commands
- **GUI desktop** using VGA Mode 13h (320x200, 256 colors)
- **Built-in web browser** with URL input
- **RAM filesystem** with full directory support
- **Network stack** (ARP, IP, ICMP, UDP, TCP)
- **Process management** and scheduling
- **Physical memory manager** with paging
- **Kernel heap allocator**

The OS boots from a single floppy disk image and runs entirely in 32-bit protected mode.

---

## Features

### Core System
- x86 32-bit Protected Mode kernel
- Monolithic architecture
- GDT (Global Descriptor Table) with ring 0 protection
- IDT (Interrupt Descriptor Table) with ISR/IRQ handlers
- 8259 PIC remapping
- PIT timer at 100Hz for scheduling
- A20 line enablement
- Multiboot-compatible boot sequence

### Shell (340+ Commands)
The interactive shell supports Unix-like commands organized in categories:

**File Operations:** `ls`, `cd`, `pwd`, `cat`, `mkdir`, `rm`, `touch`, `cp`, `mv`, `find`, `tree`, `stat`, `ln`, `chmod`, `truncate`

**Text Processing:** `head`, `tail`, `cat`, `grep`, `wc`, `sort`, `uniq`, `rev`, `tac`, `nl`, `cut`, `paste`, `fold`, `expand`, `unexpand`, `tr`, `comm`, `od`, `strings`, `hexdump`, `tee`

**Scripting:** `if/elif/else/fi`, `for/while/until/done`, `case`, `break`, `continue`, `source`, `eval`, `read`, `test`, `let`, variable assignment (`NAME=value`) with `$VAR` expansion, pipes (`cmd1 | cmd2`), redirects (`>` and `>>`), and the `at` job scheduler

**Editor:** `edit <file>` (line-based: a=append, i=insert, d=delete, r=replace, s=substitute, f=find, w=save, q=quit)

**Games:** `2048`, `snake`, `tictactoe` (with AI), `hangman`, `quiz`, `dice`, `coin`, `rps`

**Math:** `calc`, `expr`, `sci` (expression parser with functions), `stats` (count/min/max/range/sum/mean/median/mode/variance/stddev), `primes`, `fib`, `roman`, `angles` (deg/rad/grad), `factor`, `seq`, `hex`, `dec`, `bitcalc`

**Ciphers:** `caesar` (with `--brute`), `vigenere`, `atbash`, `urlencode`, `urldecode`, `hash` (djb2/sdbm/fnv1a/crc16), `rot13`, `morse`

**Animations:** `donut` (3D ASCII), `stars`, `clock`, `hollywood`, `progress`, `matrix`

**System Info:** `ver`, `uname`, `uptime`, `mem`, `free`, `lscpu`, `dmesg`, `whoami`, `hostname`, `id`, `date`, `df`, `ps`, `top`, `neofetch`, `bench` (CPU benchmark), `lspci` (PCI enumeration), `netstat`

### GUI Desktop
- VGA Mode 13h (320x200, 256 colors)
- Window management with drag support
- Desktop icons (Files, Terminal, Settings)
- Taskbar with Start button and clock
- Widget toolkit: labels, buttons, textboxes, checkboxes, progress bars
- Built-in applications: Calculator, Task Manager, System Monitor, Terminal

### Browser
- URL bar with keyboard input
- Navigation with Enter key
- Page loading simulation
- Quick access links (Wiki, News, Tech)

---

## Architecture

```
+---------------------------+
|       User Space          |
|  Shell / GUI / Browser    |
+---------------------------+
|       System Calls        |
+---------------------------+
|     Kernel Services       |
|  Process | Memory | FS    |
|  Network | Timer  | GUI   |
+---------------------------+
|       Drivers             |
|  VGA | Keyboard | PIC     |
|  Timer | Network          |
+---------------------------+
|     Hardware (x86)        |
|  CPU | VGA | Keyboard     |
|  PIC | PIT | NIC          |
+---------------------------+
```

### Memory Map
| Address Range | Description |
|--------------|-------------|
| `0x00000000 - 0x000003FF` | Real Mode IVT |
| `0x00007C00 - 0x00007DFF` | Bootloader (Stage 1) |
| `0x00008000 - 0x0000FFFF` | Bootloader Stack |
| `0x00010000 - 0x0001FFFF` | Kernel Entry |
| `0x00020000 - 0x0007FFFF` | Kernel Heap |
| `0x00080000 - 0x0009FFFF` | Loaded Kernel (ASM) |
| `0x000A0000 - 0x000AFFFF` | VGA Graphics (Mode 13h) |
| `0x000B8000 - 0x000B8F9F` | VGA Text Mode |
| `0x00100000 - 0x007FFFFF` | Physical Memory (128 MB) |

---

## Building

### Prerequisites
- **Windows:** TDM-GCC (32-bit capable), MSYS2 with NASM/binutils, QEMU (optional)
- **Linux:** NASM, GCC (i686-elf-gcc), GNU Make, QEMU

### Build on Windows

```cmd
cd blackmatrix
build_windows.bat
```

This script:
1. Assembles `boot/boot.asm` (bootloader) and the kernel assembly stubs with NASM (win32/COFF output)
2. Compiles all C source files with TDM-GCC (`-m32 -ffreestanding`)
3. Links with the custom PE linker script and converts to a flat binary
4. Creates the 1.44 MB disk image (bootloader + kernel)

The bootloader uses BIOS EDD reads to load kernels up to 256 KB.

### Build on Linux (with Makefile)

```bash
cd blackmatrix
make
```

### Build Single-File Version

```bash
nasm -f bin blackmatrix.asm -o blackmatrix.img
```

### Build Browser Edition

```bash
nasm -f bin kernel/kernel.asm -o kernel_browser.img
```

---

## Running

### Using QEMU

```bash
# Run the full C kernel
qemu-system-i386 -drive format=raw,file=cosmosos.img

# Run the single-file version
qemu-system-i386 -drive format=raw,file=blackmatrix.img

# Run with network support
qemu-system-i386 -drive format=raw,file=cosmosos.img -net nic -net user
```

### Using the Batch Script (Windows)

```cmd
run_cosmosos.bat
```

### Using Bochs

```bash
bochs -f bochsrc.bxrc
```

---

## Shell Commands Reference

### File Operations

| Command | Usage | Description |
|---------|-------|-------------|
| `ls` | `ls [dir]` | List directory contents |
| `dir` | `dir` | Alias for `ls` |
| `cd` | `cd <path>` | Change directory |
| `pwd` | `pwd` | Print working directory |
| `cat` | `cat <file>` | Display file contents |
| `mkdir` | `mkdir <dir>` | Create directory |
| `rm` | `rm <file>` | Remove file |
| `touch` | `touch <file>` | Create empty file |
| `cp` | `cp <src> <dst>` | Copy file |
| `mv` | `mv <src> <dst>` | Move/rename file |
| `find` | `find [path]` | Find files |
| `tree` | `tree [path]` | Show directory tree |
| `stat` | `stat <file>` | Display file information |
| `ln` | `ln <target> <link>` | Create hard link |
| `chmod` | `chmod <mode> <file>` | Change permissions |
| `truncate` | `truncate -s <size> <file>` | Resize file |

### Text Processing

| Command | Usage | Description |
|---------|-------|-------------|
| `head` | `head [-n N] <file>` | Show first N lines |
| `tail` | `tail [-n N] <file>` | Show last N lines |
| `cat` | `cat <file>` | Display entire file |
| `grep` | `grep <pattern> <file>` | Search for pattern |
| `wc` | `wc <file>` | Count lines, words, chars |
| `sort` | `sort <file>` | Sort lines alphabetically |
| `uniq` | `uniq <file>` | Remove duplicate adjacent lines |
| `rev` | `rev <file>` | Reverse each line |
| `tac` | `tac <file>` | Reverse line order |
| `nl` | `nl <file>` | Number lines |
| `cut` | `cut -f<N> [-d<delim>] <file>` | Extract columns |
| `paste` | `paste <file1> <file2>` | Merge files line by line |
| `fold` | `fold [-w N] <file>` | Wrap lines at width |
| `expand` | `expand <file>` | Convert tabs to spaces |
| `unexpand` | `unexpand <file>` | Convert spaces to tabs |
| `tr` | `tr <set1> <set2>` | Translate characters |
| `comm` | `comm <file1> <file2>` | Compare sorted files |
| `od` | `od <file>` | Octal dump |
| `strings` | `strings <file>` | Extract printable strings |
| `hexdump` | `hexdump <file>` | Hexadecimal dump |
| `tee` | `tee <file>` | Write to file and stdout |

### System Information

| Command | Usage | Description |
|---------|-------|-------------|
| `ver` | `ver` | Show OS version |
| `uname` | `uname [-a]` | System name and info |
| `uptime` | `uptime` | System uptime |
| `mem` | `mem` | Memory usage |
| `free` | `free` | Free/used memory table |
| `lscpu` | `lscpu` | CPU information |
| `dmesg` | `dmesg` | Kernel boot messages |
| `whoami` | `whoami` | Current username |
| `hostname` | `hostname` | System hostname |
| `id` | `id` | User and group IDs |
| `date` | `date` | Current date/time |
| `df` | `df` | Disk usage |
| `ps` | `ps` | List processes |
| `top` | `top` | Process monitor |
| `neofetch` | `neofetch` | System info display |

### Process Management

| Command | Usage | Description |
|---------|-------|-------------|
| `ps` | `ps` | List running processes |
| `kill` | `kill <pid>` | Kill process by PID |
| `killall` | `killall <name>` | Kill process by name |
| `pgrep` | `pgrep <name>` | Find process by name |
| `top` | `top` | Real-time process monitor |

### Math & Computation

| Command | Usage | Description |
|---------|-------|-------------|
| `calc` | `calc <a> <op> <b>` | Calculator (+, -, *, /, %) |
| `expr` | `expr <a> <op> <b>` | Expression evaluator |
| `factor` | `factor <n>` | Prime factorization |
| `seq` | `seq [start [step]] end` | Generate number sequence |
| `hex` | `hex <decimal>` | Convert decimal to hex |
| `dec` | `dec <hex>` | Convert hex to decimal |
| `bitcalc` | `bitcalc <a> <op> <b>` | Bitwise calculator (AND/OR/XOR/SHL/SHR) |

### Network

| Command | Usage | Description |
|---------|-------|-------------|
| `ifconfig` | `ifconfig` | Network interface info |
| `ping` | `ping <ip>` | Ping a host (simulated) |
| `netstat` | `netstat` | Network connections |

### File Checksums

| Command | Usage | Description |
|---------|-------|-------------|
| `cksum` | `cksum <file>` | CRC32 checksum |
| `sum` | `sum <file>` | BSD checksum |

### Shell & Environment

| Command | Usage | Description |
|---------|-------|-------------|
| `help` | `help` | Show all commands |
| `history` | `history` | Command history |
| `env` | `env` | Environment variables |
| `export` | `export NAME=VALUE` | Set variable |
| `alias` | `alias NAME=VALUE` | Create alias |
| `set` | `set` | Show/set variables |
| `read` | `read [var]` | Read user input |
| `which` | `which <cmd>` | Locate command |
| `test` | `test <a> <op> <b>` | Test conditions |
| `printf` | `printf <fmt> [args]` | Formatted output |
| `basename` | `basename <path>` | Strip directory |
| `dirname` | `dirname <path>` | Strip filename |
| `cmp` | `cmp <file1> <file2>` | Compare files byte by byte |

### Control Flow

| Command | Usage | Description |
|---------|-------|-------------|
| `true` | `true` | Return success (exit 0) |
| `false` | `false` | Return failure (exit 1) |
| `yes` | `yes [text]` | Print text repeatedly |
| `sleep` | `sleep <seconds>` | Delay execution |

### Fun

| Command | Usage | Description |
|---------|-------|-------------|
| `cowsay` | `cowsay <message>` | ASCII cow says message |
| `matrix` | `matrix` | Matrix rain effect |
| `color` | `color` | Show color palette |
| `banner` | `banner` | Large text banner |

### System Control

| Command | Usage | Description |
|---------|-------|-------------|
| `gui` | `gui` | Enter graphical mode (or press F1) |
| `reboot` | `reboot` | Restart the system |
| `clear` | `clear` | Clear screen |
| `cls` | `cls` | Alias for clear |

---

## GUI Mode

### Entering GUI Mode

- Press **F1** from the shell
- Type `gui` and press Enter

### Exiting GUI Mode

- Press **ESC** to return to the text shell

### Desktop Features

The GUI desktop provides:

1. **Desktop Background** - Gradient color scheme
2. **Desktop Icons** - Clickable icons for Files, Terminal, Settings
3. **Taskbar** - Bottom bar with Start button, window list, and clock
4. **Window System** - Draggable windows with title bars and close buttons
5. **Widget Toolkit** - Labels, buttons, textboxes, checkboxes, progress bars

### Built-in GUI Applications

- **System Monitor** - CPU and memory usage with progress bars
- **Calculator** - Numeric keypad with arithmetic operations
- **Task Manager** - Process list viewer
- **Terminal** - Command-line window (GUI wrapper)

### GUI Drawing API

The GUI system provides a full drawing API:
- `gui_draw_pixel(x, y, color)` - Draw a pixel
- `gui_draw_rect(x, y, w, h, color)` - Draw rectangle outline
- `gui_draw_filled_rect(x, y, w, h, color)` - Draw filled rectangle
- `gui_draw_line(x0, y0, x1, y1, color)` - Bresenham line
- `gui_draw_char(x, y, c, fg, bg)` - Draw character with 8x8 font
- `gui_draw_string(x, y, text, fg, bg)` - Draw text string
- `gui_draw_button(x, y, w, h, text, pressed)` - Draw button

---

## Browser Mode

### Opening the Browser

- Press **F1** or type `gui` in the shell (Browser Edition)

### Browser Features

- **URL Bar** - Type a URL and press Enter to navigate
- **Backspace** - Edit the URL
- **ESC** - Exit browser and return to shell
- **Quick Access Links** - Wiki, News, Tech
- **Page Display** - Shows loaded page content

### Browser Keyboard Controls

| Key | Action |
|-----|--------|
| Any character | Type into URL bar |
| Backspace | Delete last character |
| Enter | Navigate to URL |
| ESC | Close browser |

---

## File System

### RAM Filesystem (RAMFS)

BlackMatrixOS uses an in-memory filesystem:

- **Max files:** 512
- **Max file size:** Configurable per block
- **Block size:** 512 bytes
- **Total blocks:** 4096
- **Supported types:** Regular files, directories, symlinks, devices

### Default Directory Structure

```
/
├── bin/          # Executables
├── etc/          # Configuration
│   └── hostname  # System hostname
├── home/
│   └── user/     # User home directory
├── tmp/          # Temporary files
├── var/          # Variable data
├── usr/          # User programs
├── dev/          # Device files
└── proc/         # Process information
```

### File Operations

The filesystem supports:
- Create, read, write, delete files
- Create, list, remove directories
- Hard links and symbolic links
- File permissions (read, write, execute)
- File stat with size, timestamps, ownership

---

## Network Stack

### Protocol Support

The network stack implements:

1. **Ethernet** (Layer 2) - Frame construction and parsing
2. **ARP** - Address Resolution Protocol
3. **IP** (Layer 3) - IPv4 packet handling
4. **ICMP** - Ping (echo request/reply)
5. **UDP** - User Datagram Protocol
6. **TCP** - Transmission Control Protocol (basic)

### Network Configuration (QEMU default)

```
Interface: eth0
IP Address:  10.0.2.15
Netmask:     255.255.255.0
Gateway:     10.0.2.2
MAC Address: 52:54:00:12:34:56
```

---

## Project Structure

```
blackmatrix/
├── blackmatrix.asm          # Single-file OS (bootloader + kernel)
├── Makefile                 # GNU Make build system
├── build.bat                # MSYS2/MinGW build script
├── build_windows.bat        # Alternative Windows build
├── run_cosmosos.bat         # QEMU launcher
├── cosmosos.img             # Compiled disk image
├── blackmatrix.img          # Single-file OS image
│
├── boot/
│   └── boot.asm             # Stage 1 bootloader (512 bytes)
│
├── kernel/
│   ├── entry.asm            # 32-bit entry point
│   ├── kernel.asm           # Standalone assembly kernel (Browser Edition)
│   ├── main.c               # Main C kernel (shell + GUI)
│   ├── gdt.c                # Global Descriptor Table
│   ├── gdt_flush.asm        # GDT flush routine
│   ├── idt.c                # Interrupt Descriptor Table
│   ├── idt_flush.asm        # IDT flush routine
│   ├── isr.asm              # Interrupt Service Routines
│   ├── context.asm          # Context switching
│   └── process.c            # Process management
│
├── drivers/
│   ├── keyboard.c           # PS/2 keyboard driver (IRQ1)
│   ├── vga.c                # VGA text + Mode 13h graphics
│   ├── pic.c                # 8259 PIC controller
│   └── timer.c              # PIT timer driver
│
├── include/
│   ├── types.h              # Basic types (uint8_t, size_t, etc.)
│   ├── stdarg.h             # Variable arguments
│   ├── io.h                 # I/O port operations
│   ├── string.h             # String functions
│   ├── stdio.h              # printf/kprintf
│   ├── keyboard.h           # Keyboard driver API
│   ├── vga.h                # VGA driver API
│   ├── idt.h                # IDT definitions
│   ├── gdt.h                # GDT definitions
│   ├── pic.h                # PIC definitions
│   ├── timer.h              # Timer API
│   ├── memory.h             # Memory management API
│   ├── process.h            # Process management API
│   ├── fs.h                 # Filesystem API
│   ├── gui.h                # GUI framework API
│   ├── shell.h              # Shell API
│   └── net.h                # Network stack API
│
├── lib/
│   ├── string.c             # String library functions
│   ├── stdio.c              # printf implementation
│   └── div64.c              # 64-bit division helpers (libgcc replacement)
│
├── mm/
│   └── memory.c             # PMM, paging, heap allocator
│
├── fs/
│   └── fs.c                 # RAM filesystem implementation
│
├── shell/
│   └── shell.c              # Interactive shell (340+ commands, scripting engine)
│
├── gui/
│   └── gui.c                # Window manager + GUI framework
│
├── net/
│   └── net.c                # Network stack (ARP/IP/ICMP/UDP/TCP)
│
└── scripts/
    ├── linker.ld            # ELF linker script
    └── linker_pe.ld         # PE linker script
```

---

## Kernel Variants

BlackMatrixOS includes three kernel variants:

### 1. Full C Kernel (`kernel/main.c` + all drivers)
- Complete OS with filesystem, process management, network stack
- 340+ shell commands including scripting, editor, games, math, ciphers and animations
- GUI desktop with window manager
- Requires all source files to compile

### 2. Browser Edition (`kernel/kernel.asm`)
- Pure assembly standalone kernel
- Built-in browser with URL input
- Matrix rain effect
- 10+ commands
- Single-file compilation with NASM

### 3. Single-File OS (`blackmatrix.asm`)
- Complete bootloader + kernel in one file
- Boots from floppy disk image
- Command-line shell + GUI
- Matrix rain effect
- No external dependencies

---

## VGA Graphics

### Text Mode (Default)

- **Resolution:** 80x25 characters
- **Framebuffer:** `0xB8000`
- **Colors:** 16 foreground, 16 background
- **Cursor:** Hardware cursor via CRTC registers

### Graphics Mode (Mode 13h)

- **Resolution:** 320x200 pixels
- **Colors:** 256 (8-bit palette)
- **Framebuffer:** `0xA0000`
- **Used by:** GUI desktop and browser

### Mode Switching

```
Text → Graphics:  VGA Mode 13h register programming
Graphics → Text:  INT 10h AH=00h AL=03h (BIOS mode set)
```

### Color Palette (Mode 13h)

| Index | Color |
|-------|-------|
| 0 | Black |
| 1 | Blue |
| 2 | Green |
| 3 | Cyan |
| 4 | Red |
| 5 | Magenta |
| 6 | Brown |
| 7 | Light Gray |
| 8 | Dark Gray |
| 9 | Light Blue |
| 10 | Lime |
| 11 | Light Cyan |
| 12 | Light Red |
| 13 | Pink |
| 14 | Yellow |
| 15 | White |

---

## Keyboard Driver

### PS/2 Keyboard Interface

- **Data Port:** `0x60` (scancode read)
- **Status Port:** `0x64` (buffer status)
- **IRQ:** IRQ1 (ISR 33)

### Scancode Handling

1. Read scancode from port `0x60`
2. Check bit 7 for key release (ignore)
3. Translate via scancode-to-ASCII table
4. Buffer character for shell consumption

### Special Keys

| Key | Scancode | Action |
|-----|----------|--------|
| ESC | `0x01` | Exit/Cancel |
| F1 | `0x3B` | Enter GUI mode |
| Enter | `0x1C` | Execute command |
| Backspace | `0x0E` | Delete character |
| Caps Lock | `0x3A` | Toggle caps |

---

## Memory Management

### Physical Memory Manager (PMM)

- **Bitmap-based** frame allocator
- **Frame size:** 4 KB
- **Total frames:** 32,768 (128 MB)
- Supports frame allocation and deallocation

### Paging

- **Page directory** with 1024 entries
- **Page tables** for 4 MB each
- Identity mapping for first 4 MB
- Kernel mapped to higher half (optional)

### Kernel Heap

- **Dynamic allocator** for kernel objects
- Supports `kmalloc()` and `kfree()`
- Best-fit allocation strategy
- Automatic coalescing of free blocks

---

## Process Management

### Process Model

- **PCB (Process Control Block)** for each process
- **PID** assignment (sequential)
- **State:** Running, Ready, Blocked, Zombie
- **Round-robin** scheduling
- **Context switching** via assembly stubs

### Process Operations

- `process_create()` - Create new process
- `process_kill()` - Terminate process
- `process_list()` - List all processes
- `process_switch()` - Context switch

---

## Development

### Adding a New Shell Command

1. Add function declaration to `include/shell.h`:
```c
int cmd_mycommand(int argc, char** argv);
```

2. Implement in `shell/shell.c`:
```c
int cmd_mycommand(int argc, char** argv) {
    kprintf("Hello from mycommand!\n");
    return 0;
}
```

3. Register in the command table:
```c
{"mycommand", cmd_mycommand, "My custom command"},
```

4. Rebuild and run.

### Adding a GUI Application

1. Include `gui.h` in your source
2. Create a window with `gui_create_window()`
3. Add widgets with `gui_add_*()` functions
4. Register as desktop icon or menu item

### Debugging

- Use QEMU monitor: `Ctrl+Alt+2`
- GDB remote debug: `qemu -s -S`
- Connect GDB: `target remote localhost:1234`

---

## Keyboard Shortcuts

| Shortcut | Context | Action |
|----------|---------|--------|
| F1 | Shell | Enter GUI/Browser mode |
| ESC | GUI | Return to shell |
| ESC | Browser | Close browser |
| ESC | Matrix | Stop matrix rain |
| Ctrl+L | Shell | Clear screen |
| Enter | Browser | Navigate to URL |
| Backspace | Shell/Browser | Delete character |

---

## Known Limitations

- No multi-user authentication
- No persistent storage (RAM only)
- No preemptive multitasking (cooperative)
- No hardware-accelerated graphics
- No sound support
- No USB support
- Limited TCP implementation
- Browser is simulated (no real HTTP)
- Variable expansion happens when a line is read, so single-line `for i in a b; do echo $i; done` cannot use `$i` — use the multi-line form instead
- Pipes support a single level (`cmd1 | cmd2`); `cmd2` receives the output as a file argument
- `at` jobs only run when the shell is idle at the prompt, not during blocking commands

---

## Future Roadmap

- [ ] Preemptive multitasking
- [ ] ELF binary loading
- [ ] FAT32 filesystem support
- [ ] AHCI/SATA driver
- [ ] Sound Blaster driver
- [ ] VESA VBE graphics modes
- [ ] Package manager
- [ ] Real HTTP client

---

## Credits

- **Developer:** BlackMatrix Project
- **Architecture:** x86 (i686) 32-bit
- **License:** MIT
- **Build Date:** 2026-03-29

---

## Contact

- **GitHub:** [BlackMatrixOS Repository]
- **Issues:** Report bugs and feature requests

---

*"Welcome to the Matrix. There is no spoon."*
