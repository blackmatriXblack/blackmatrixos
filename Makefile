# ==============================================================================
# CosmosOS Makefile
# Build system for the CosmosOS operating system
# ==============================================================================

# Toolchain (MSYS2/MinGW paths)
CC      := C:/msys64/mingw64/bin/gcc.exe
LD      := C:/msys64/mingw64/bin/ld.exe
AS      := C:/msys64/mingw64/bin/gcc.exe
NASM    := C:/msys64/mingw64/bin/nasm.exe
OBJCOPY := C:/msys64/mingw64/bin/objcopy.exe
OBJDUMP := C:/msys64/mingw64/bin/objdump.exe
QEMU    := qemu-system-i386

# Compiler flags
CFLAGS  := -m32 -ffreestanding -fno-pie -fno-stack-protector \
           -fno-builtin -nostdlib -nostdinc \
           -Wall -Wextra -Werror -O2 \
           -Iinclude

# Assembler flags (GAS)
ASFLAGS := -m32 -ffreestanding -c

# NASM flags
NASMFLAGS := -f elf32

# Linker flags
LDFLAGS := -m elf_i386 -T scripts/linker.ld --oformat binary

# Source files
C_SRCS := kernel/main.c \
          kernel/gdt.c \
          kernel/idt.c \
          kernel/process.c \
          drivers/vga.c \
          drivers/pic.c \
          drivers/keyboard.c \
          drivers/timer.c \
          mm/memory.c \
          fs/fs.c \
          shell/shell.c \
          gui/gui.c \
          net/net.c \
          lib/string.c \
          lib/stdio.c

ASM_SRCS := kernel/entry.asm \
            kernel/gdt_flush.asm \
            kernel/idt_flush.asm \
            kernel/isr.asm \
            kernel/context.asm

# Object files
C_OBJS   := $(C_SRCS:.c=.o)
ASM_OBJS := $(ASM_SRCS:.asm=.o)
OBJS     := $(C_OBJS) $(ASM_OBJS)

# Output files
KERNEL_BIN := cosmos.bin
KERNEL_ELF := cosmos.elf
BOOT_BIN   := boot/boot.bin
OS_IMG     := cosmosos.img

# ==============================================================================
# Targets
# ==============================================================================

.PHONY: all clean run debug iso

all: $(OS_IMG)

# Link all objects into ELF, then convert to flat binary
$(KERNEL_BIN): $(OBJS)
	$(LD) $(LDFLAGS) -o $(KERNEL_ELF) $(OBJS)
	$(OBJCOPY) -O binary $(KERNEL_ELF) $(KERNEL_BIN)
	@echo "Kernel size: $$(wc -c < $(KERNEL_BIN)) bytes"

# Compile C sources
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble NASM files
kernel/entry.o: kernel/entry.asm
	$(NASM) $(NASMFLAGS) $< -o $@

kernel/gdt_flush.o: kernel/gdt_flush.asm
	$(NASM) $(NASMFLAGS) $< -o $@

kernel/idt_flush.o: kernel/idt_flush.asm
	$(NASM) $(NASMFLAGS) $< -o $@

kernel/isr.o: kernel/isr.asm
	$(NASM) $(NASMFLAGS) $< -o $@

kernel/context.o: kernel/context.asm
	$(NASM) $(NASMFLAGS) $< -o $@

# Build bootloader
$(BOOT_BIN): boot/boot.asm
	$(NASM) -f bin $< -o $@

# Create disk image (bootloader + kernel)
$(OS_IMG): $(BOOT_BIN) $(KERNEL_BIN)
	@echo "Creating disk image..."
	@# Copy bootloader (512 bytes = sector 1)
	cp $(BOOT_BIN) $(OS_IMG)
	@# Pad bootloader to fill sector 1, then append kernel
	@# The bootloader loads sectors 2+ from disk, so kernel goes there
	@# Kernel starts at offset 512 in the image
	cat $(KERNEL_BIN) >> $(OS_IMG)
	@# Pad to minimum 1.44MB floppy size
	@SIZE=$$(wc -c < $(OS_IMG)); \
	if [ $$SIZE -lt 1474560 ]; then \
		dd if=/dev/zero bs=1 count=$$((1474560 - $$SIZE)) >> $(OS_IMG) 2>/dev/null; \
	fi
	@echo "Disk image: $(OS_IMG) ($$(wc -c < $(OS_IMG)) bytes)"

# Run in QEMU
run: $(OS_IMG)
	$(QEMU) -drive format=raw,file=$(OS_IMG) -m 128M -monitor stdio

# Run in QEMU with debug
debug: $(OS_IMG)
	$(QEMU) -drive format=raw,file=$(OS_IMG) -m 128M -s -S -monitor stdio

# Generate disassembly
disasm: $(KERNEL_ELF)
	$(OBJDUMP) -d $(KERNEL_ELF) > cosmos.asm

# Clean build artifacts
clean:
	rm -f $(OBJS) $(KERNEL_BIN) $(KERNEL_ELF) $(BOOT_BIN) $(OS_IMG) cosmos.asm

# Show help
help:
	@echo "CosmosOS Build System"
	@echo "====================="
	@echo "  make all      - Build everything"
	@echo "  make run      - Build and run in QEMU"
	@echo "  make debug    - Build and run with GDB server"
	@echo "  make disasm   - Generate kernel disassembly"
	@echo "  make clean    - Remove build artifacts"
	@echo ""
	@echo "Requirements:"
	@echo "  - MSYS2/MinGW with GCC, NASM, LD"
	@echo "  - QEMU for testing"
