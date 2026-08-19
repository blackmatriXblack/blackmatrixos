/*
 * BlackMatrixOS Kernel - main entry point
 * Initializes hardware and subsystems, then starts the shell.
 */

#include "../include/vga.h"
#include "../include/gdt.h"
#include "../include/idt.h"
#include "../include/pic.h"
#include "../include/timer.h"
#include "../include/keyboard.h"
#include "../include/memory.h"
#include "../include/fs.h"
#include "../include/shell.h"
#include "../include/gui.h"
#include "../include/net.h"
#include "../include/process.h"
#include "../include/stdio.h"
#include "../include/io.h"

void kernel_main(void) {
    /* Initialize hardware */
    vga_init();
    gdt_init();
    idt_init();
    pic_init();
    timer_init(100);   /* 100 Hz */
    keyboard_init();

    /* Enable interrupts (PIT timer, keyboard, ...) */
    sti();

    /* Initialize subsystems */
    pmm_init(128 * 1024);   /* 128 MB */
    heap_init();
    fs_init();
    net_init();
    scheduler_init();

    /* Initialize shell */
    shell_init();

    /* Welcome banner */
    kprintf("BlackMatrixOS v2.0 - Matrix Operating System\n");
    kprintf("Type 'help' for commands, 'gui' to launch GUI\n\n");

    /* Start shell */
    shell_run();

    /* Should never reach here */
    while (1) __asm__ volatile ("hlt");
}