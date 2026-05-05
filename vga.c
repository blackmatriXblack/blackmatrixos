#include "../include/vga.h"
#include "../include/io.h"

static int     cursor_row = 0;
static int     cursor_col = 0;
static uint8_t text_color = VGA_COLOR(VGA_LIGHT_GREY, VGA_BLACK);

void vga_init(void) {
    cursor_row = 0;
    cursor_col = 0;
    text_color = VGA_COLOR(VGA_LIGHT_GREY, VGA_BLACK);
    vga_clear();
    /* Enable cursor (scanline 14-15) */
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | 14);
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | 15);
}

void vga_clear(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_MEMORY[i] = ((uint16_t)text_color << 8) | ' ';
    }
    cursor_row = 0;
    cursor_col = 0;
    vga_set_cursor(0, 0);
}

void vga_set_color(uint8_t color) {
    text_color = color;
}

uint8_t vga_get_color(void) {
    return text_color;
}

int vga_get_row(void) { return cursor_row; }
int vga_get_col(void) { return cursor_col; }

void vga_set_cursor(int row, int col) {
    uint16_t pos = row * VGA_WIDTH + col;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void vga_scroll(void) {
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
        VGA_MEMORY[i] = VGA_MEMORY[i + VGA_WIDTH];
    }
    for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
        VGA_MEMORY[i] = ((uint16_t)text_color << 8) | ' ';
    }
    cursor_row = VGA_HEIGHT - 1;
}

void vga_putchar(char c) {
    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
    } else if (c == '\r') {
        cursor_col = 0;
    } else if (c == '\t') {
        cursor_col = (cursor_col + 8) & ~7;
    } else if (c == '\b') {
        if (cursor_col > 0) {
            cursor_col--;
            VGA_MEMORY[cursor_row * VGA_WIDTH + cursor_col] = ((uint16_t)text_color << 8) | ' ';
        }
    } else {
        VGA_MEMORY[cursor_row * VGA_WIDTH + cursor_col] = ((uint16_t)text_color << 8) | c;
        cursor_col++;
    }

    if (cursor_col >= VGA_WIDTH) {
        cursor_col = 0;
        cursor_row++;
    }
    if (cursor_row >= VGA_HEIGHT) {
        vga_scroll();
    }
    vga_set_cursor(cursor_row, cursor_col);
}

void vga_puts(const char* s) {
    while (*s) {
        vga_putchar(*s++);
    }
}

void vga_puts_color(const char* s, uint8_t color) {
    uint8_t old = text_color;
    text_color = color;
    vga_puts(s);
    text_color = old;
}

void vga_put_at(char c, uint8_t color, int row, int col) {
    VGA_MEMORY[row * VGA_WIDTH + col] = ((uint16_t)color << 8) | c;
}

/* ==============================================================================
 * Graphics Mode (VGA Mode 13h: 320x200x256)
 * ============================================================================== */

static bool graphics_mode = false;

void gfx_graphics_mode(void) {
    /* Switch to VGA mode 13h (320x200, 256 colors) */
    outb(0x3C2, 0x63);
    outw(0x3D4, 0x0E11);  /* Unlock CRTC registers (clear bit 7) */
    outw(0x3D4, 0x0D06);
    outw(0x3D4, 0x3E07);
    outw(0x3D4, 0x4109);
    outw(0x3D4, 0xEA10);
    outw(0x3D4, 0x2C11);  /* Vert Retrace End (bit 7 = 0) */
    outw(0x3D4, 0xDF12);
    outw(0x3D4, 0x0014);
    outw(0x3D4, 0xE715);
    outw(0x3D4, 0x0616);
    outw(0x3D4, 0xE317);
    outw(0x3C4, 0x0101);
    outw(0x3C4, 0x0E04);
    outw(0x3CE, 0x4005);
    outw(0x3CE, 0x0506);
    graphics_mode = true;
    gfx_clear(0);
}

void gfx_text_mode(void) {
    /* Switch back to text mode via VGA registers (int 10h won't work in pmode) */
    outb(0x3C2, 0x67);
    outw(0x3D4, 0x0E11);  /* Unlock CRTC */
    outw(0x3D4, 0x0D06);
    outw(0x3D4, 0x3E07);
    outw(0x3D4, 0x0009);
    outw(0x3D4, 0xEA10);
    outw(0x3D4, 0x8E11);
    outw(0x3D4, 0xDF12);
    outw(0x3D4, 0x2813);
    outw(0x3D4, 0x0014);
    outw(0x3D4, 0xE715);
    outw(0x3D4, 0x0416);
    outw(0x3D4, 0xE317);
    outw(0x3C4, 0x0101);
    outw(0x3C4, 0x0302);
    outw(0x3C4, 0x0003);
    outw(0x3C4, 0x0204);
    outw(0x3CE, 0x1005);
    outw(0x3CE, 0x0E06);
    outb(0x3C0, 0x20);     /* Re-enable video */
    graphics_mode = false;
    vga_init();
}

void gfx_init(void) {
    /* Start in text mode */
    graphics_mode = false;
}

void gfx_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < GFX_WIDTH && y >= 0 && y < GFX_HEIGHT) {
        GFX_MEM[y * GFX_WIDTH + x] = color;
    }
}

void gfx_clear(uint8_t color) {
    for (int i = 0; i < GFX_WIDTH * GFX_HEIGHT; i++) {
        GFX_MEM[i] = color;
    }
}

void gfx_fill_rect(int x, int y, int w, int h, uint8_t color) {
    for (int dy = 0; dy < h; dy++) {
        for (int dx = 0; dx < w; dx++) {
            gfx_pixel(x + dx, y + dy, color);
        }
    }
}

void gfx_rect(int x, int y, int w, int h, uint8_t color) {
    for (int i = 0; i < w; i++) {
        gfx_pixel(x + i, y, color);
        gfx_pixel(x + i, y + h - 1, color);
    }
    for (int i = 0; i < h; i++) {
        gfx_pixel(x, y + i, color);
        gfx_pixel(x + w - 1, y + i, color);
    }
}

void gfx_line(int x0, int y0, int x1, int y1, uint8_t color) {
    int dx = (x1 > x0) ? x1 - x0 : x0 - x1;
    int dy = (y1 > y0) ? y1 - y0 : y0 - y1;
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    for (;;) {
        gfx_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }
}

void gfx_circle(int cx, int cy, int r, uint8_t color) {
    int x = r, y = 0;
    int d = 1 - r;
    while (x >= y) {
        gfx_pixel(cx + x, cy + y, color);
        gfx_pixel(cx - x, cy + y, color);
        gfx_pixel(cx + x, cy - y, color);
        gfx_pixel(cx - x, cy - y, color);
        gfx_pixel(cx + y, cy + x, color);
        gfx_pixel(cx - y, cy + x, color);
        gfx_pixel(cx + y, cy - x, color);
        gfx_pixel(cx - y, cy - x, color);
        y++;
        if (d <= 0) {
            d += 2 * y + 1;
        } else {
            x--;
            d += 2 * (y - x) + 1;
        }
    }
}
