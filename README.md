# BlackMatrixOS Technical Reference Manual
**Version:** 1.0.0  
**Architecture:** x86 (16-bit Real Mode / 32-bit Protected Mode)  
**Author:** cagent (blackmatriXblack)  
**Build Date:** 2026-03-29  
**File Format:** NASM Assembly Source (`blackmatrix.asm`)

---

## 1. Executive Summary

**BlackMatrixOS** is a monolithic, single-file operating system kernel and bootloader designed for the x86 architecture. It implements a minimalistic yet functional environment featuring a dual-mode interface: a text-based Command Line Interface (CLI) and a primitive Graphical User Interface (GUI) utilizing VGA Mode 13h.

The system is self-contained within a single assembly file, handling the entire boot process from the Master Boot Record (MBR) stage, transitioning through the Global Descriptor Table (GDT setup, entering protected mode, and executing a 32-bit kernel. The kernel provides basic hardware abstraction for VGA text/framebuffer memory, PS/2 keyboard input, and rudimentary PCI device enumeration simulation.

This document serves as a comprehensive technical deep-dive into the memory layout, execution flow, hardware interaction mechanisms, and software architecture of BlackMatrixOS.

---

## 2. System Architecture & Memory Map

### 2.1. Boot Process Overview
The OS follows a standard two-stage boot process, though both stages are linked into a single binary image:
1.  **Stage 1 (Bootloader):** Executed by the BIOS at physical address `0x7C00`. It operates in **Real Mode (16-bit)**. Its primary responsibilities are disk I/O to load the kernel, enabling the A20 line, setting up the GDT, and switching the CPU to Protected Mode.
2.  **Stage 2 (Kernel):** Loaded at physical address `0x80000` (Segment `0x0800`, Offset `0x0000`). It operates in **Protected Mode (32-bit)**. It initializes the video subsystem, handles user input, and manages the command execution loop.

### 2.2. Memory Layout

| Segment/Address | Size | Description | Access |
| :--- | :--- | :--- | :--- |
| `0x0000:0x7C00` | 512 Bytes | **Bootloader Entry Point**. Contains MBR signature `0xAA55`. | R/X |
| `0x0000:0x7E00` | ~51 KB | **Kernel Image**. Loaded immediately after the bootloader. Contains code, data, and BSS sections. | R/X/RW |
| `0x0800:0x0000` | - | Linear Address `0x80000`. The logical entry point for the 32-bit kernel (`pm_entry`). | R/X |
| `0xB8000` | 4KB | **VGA Text Buffer**. Used for CLI output. 80x25 characters, 2 bytes per char (ASCII + Attribute). | RW |
| `0xA0000` | 64KB | **VGA Framebuffer (Mode 13h)**. Used for GUI rendering. 320x200 pixels, 1 byte per pixel (256 colors). | RW |
| `0x90000` | - | **Stack Pointer Initialization**. The stack grows downwards from this address in Protected Mode. | RW |

---

## 3. Stage 1: The Bootloader (Real Mode)

### 3.1. Initialization Sequence
The entry point `boot_start` performs critical low-level initialization required before transitioning to protected mode.

1.  **Segment Register Reset:**
    ```assembly
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    ```
    All segment registers are zeroed. The stack pointer is set to the bottom of the bootloader space to prevent overwriting code during early execution.

2.  **Disk Load Strategy:**
    The bootloader uses BIOS Interrupt `0x13` (Function `0x02`: Read Sectors From Drive) to load the kernel.
    *   **Destination:** `ES:BX` = `0x0800:0x0000` (Linear `0x80000`).
    *   **Source:** Cylinder `0`, Head `0`, Sector `2` (immediately following the MBR).
    *   **Count:** `100` sectors (approx. 50KB). This defines the maximum size of the kernel.
    *   **Error Handling:** If the Carry Flag (CF) is set after `int 0x13`, execution jumps to `boot_disk_err`, which prints "Boot Error!" via BIOS Teletype (`int 0x10`) and halts.

3.  **A20 Gate Enable:**
    To access memory above 1MB, the A20 line must be enabled. The code uses the "Fast A20" method via Port `0x92`:
    ```assembly
    in al, 0x92
    or al, 2      ; Set bit 1 (A20 enable)
    out 0x92, al
    ```

### 3.2. Transition to Protected Mode

1.  **Global Descriptor Table (GDT) Definition:**
    A minimal GDT is defined inline to facilitate the mode switch.
    *   **Null Descriptor:** Required by Intel spec.
    *   **Code Segment (`gdt_code`):** Base `0x00000000`, Limit `0xFFFFF` (4GB), Granularity `4KB`, Present, Ring 0, Execute/Read. Selector: `0x08`.
    *   **Data Segment (`gdt_data`):** Base `0x00000000`, Limit `0xFFFFF` (4GB), Granularity `4KB`, Present, Ring 0, Read/Write. Selector: `0x10`.

2.  **Loading GDT:**
    ```assembly
    lgdt [gdt_desc]
    ```
    Loads the GDT register with the limit and base address of the local GDT structure.

3.  **Protection Enable (PE) Bit:**
    ```assembly
    mov eax, cr0
    or eax, 1       ; Set PE bit (bit 0)
    mov cr0, eax
    ```
    This instruction activates Protected Mode. However, the CPU still executes in 16-bit mode until a far jump is performed.

4.  **Far Jump:**
    ```assembly
    jmp 0x08:pm_entry
    ```
    This flushes the prefetch queue and loads the CS register with the new Code Segment selector (`0x08`), effectively jumping to the 32-bit entry point `pm_entry`.

---

## 4. Stage 2: The Kernel (Protected Mode)

### 4.1. Environment Setup (`pm_entry`)

Upon entering `pm_entry`, the system is in 32-bit Protected Mode but lacks a valid data segment context.

1.  **Segment Register Reload:**
    ```assembly
    mov ax, 0x10    ; Data Segment Selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ```
    All data segments are pointed to the flat data descriptor (`0x10`).

2.  **Stack Initialization:**
    ```assembly
    mov esp, 0x90000
    ```
    The stack is placed at `0x90000`, safely above the kernel image (`0x80000` + ~50KB) and below the VGA memory regions.

3.  **Hardware Sanitization:**
    *   **Keyboard Flush (`kb_flush`):** Reads from port `0x64` and `0x60` to clear any pending scan codes from the BIOS boot process.
    *   **Screen Clear (`clear_screen`):** Fills the VGA text buffer (`0xB8000`) with `0x0720` (Space character, Light Gray on Black) for 2000 characters (80x25).

### 4.2. Video Subsystem: Text Mode

The CLI operates directly on the VGA Text Buffer at `0xB8000`.

*   **Coordinate System:** The screen is treated as a linear array of 16-bit words.
    *   `COL80 equ 160` (80 columns * 2 bytes/char).
    *   Cursor position is tracked in the global variable `cursor` (DWORD).
*   **String Printing (`print_str_color`):**
    *   Input: `ESI` (source string), `EDI` (destination VRAM offset), `AH` (color attribute).
    *   Logic: Iterates through the null-terminated string. For each character, it writes `AX` (Char + Attr) to `[EDI]` and increments `EDI` by 2.
    *   Newline Handling: Detects ASCII `10` (`\n`). Calculates the next line start by aligning the current offset to the next multiple of `COL80`.
*   **Character Printing (`print_char`):**
    *   Writes a single character at the current `[cursor]` position and advances the cursor by 2 bytes.

### 4.3. Input Subsystem: PS/2 Keyboard

The OS polls the PS/2 controller directly without using interrupts (PIC/IDT is not initialized).

1.  **Wait for Key (`kb_wait_key`):**
    *   Polls Port `0x64` (Status Register). Waits until Bit 0 (Output Buffer Full) is set.
    *   Reads Scan Code from Port `0x60`.
    *   Checks Bit 7 of the scan code. If set, it is a "Make" code (key press); if clear, it is a "Break" code (key release). The code ignores Break codes.

2.  **Scan Code Translation (`scancode_to_ascii`):**
    *   Uses a lookup table `scantbl` indexed by the scan code.
    *   Supports US QWERTY layout for alphanumeric keys, Enter, Backspace, Escape, and Tab.
    *   Special Handling:
        *   `0x3B` (F1): Triggers `enter_gui_mode`.
        *   `0x01` (Esc): Returns ASCII `27`.

### 4.4. Shell Implementation

The shell is a simple REPL (Read-Eval-Print Loop) located in `shell_main`.

1.  **Input Buffering:**
    *   `input_buf`: 64-byte buffer for storing typed commands.
    *   `input_pos`: Tracks the current length of the input.
    *   Supports Backspace (ASCII 8): Decrements `input_pos`, moves cursor back, and prints a space to erase the character visually.

2.  **Command Execution (`execute_cmd`):**
    *   Upon pressing Enter, the input buffer is compared against a hardcoded list of commands using `strcmp` (full match) or `strncmp5` (prefix match for `echo`).
    *   **Command Dispatch Table:**
        *   `help`, `clear`, `ver`, `about`, `gui`, `matrix`, `reboot`, `cowsay`, `whoami`, `hostname`, `uname`, `date`, `lspci`, `color`, `mem`, `uptime`.
    *   **Unknown Command:** If no match is found, prints "Unknown command. Type 'help'." in Red (`0x0C`).

3.  **String Comparison Utilities:**
    *   `strcmp`: Compares two null-terminated strings. Sets ZF=1 if equal.
    *   `strncmp5`: Compares the first 5 bytes. Used specifically for parsing `echo <text>`.

---

## 5. Advanced Features & Effects

### 5.1. Matrix Rain Effect (`do_matrix_rain`)

This is a computationally intensive visual effect rendered in Text Mode.

*   **Data Structure:** `matrix_cols`: An array of 80 DWORDs, one for each column. Each DWORD stores the current row position of the "drop" head. Initialized to negative values (`-20`) to stagger starts.
*   **Rendering Loop:**
    1.  **Iterate Columns:** For each of the 80 columns:
        *   Retrieve the current row index from `matrix_cols`.
        *   If the index is valid (>=0 and <25):
            *   Calculate VRAM address: `VGA + (row * 160) + (col * 2)`.
            *   **Head Character:** Generate a random-ish ASCII character (`al = (col + row) & 0x3F + 0x21`). Set color to White (`0x0F`).
            *   **Trail Character:** Write to the previous row (`row - 1`). Set color to Dark Gray (`0x0A`). This creates the fading trail effect.
        *   Increment the row index in `matrix_cols`.
        *   If the row exceeds 25, reset the column's state to `-20` (off-screen).
    2.  **Delay:** A busy-wait loop (`dec ecx`) controls the frame rate.
    3.  **Exit Condition:** Polls keyboard. If `Esc` is pressed, clears screen and returns to shell.

### 5.2. Graphical User Interface (GUI)

The GUI is implemented by switching the VGA hardware into **Mode 13h** (320x200, 256 colors) via direct port I/O, bypassing BIOS interrupts for performance and control.

#### 5.2.1. Mode Switching (`set_mode13h`)
The function programs the VGA CRTC (Cathode Ray Tube Controller), Sequencer, and Graphics Controller registers directly via ports `0x3C2`, `0x3D4`, `0x3C4`, and `0x3CE`. This ensures a clean transition to linear framebuffer mode at `0xA0000`.

#### 5.2.2. UI Composition (`enter_gui_mode`)
The GUI is statically drawn using `rep stosb` and nested loops. It does not support window management, dragging, or dynamic widgets. It is a "snapshot" UI.

*   **Background:** Filled with Color Index `1` (Blue).
*   **Taskbar:** A horizontal bar at the bottom (Y=180 to 200), filled with Color Index `8` (Dark Gray).
*   **Start Button:** A rectangle at (4, 182) with size 50x16, filled with Color Index `2` (Green).
*   **Window 1 (Terminal Emulator):**
    *   Position: (20, 20), Size: 200x140.
    *   Body: Color Index `7` (Light Gray).
    *   Title Bar: Color Index `9` (Light Blue).
    *   Close Button: Red square (Color Index `4`) at top-right.
*   **Window 2 (System Info):**
    *   Position: (140, 50), Size: 150x100.
    *   Body: Color Index `15` (White).
    *   Title Bar: Color Index `1` (Blue).
*   **Desktop Icons:** Two 32x32 squares (Color Index `8`) positioned on the right side.

#### 5.2.3. Event Loop
The GUI enters a blocking loop (`gui_loop`) polling the PS/2 keyboard. It waits specifically for the `Esc` key (`0x01`). Upon detection, it calls `set_textmode` to restore the VGA text mode and returns control to the CLI.

#### 5.2.4. Text Mode Restoration (`set_textmode`)
Mirrors `set_mode13h` but writes the standard VGA text mode register values (80x25, 16 colors) to the VGA ports. It also toggles the Attribute Controller palette source via port `0x3C0`.

---

## 6. Command Reference

| Command | Description | Implementation Details |
| :--- | :--- | :--- |
| `help` | Displays available commands. | Prints `s_help` string. |
| `clear` | Clears the screen. | Calls `clear_screen`, resets cursor to line 3. |
| `ver` | Shows version info. | Prints `s_ver_out` (v1.0.0, Build date, Arch). |
| `about` | Displays ASCII art logo. | Prints `s_about` containing stylized text. |
| `gui` | Enters Graphical Mode. | Calls `enter_gui_mode`. Also triggered by F1. |
| `matrix` | Starts Matrix Rain effect. | Calls `do_matrix_rain`. Blocks until Esc. |
| `echo [txt]` | Prints text to screen. | Uses `strncmp5` to detect "echo ". Prints remainder of buffer. |
| `color` | Displays color palette. | Loops 0-15, printing '#' with corresponding attribute. |
| `mem` | Shows memory stats. | Static string: 128MB Total, 2MB Used. |
| `uptime` | Shows uptime. | Static string: "boot complete". No real timer implemented. |
| `whoami` | Current user. | Prints "root". |
| `hostname` | System hostname. | Prints "blackmatrix-os". |
| `uname` | System info. | Prints kernel name, node, release, machine. |
| `date` | Current date. | Static string: "2026-03-29 00:00:00 UTC". No RTC read. |
| `lspci` | List PCI devices. | Static simulation of Intel 440FX/PIIX3 chipset. |
| `cowsay` | ASCII Cow. | Prints `s_cowsay` ASCII art. |
| `reboot` | Restarts the system. | Writes `0xFE` to Port `0x64` (Keyboard Controller Reset). |

---

## 7. Data Structures & Constants

### 7.1. Global Variables (BSS/Data)

*   `cursor` (DWORD): Stores the linear VRAM offset of the next character position in Text Mode. Initialized to `VGA + COL80*7`.
*   `color_attr` (BYTE): Default text attribute (`0x07` - Light Gray on Black).
*   `input_buf` (64 Bytes): Buffer for shell input.
*   `input_pos` (DWORD): Current index in `input_buf`.
*   `matrix_cols` (80 DWORDs): State array for the Matrix effect.

### 7.2. Hardware Ports Used

| Port | Direction | Usage |
| :--- | :--- | :--- |
| `0x60` | Read | PS/2 Keyboard Data Register. |
| `0x64` | Read | PS/2 Keyboard Status Register. |
| `0x64` | Write | PS/2 Keyboard Command Register (Reboot). |
| `0x92` | Read/Write | Fast A20 Gate Control. |
| `0x3C0` | Write | VGA Attribute Controller Index/Data. |
| `0x3C2` | Write | VGA Miscellaneous Output Register. |
| `0x3C4` | Write | VGA Sequencer Index/Data. |
| `0x3CE` | Write | VGA Graphics Controller Index/Data. |
| `0x3D4` | Write | VGA CRTC Index/Data. |

---

## 8. Limitations & Future Work

1.  **No Interrupt Descriptor Table (IDT):** The OS runs with interrupts disabled (`cli`) or ignored. It relies entirely on polling for I/O. This prevents multitasking and efficient power management.
2.  **No Memory Management:** There is no heap, stack protection, or virtual memory. The kernel operates in a flat physical memory model.
3.  **Static GUI:** The GUI is non-interactive except for exiting. No mouse support is implemented.
4.  **No File System:** The OS cannot load external files. All content is compiled into the binary.
5.  **Hardcoded Dates/Memory:** Commands like `date`, `mem`, and `uptime` return static strings rather than reading hardware registers (RTC, PMIC, or PIT).
6.  **Single-Core:** No SMP (Symmetric Multi-Processing) support.

---

## 9. Compilation Instructions

To assemble and create a bootable image:

```bash
nasm -f bin blackmatrix.asm -o blackmatrix.bin
```

To run in QEMU:

```bash
qemu-system-i386 -fda blackmatrix.bin
```

To write to a USB drive (Linux):

```bash
sudo dd if=blackmatrix.bin of=/dev/sdX bs=512 count=1 conv=fsync
```
*(Replace `/dev/sdX` with the target device identifier)*

---

**End of Document**
