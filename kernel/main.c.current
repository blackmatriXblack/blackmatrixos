/*
 * BlackMatrixOS Kernel
 * A simple operating system with command line and GUI
 */

#include "../include/gdt.h"
#include "../include/idt.h"
#include "../include/pic.h"
#include "../include/timer.h"
#include "../include/keyboard.h"
#include "../include/vga.h"
#include "../include/memory.h"
#include "../include/fs.h"
#include "../include/shell.h"
#include "../include/gui.h"
#include "../include/net.h"
#include "../include/process.h"
#include "../include/stdio.h"

/* VGA text mode constants */
#define VGA_MEM ((volatile unsigned short*)0xB8000)
#define VGA_W   80
#define VGA_H   25
#define VGA_COLOR(fg, bg) (((bg) << 4) | ((fg) & 0x0F))

/* VGA colors */
#define BLACK   0
#define BLUE    1
#define GREEN   2
#define CYAN    3
#define RED     4
#define MAGENTA 5
#define BROWN   6
#define LGRAY   7
#define DGRAY   8
#define LBLUE   9
#define LGREEN  10
#define LCYAN   11
#define LRED    12
#define LMAGENTA 13
#define YELLOW  14
#define WHITE   15

/* I/O port helpers */
static inline void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* Global state */
static int cursor_x = 0;
static int cursor_y = 0;
static unsigned char current_color = VGA_COLOR(LGRAY, BLACK);
static char input_buf[256];
static int input_pos = 0;
static int gui_mode = 0;
static int cmd_hist_count = 0;
static char cmd_hist[32][64];

/* Scancode to ASCII table (US layout, key-down only) */
static const char scancode_ascii[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=',8,
    9, 'q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0, 'a','s','d','f','g','h','j','k','l',';','\'','`',
    0, '\\','z','x','c','v','b','n','m',',','.','/',0,
    '*', 0, ' '
};

/* Simple string functions */
static int strlen(const char* s) {
    int n = 0;
    while (s[n]) n++;
    return n;
}
static int strcmp(const char* a, const char* b) {
    while (*a && *a == *b) { a++; b++; }
    return *(unsigned char*)a - *(unsigned char*)b;
}
static void strcpy(char* d, const char* s) {
    while ((*d++ = *s++));
}
static int strncmp(const char* a, const char* b, int n) {
    while (n-- && *a && *a == *b) { a++; b++; }
    if (n < 0) return 0;
    return *(unsigned char*)a - *(unsigned char*)b;
}

/* VGA functions */
static void vga_clear(void) {
    for (int i = 0; i < VGA_W * VGA_H; i++)
        VGA_MEM[i] = (current_color << 8) | ' ';
    cursor_x = 0;
    cursor_y = 0;
}

static void vga_update_cursor(void) {
    unsigned short pos = cursor_y * VGA_W + cursor_x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, pos & 0xFF);
    outb(0x3D4, 0x0E);
    outb(0x3D5, (pos >> 8) & 0xFF);
}

static void vga_scroll(void) {
    for (int i = 0; i < (VGA_H - 1) * VGA_W; i++)
        VGA_MEM[i] = VGA_MEM[i + VGA_W];
    for (int i = (VGA_H - 1) * VGA_W; i < VGA_H * VGA_W; i++)
        VGA_MEM[i] = (current_color << 8) | ' ';
    cursor_y = VGA_H - 1;
}

static void vga_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            VGA_MEM[cursor_y * VGA_W + cursor_x] = (current_color << 8) | ' ';
        }
    } else if (c >= 32) {
        VGA_MEM[cursor_y * VGA_W + cursor_x] = (current_color << 8) | c;
        cursor_x++;
    }
    if (cursor_x >= VGA_W) {
        cursor_x = 0;
        cursor_y++;
    }
    if (cursor_y >= VGA_H) vga_scroll();
    vga_update_cursor();
}

static void vga_puts(const char* s) {
    while (*s) vga_putchar(*s++);
}

static void vga_set_color(unsigned char c) {
    current_color = c;
}

/* Simple printf replacement */
static void kprintf(const char* fmt, ...) {
    /* Simple implementation - just print string */
    vga_puts(fmt);
}

/* Keyboard input */
static int kb_has_input(void) {
    return inb(0x64) & 1;
}

static char kb_getchar(void) {
    while (!kb_has_input());
    unsigned char sc = inb(0x60);
    if (sc & 0x80) return 0;  /* Key release */
    if (sc < 128) return scancode_ascii[sc];
    return 0;
}

static char kb_wait_char(void) {
    char c;
    while ((c = kb_getchar()) == 0);
    return c;
}

/* Timer delay (approximate) */
static void delay(int ms) {
    for (volatile int i = 0; i < ms * 1000; i++);
}

/* ==============================================================================
 * Command implementations
 * ============================================================================== */

static void cmd_help(void) {
    vga_set_color(VGA_COLOR(LCYAN, BLACK));
    vga_puts("\nBlackMatrixOS Commands:\n");
    vga_set_color(VGA_COLOR(LGRAY, BLACK));
    vga_puts("  help      - Show this help\n");
    vga_puts("  clear     - Clear screen\n");
    vga_puts("  echo      - Print text\n");
    vga_puts("  ver       - OS version\n");
    vga_puts("  time      - Show uptime\n");
    vga_puts("  mem       - Memory info\n");
    vga_puts("  color     - Change colors\n");
    vga_puts("  calc      - Calculator\n");
    vga_puts("  gui       - Enter GUI mode (or F1)\n");
    vga_puts("  reboot    - Restart system\n");
    vga_puts("  matrix    - Matrix rain\n");
    vga_puts("  about     - About BlackMatrixOS\n");
    vga_puts("  hostname  - Show hostname\n");
    vga_puts("  whoami    - Show current user\n");
    vga_puts("  uname     - System information\n");
    vga_puts("  date      - Show date/time\n");
    vga_puts("  cowsay    - Talking cow\n");
    vga_puts("  banner    - Large text banner\n");
    vga_puts("  hexdump   - Hex dump of address\n");
    vga_puts("  lspci     - List PCI devices\n");
    vga_puts("  ls        - List files\n");
    vga_puts("  history   - Command history\n");
}

static void cmd_ver(void) {
    vga_set_color(VGA_COLOR(LGREEN, BLACK));
    vga_puts("\nBlackMatrixOS v1.0.0\n");
    vga_set_color(VGA_COLOR(LGRAY, BLACK));
    vga_puts("Build: 2026-03-29\n");
    vga_puts("Architecture: x86 (i686) 32-bit\n");
    vga_puts("Kernel: Monolithic\n");
}

static void cmd_about(void) {
    vga_set_color(VGA_COLOR(LCYAN, BLACK));
    vga_puts("\n");
    vga_puts("  ____  _            _    _   _      _   _         ___  ____\n");
    vga_puts(" | __ )| | __ _  ___| | _| | | | ___| |_(_) ___   / _ \\/ ___|\n");
    vga_puts(" |  _ \\| |/ _` |/ __| |/ / |_| |/ _ \\ __| |/ __| | | | \\___ \\\n");
    vga_puts(" | |_) | | (_| | (__|   <|  _  |  __/ |_| | (__  | |_| |___) |\n");
    vga_puts(" |____/|_|\\__,_|\\___|_|\\_\\_| |_|\\___|\\__|_|\\___|  \\___/|____/\n");
    vga_puts("\n");
    vga_set_color(VGA_COLOR(LGRAY, BLACK));
    vga_puts(" BlackMatrixOS - The Matrix Operating System\n");
    vga_puts(" A minimalist OS with retro aesthetics\n");
}

static void cmd_calc(void) {
    vga_puts("\nSimple Calculator (enter expression like: 5+3)\n");
    vga_puts("Enter: ");
    char expr[64];
    int i = 0;
    while (i < 63) {
        char c = kb_wait_char();
        if (c == '\n') break;
        if (c == '\b' && i > 0) { i--; vga_putchar('\b'); continue; }
        if (c >= 32) { expr[i++] = c; vga_putchar(c); }
    }
    expr[i] = 0;
    /* Very simple: expects A+B, A-B, A*B, A/B */
    if (i >= 3) {
        int a = expr[0] - '0';
        int b = expr[2] - '0';
        int r = 0;
        if (expr[1] == '+') r = a + b;
        else if (expr[1] == '-') r = a - b;
        else if (expr[1] == '*') r = a * b;
        else if (expr[1] == '/' && b != 0) r = a / b;
        vga_puts("\nResult: ");
        /* Print number (simple) */
        if (r < 0) { vga_putchar('-'); r = -r; }
        if (r >= 10) vga_putchar('0' + r / 10);
        vga_putchar('0' + r % 10);
    }
    vga_putchar('\n');
}

/* ==============================================================================
 * Matrix Rain Effect
 * ============================================================================== */

static void cmd_matrix(void) {
    vga_set_color(VGA_COLOR(GREEN, BLACK));
    vga_clear();
    vga_puts("Matrix Rain - Press ESC to exit\n\n");

    int cols[VGA_W];
    for (int i = 0; i < VGA_W; i++) cols[i] = -(i * 7 % 25);

    int running = 1;
    while (running) {
        if (kb_has_input()) {
            unsigned char sc = inb(0x60);
            if (sc == 0x01) running = 0;  /* ESC */
        }

        for (int x = 0; x < VGA_W; x++) {
            if (cols[x] >= 0 && cols[x] < VGA_H) {
                char c = 33 + (x * 7 + cols[x]) % 93;
                VGA_MEM[cols[x] * VGA_W + x] = (VGA_COLOR(LGREEN, BLACK) << 8) | c;
                if (cols[x] > 0)
                    VGA_MEM[(cols[x]-1) * VGA_W + x] = (VGA_COLOR(GREEN, BLACK) << 8) | (33 + (x + cols[x]) % 93);
            }
            cols[x]++;
            if (cols[x] > VGA_H + 3) cols[x] = -(x * 3 % 20);
        }
        delay(50);
    }
    vga_set_color(VGA_COLOR(LGRAY, BLACK));
    vga_clear();
}

/* ==============================================================================
 * Additional Commands
 * ============================================================================== */

static void cmd_hostname(void) {
    vga_puts("blackmatrix-os\n");
}

static void cmd_whoami(void) {
    vga_puts("root\n");
}

static void cmd_uname(void) {
    vga_puts("BlackMatrixOS blackmatrix-os 1.0.0 x86 i686 BlackMatrixOS\n");
}

static void cmd_date(void) {
    vga_puts("Sat Mar 29 00:00:00 UTC 2026\n");
}

static void cmd_cowsay(void) {
    vga_puts(" _________________________________\n");
    vga_puts("< BlackMatrixOS is udderly great! >\n");
    vga_puts(" ---------------------------------\n");
    vga_puts("        \\   ^__^\n");
    vga_puts("         \\  (oo)\\_______\n");
    vga_puts("            (__)\\       )\\/\\\n");
    vga_puts("                ||----w |\n");
    vga_puts("                ||     ||\n");
}

static void cmd_banner(void) {
    vga_puts("Type text: ");
    char text[32];
    int i = 0;
    while (i < 31) {
        char c = kb_wait_char();
        if (c == '\n') break;
        if (c >= 32 && c < 127) { text[i++] = c; vga_putchar(c); }
    }
    text[i] = 0;
    vga_putchar('\n');
    vga_set_color(VGA_COLOR(LGREEN, BLACK));
    for (int j = 0; j < i; j++) {
        vga_putchar(' ');
        vga_putchar(text[j]);
        vga_putchar(text[j]);
    }
    vga_putchar('\n');
    for (int j = 0; j < i; j++) {
        vga_putchar(text[j]);
        vga_putchar(' ');
        vga_putchar(text[j]);
    }
    vga_putchar('\n');
    for (int j = 0; j < i; j++) {
        vga_putchar(' ');
        vga_putchar(text[j]);
        vga_putchar(text[j]);
    }
    vga_putchar('\n');
    vga_set_color(VGA_COLOR(LGRAY, BLACK));
}

static void cmd_hexdump_addr(void) {
    vga_puts("Address (hex): ");
    char addr_str[16];
    int i = 0;
    while (i < 15) {
        char c = kb_wait_char();
        if (c == '\n') break;
        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
            addr_str[i++] = c;
            vga_putchar(c);
        }
    }
    addr_str[i] = 0;
    vga_putchar('\n');
    /* Parse hex address */
    unsigned int addr = 0;
    for (int j = 0; j < i; j++) {
        addr <<= 4;
        if (addr_str[j] >= '0' && addr_str[j] <= '9') addr |= (addr_str[j] - '0');
        else if (addr_str[j] >= 'a' && addr_str[j] <= 'f') addr |= (addr_str[j] - 'a' + 10);
        else if (addr_str[j] >= 'A' && addr_str[j] <= 'F') addr |= (addr_str[j] - 'A' + 10);
    }
    unsigned char* ptr = (unsigned char*)addr;
    for (int row = 0; row < 4; row++) {
        vga_puts("0x");
        vga_putchar("0123456789ABCDEF"[((addr + row * 16) >> 12) & 0xF]);
        vga_putchar("0123456789ABCDEF"[((addr + row * 16) >> 8) & 0xF]);
        vga_putchar("0123456789ABCDEF"[((addr + row * 16) >> 4) & 0xF]);
        vga_putchar("0123456789ABCDEF"[(addr + row * 16) & 0xF]);
        vga_puts(": ");
        for (int col = 0; col < 16; col++) {
            unsigned char b = ptr[row * 16 + col];
            vga_putchar("0123456789ABCDEF"[b >> 4]);
            vga_putchar("0123456789ABCDEF"[b & 0xF]);
            vga_putchar(' ');
        }
        vga_puts(" |");
        for (int col = 0; col < 16; col++) {
            unsigned char b = ptr[row * 16 + col];
            vga_putchar((b >= 32 && b < 127) ? b : '.');
        }
        vga_puts("|\n");
    }
}

static void cmd_lspci(void) {
    vga_puts("PCI Devices:\n");
    vga_puts("  00:00.0 Host bridge: Intel 440FX - 82441FX PMC\n");
    vga_puts("  00:01.0 ISA bridge: Intel PIIX3\n");
    vga_puts("  00:01.1 IDE interface: Intel PIIX3 IDE\n");
    vga_puts("  00:02.0 VGA compatible: Bochs/QEMU VGA\n");
    vga_puts("  00:03.0 Ethernet controller: Realtek RTL-8029\n");
}

static void cmd_ls_main(void) {
    vga_puts("/\n");
    vga_puts("  bin/\n");
    vga_puts("  etc/\n");
    vga_puts("  home/\n");
    vga_puts("  tmp/\n");
    vga_puts("  var/\n");
    vga_puts("  usr/\n");
    vga_puts("  dev/\n");
    vga_puts("  proc/\n");
}

static void cmd_history(void) {
    for (int i = 0; i < cmd_hist_count; i++) {
        vga_puts("  ");
        /* Print number */
        if (i >= 9) vga_putchar('0' + (i + 1) / 10);
        vga_putchar('0' + (i + 1) % 10);
        vga_puts("  ");
        vga_puts(cmd_hist[i]);
        vga_putchar('\n');
    }
}

/* ==============================================================================
 * GUI Mode (VGA Mode 13h - 320x200x256 colors)
 * ============================================================================== */

#define GFX ((volatile unsigned char*)0xA0000)

static void gfx_pixel(int x, int y, unsigned char color) {
    if (x >= 0 && x < 320 && y >= 0 && y < 200)
        GFX[y * 320 + x] = color;
}

static void gfx_fill_rect(int x, int y, int w, int h, unsigned char color) {
    for (int dy = 0; dy < h; dy++)
        for (int dx = 0; dx < w; dx++)
            gfx_pixel(x + dx, y + dy, color);
}

static void gfx_rect(int x, int y, int w, int h, unsigned char color) {
    for (int i = 0; i < w; i++) {
        gfx_pixel(x + i, y, color);
        gfx_pixel(x + i, y + h - 1, color);
    }
    for (int i = 0; i < h; i++) {
        gfx_pixel(x, y + i, color);
        gfx_pixel(x + w - 1, y + i, color);
    }
}

/* 8x8 mini font for GUI */
static const unsigned char font8[96][8] = {
    {0,0,0,0,0,0,0,0},{0x18,0x3C,0x3C,0x18,0x18,0,0x18,0},
    {0x6C,0x6C,0x24,0,0,0,0,0},{0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0},
    {0x18,0x7E,0xC0,0x7C,0x06,0xFC,0x18,0},{0,0xC6,0xCC,0x18,0x30,0x66,0xC6,0},
    {0x38,0x6C,0x38,0x76,0xDC,0xCC,0x76,0},{0x18,0x18,0x30,0,0,0,0,0},
    {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0},{0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0},
    {0,0x66,0x3C,0xFF,0x3C,0x66,0,0},{0,0x18,0x18,0x7E,0x18,0x18,0,0},
    {0,0,0,0,0,0x18,0x18,0x30},{0,0,0,0x7E,0,0,0,0},
    {0,0,0,0,0,0x18,0x18,0},{0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0},
    {0x7C,0xC6,0xCE,0xD6,0xE6,0xC6,0x7C,0},{0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0},
    {0x7C,0xC6,0x06,0x1C,0x30,0x66,0xFE,0},{0x7C,0xC6,0x06,0x3C,0x06,0xC6,0x7C,0},
    {0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x1E,0},{0xFE,0xC0,0xFC,0x06,0x06,0xC6,0x7C,0},
    {0x7C,0xC6,0xC0,0xFC,0xC6,0xC6,0x7C,0},{0xFE,0xC6,0x0C,0x18,0x30,0x30,0x30,0},
    {0x7C,0xC6,0xC6,0x7C,0xC6,0xC6,0x7C,0},{0x7C,0xC6,0xC6,0x7E,0x06,0x0C,0x78,0},
    {0,0x18,0x18,0,0,0x18,0x18,0},{0,0x18,0x18,0,0,0x18,0x18,0x30},
    {0x06,0x0C,0x18,0x30,0x18,0x0C,0x06,0},{0,0,0x7E,0,0x7E,0,0,0},
    {0x60,0x30,0x18,0x0C,0x18,0x30,0x60,0},{0x7C,0xC6,0x0C,0x18,0x18,0,0x18,0},
    {0x7C,0xC6,0xDE,0xDE,0xDE,0xC0,0x78,0},{0x38,0x6C,0xC6,0xC6,0xFE,0xC6,0xC6,0},
    {0xFC,0x66,0x66,0x7C,0x66,0x66,0xFC,0},{0x3C,0x66,0xC0,0xC0,0xC0,0x66,0x3C,0},
    {0xF8,0x6C,0x66,0x66,0x66,0x6C,0xF8,0},{0xFE,0x62,0x68,0x78,0x68,0x62,0xFE,0},
    {0xFE,0x62,0x68,0x78,0x68,0x60,0xF0,0},{0x3C,0x66,0xC0,0xC0,0xCE,0x66,0x3E,0},
    {0xC6,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0},{0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0},
    {0x1E,0x0C,0x0C,0x0C,0xCC,0xCC,0x78,0},{0xE6,0x66,0x6C,0x78,0x6C,0x66,0xE6,0},
    {0xF0,0x60,0x60,0x60,0x62,0x66,0xFE,0},{0xC6,0xEE,0xFE,0xD6,0xC6,0xC6,0xC6,0},
    {0xC6,0xE6,0xF6,0xDE,0xCE,0xC6,0xC6,0},{0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0},
    {0xFC,0x66,0x66,0x7C,0x60,0x60,0xF0,0},{0x7C,0xC6,0xC6,0xC6,0xD6,0xDE,0x7C,0x0E},
    {0xFC,0x66,0x66,0x7C,0x6C,0x66,0xE6,0},{0x7C,0xC6,0xC0,0x7C,0x06,0xC6,0x7C,0},
    {0xFE,0x92,0x18,0x18,0x18,0x18,0x3C,0},{0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0},
    {0xC6,0xC6,0xC6,0xC6,0x6C,0x38,0x10,0},{0xC6,0xC6,0xC6,0xD6,0xFE,0xEE,0xC6,0},
    {0xC6,0xC6,0x6C,0x38,0x6C,0xC6,0xC6,0},{0xC6,0xC6,0xC6,0x7C,0x0C,0x18,0xF0,0},
    {0xFE,0xC6,0x8C,0x18,0x32,0x66,0xFE,0},{0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0},
    {0xC0,0x60,0x30,0x18,0x0C,0x06,0x02,0},{0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0},
    {0x10,0x38,0x6C,0xC6,0,0,0,0},{0,0,0,0,0,0,0,0xFF},
    {0x30,0x18,0x0C,0,0,0,0,0},{0,0,0x78,0x0C,0x7C,0xCC,0x76,0},
    {0xE0,0x60,0x7C,0x66,0x66,0x66,0xDC,0},{0,0,0x7C,0xC6,0xC0,0xC6,0x7C,0},
    {0x1C,0x0C,0x7C,0xCC,0xCC,0xCC,0x76,0},{0,0,0x7C,0xC6,0xFE,0xC0,0x7C,0},
    {0x1C,0x36,0x30,0x78,0x30,0x30,0x78,0},{0,0,0x76,0xCC,0xCC,0x7C,0x0C,0x78},
    {0xE0,0x60,0x6C,0x76,0x66,0x66,0xE6,0},{0x18,0,0x38,0x18,0x18,0x18,0x3C,0},
    {0x06,0,0x0E,0x06,0x06,0x66,0x66,0x3C},{0xE0,0x60,0x66,0x6C,0x78,0x6C,0xE6,0},
    {0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0},{0,0,0xEC,0xFE,0xD6,0xC6,0xC6,0},
    {0,0,0xDC,0x66,0x66,0x66,0x66,0},{0,0,0x7C,0xC6,0xC6,0xC6,0x7C,0},
    {0,0,0xDC,0x66,0x66,0x7C,0x60,0xF0},{0,0,0x76,0xCC,0xCC,0x7C,0x0C,0x1E},
    {0,0,0xDC,0x76,0x60,0x60,0xF0,0},{0,0,0x7C,0xC0,0x7C,0x06,0xFC,0},
    {0x30,0x30,0x7C,0x30,0x30,0x36,0x1C,0},{0,0,0xCC,0xCC,0xCC,0xCC,0x76,0},
    {0,0,0xC6,0xC6,0xC6,0x6C,0x38,0},{0,0,0xC6,0xC6,0xD6,0xFE,0x6C,0},
    {0,0,0xC6,0x6C,0x38,0x6C,0xC6,0},{0,0,0xC6,0xC6,0xC6,0x7E,0x06,0xFC},
    {0,0,0xFE,0x8C,0x18,0x32,0xFE,0},{0x1C,0x30,0x30,0x60,0x30,0x30,0x1C,0},
    {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0},{0x38,0x0C,0x0C,0x06,0x0C,0x0C,0x38,0},
    {0,0x76,0xDC,0,0,0,0,0},{0x10,0x38,0x6C,0xC6,0xC6,0xFE,0,0}
};

static void gfx_draw_char(int x, int y, char c, unsigned char fg) {
    if (c < 32 || c > 126) c = '?';
    const unsigned char* g = font8[c - 32];
    for (int r = 0; r < 8; r++)
        for (int col = 0; col < 8; col++)
            if (g[r] & (1 << col))
                gfx_pixel(x + col, y + r, fg);
}

static void gfx_draw_string(int x, int y, const char* s, unsigned char fg) {
    while (*s) {
        gfx_draw_char(x, y, *s++, fg);
        x += 8;
    }
}

static void gui_mode_run(void) {
    /* Clear screen to blue desktop */
    volatile unsigned short* vga = (volatile unsigned short*)0xB8000;
    for (int i = 0; i < 80 * 25; i++)
        vga[i] = (0x1F << 8) | ' ';

    /* Taskbar (row 24) - light gray bg */
    for (int i = 0; i < 80; i++)
        vga[24 * 80 + i] = (0x70 << 8) | ' ';

    /* Start button */
    vga[24 * 80 + 0] = (0x2F << 8) | 'S';
    vga[24 * 80 + 1] = (0x2F << 8) | 't';
    vga[24 * 80 + 2] = (0x2F << 8) | 'a';
    vga[24 * 80 + 3] = (0x2F << 8) | 'r';
    vga[24 * 80 + 4] = (0x2F << 8) | 't';

    /* Clock */
    vga[24 * 80 + 75] = (0x70 << 8) | '1';
    vga[24 * 80 + 76] = (0x70 << 8) | '2';
    vga[24 * 80 + 77] = (0x70 << 8) | ':';
    vga[24 * 80 + 78] = (0x70 << 8) | '0';
    vga[24 * 80 + 79] = (0x70 << 8) | '0';

    /* Window 1: Terminal (rows 2-14, cols 2-48) */
    for (int r = 2; r <= 14; r++)
        for (int c = 2; c <= 48; c++)
            vga[r * 80 + c] = (r == 2) ? (0x1F << 8) | ' ' : (0x07 << 8) | ' ';

    /* Terminal title */
    const char* ttitle = "Terminal";
    for (int i = 0; ttitle[i]; i++)
        vga[2 * 80 + 6 + i] = (0x1F << 8) | ttitle[i];

    /* Close button */
    vga[2 * 80 + 47] = (0x4F << 8) | 'X';

    /* Terminal content */
    const char* tline1 = "root@bmos:~$ ";
    for (int i = 0; tline1[i]; i++)
        vga[4 * 80 + 4 + i] = (0x0F << 8) | tline1[i];

    const char* tline2 = "BlackMatrixOS Terminal v2.0";
    for (int i = 0; tline2[i]; i++)
        vga[5 * 80 + 4 + i] = (0x0A << 8) | tline2[i];

    const char* tline3 = "Type 'help' for commands";
    for (int i = 0; tline3[i]; i++)
        vga[6 * 80 + 4 + i] = (0x07 << 8) | tline3[i];

    /* Window 2: System Info (rows 4-12, cols 52-78) */
    for (int r = 4; r <= 12; r++)
        for (int c = 52; c <= 78; c++)
            vga[r * 80 + c] = (r == 4) ? (0x1F << 8) | ' ' : (0x07 << 8) | ' ';

    const char* stitle = "System Info";
    for (int i = 0; stitle[i]; i++)
        vga[4 * 80 + 54 + i] = (0x1F << 8) | stitle[i];

    vga[4 * 80 + 77] = (0x4F << 8) | 'X';

    /* Info lines */
    const char* info1 = "OS: BlackMatrixOS v2.0";
    for (int i = 0; info1[i]; i++) vga[6 * 80 + 54 + i] = (0x0A << 8) | info1[i];

    const char* info2 = "Arch: x86 i686 32-bit";
    for (int i = 0; info2[i]; i++) vga[7 * 80 + 54 + i] = (0x07 << 8) | info2[i];

    const char* info3 = "RAM: 128 MB";
    for (int i = 0; info3[i]; i++) vga[8 * 80 + 54 + i] = (0x0B << 8) | info3[i];

    const char* info4 = "CPU: QEMU Virtual";
    for (int i = 0; info4[i]; i++) vga[9 * 80 + 54 + i] = (0x07 << 8) | info4[i];

    const char* info5 = "Status: Running";
    for (int i = 0; info5[i]; i++) vga[10 * 80 + 54 + i] = (0x0A << 8) | info5[i];

    /* Desktop icons */
    vga[18 * 80 + 60] = (0x1F << 8) | 'F';
    vga[18 * 80 + 61] = (0x1F << 8) | 'S';
    vga[19 * 80 + 60] = (0x1F << 8) | ' ';
    vga[19 * 80 + 61] = (0x1F << 8) | ' ';

    vga[18 * 80 + 70] = (0x1F << 8) | 'T';
    vga[18 * 80 + 71] = (0x1F << 8) | 'S';

    /* URL bar */
    const char* url = "https://bmatrix.os/home";
    for (int i = 0; i < 60; i++)
        vga[16 * 80 + 10 + i] = (0x30 << 8) | ' ';
    for (int i = 0; url[i]; i++)
        vga[16 * 80 + 12 + i] = (0x3F << 8) | url[i];

    /* Wait for ESC or F1 */
    while (1) {
        if (kb_has_input()) {
            unsigned char sc = inb(0x60);
            if (sc == 0x01) break;  /* ESC */
            if (sc == 0x3B) break;  /* F1 */
        }
    }

    vga_clear();
    gui_mode = 0;
}

/* ==============================================================================
 * Shell / Command Loop
 * ============================================================================== */

static void print_prompt(void) {
    vga_set_color(VGA_COLOR(LGREEN, BLACK));
    vga_puts("blackmatrix");
    vga_set_color(VGA_COLOR(WHITE, BLACK));
    vga_puts("@");
    vga_set_color(VGA_COLOR(LCYAN, BLACK));
    vga_puts("bmos");
    vga_set_color(VGA_COLOR(LGRAY, BLACK));
    vga_puts("$ ");
}

static void execute_command(const char* cmd) {
    if (cmd[0] == 0) return;
    vga_putchar('\n');

    /* Add to history */
    if (cmd_hist_count < 32) {
        int i = 0;
        while (cmd[i] && i < 63) { cmd_hist[cmd_hist_count][i] = cmd[i]; i++; }
        cmd_hist[cmd_hist_count][i] = 0;
        cmd_hist_count++;
    }

    if (strcmp(cmd, "help") == 0) cmd_help();
    else if (strcmp(cmd, "clear") == 0 || strcmp(cmd, "cls") == 0) vga_clear();
    else if (strcmp(cmd, "ver") == 0) cmd_ver();
    else if (strcmp(cmd, "about") == 0) cmd_about();
    else if (strcmp(cmd, "calc") == 0) cmd_calc();
    else if (strcmp(cmd, "matrix") == 0) cmd_matrix();
    else if (strcmp(cmd, "gui") == 0) gui_mode_run();
    else if (strcmp(cmd, "reboot") == 0) {
        vga_puts("Rebooting...\n");
        outb(0x64, 0xFE);
    }
    else if (strncmp(cmd, "echo ", 5) == 0) {
        vga_puts(cmd + 5);
        vga_putchar('\n');
    }
    else if (strcmp(cmd, "time") == 0) {
        vga_puts("System uptime: ");
        vga_puts("(timer not available in this mode)\n");
    }
    else if (strcmp(cmd, "mem") == 0) {
        vga_puts("Memory: 128 MB total\n");
        vga_puts("Used:  ~2 MB\n");
        vga_puts("Free:  ~126 MB\n");
    }
    else if (strcmp(cmd, "color") == 0) {
        vga_puts("Colors: ");
        for (int i = 0; i < 16; i++) {
            vga_set_color(VGA_COLOR(i, BLACK));
            vga_putchar('#');
        }
        vga_set_color(VGA_COLOR(LGRAY, BLACK));
        vga_putchar('\n');
    }
    else if (strcmp(cmd, "hostname") == 0) cmd_hostname();
    else if (strcmp(cmd, "whoami") == 0) cmd_whoami();
    else if (strcmp(cmd, "uname") == 0 || strcmp(cmd, "uname -a") == 0) cmd_uname();
    else if (strcmp(cmd, "date") == 0) cmd_date();
    else if (strcmp(cmd, "cowsay") == 0) cmd_cowsay();
    else if (strcmp(cmd, "banner") == 0) cmd_banner();
    else if (strcmp(cmd, "hexdump") == 0) cmd_hexdump_addr();
    else if (strcmp(cmd, "lspci") == 0) cmd_lspci();
    else if (strcmp(cmd, "ls") == 0 || strcmp(cmd, "dir") == 0) cmd_ls_main();
    else if (strcmp(cmd, "history") == 0) cmd_history();
    else {
        vga_set_color(VGA_COLOR(LRED, BLACK));
        vga_puts("Unknown command: ");
        vga_puts(cmd);
        vga_set_color(VGA_COLOR(LGRAY, BLACK));
        vga_putchar('\n');
    }
}

static void shell(void) {
    input_pos = 0;
    input_buf[0] = 0;

    while (1) {
        print_prompt();

        /* Read input line */
        input_pos = 0;
        while (1) {
            char c = kb_wait_char();
            if (c == '\n') {
                vga_putchar('\n');
                break;
            } else if (c == '\b') {
                if (input_pos > 0) {
                    input_pos--;
                    vga_putchar('\b');
                }
            } else if (c >= 32 && c < 127) {
                if (input_pos < 255) {
                    input_buf[input_pos++] = c;
                    vga_putchar(c);
                }
            }
        }
        input_buf[input_pos] = 0;

        /* Check for F1 key (handled via scancode directly) */
        execute_command(input_buf);
    }
}

/* ==============================================================================
 * Kernel Entry Point
 * ============================================================================== */

void kernel_main(void) {
    /* Initialize hardware */
    vga_init();
    gdt_init();
    idt_init();
    pic_init();
    timer_init(100);   // 100 Hz
    keyboard_init();
    
    /* Initialize subsystems */
    pmm_init();
    fs_init();
    net_init();
    process_init();
    
    /* Initialize and run shell */
    shell_init();
    
    /* Welcome banner */
    kprintf("BlackMatrixOS v1.0 - Matrix Operating System\n");
    kprintf("Type 'help' for commands, 'gui' to launch GUI\n\n");
    
    /* Start shell */
    shell_run();

    /* Should never reach here */
    while (1) __asm__ volatile ("hlt");
}
