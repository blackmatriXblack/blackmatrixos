#ifndef VGA_H
#define VGA_H

#include "types.h"

#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEMORY  ((uint16_t*)0xB8000)

/* VGA colors */
enum vga_color {
    VGA_BLACK        = 0,
    VGA_BLUE         = 1,
    VGA_GREEN        = 2,
    VGA_CYAN         = 3,
    VGA_RED          = 4,
    VGA_MAGENTA      = 5,
    VGA_BROWN        = 6,
    VGA_LIGHT_GREY   = 7,
    VGA_DARK_GREY    = 8,
    VGA_LIGHT_BLUE   = 9,
    VGA_LIGHT_GREEN  = 10,
    VGA_LIGHT_CYAN   = 11,
    VGA_LIGHT_RED    = 12,
    VGA_LIGHT_MAGENTA= 13,
    VGA_YELLOW       = 14,
    VGA_WHITE        = 15,
};

#define VGA_COLOR(fg, bg) ((bg << 4) | (fg & 0x0F))

void     vga_init(void);
void     vga_clear(void);
void     vga_set_color(uint8_t color);
void     vga_set_cursor(int row, int col);
void     vga_putchar(char c);
void     vga_puts(const char* s);
void     vga_puts_color(const char* s, uint8_t color);
void     vga_put_at(char c, uint8_t color, int row, int col);
void     vga_scroll(void);
uint8_t  vga_get_color(void);
int      vga_get_row(void);
int      vga_get_col(void);

/* Framebuffer graphics mode (mode 13h: 320x200x256) */
#define GFX_WIDTH  320
#define GFX_HEIGHT 200
#define GFX_MEM    ((uint8_t*)0xA0000)

void     gfx_init(void);
void     gfx_pixel(int x, int y, uint8_t color);
void     gfx_fill_rect(int x, int y, int w, int h, uint8_t color);
void     gfx_rect(int x, int y, int w, int h, uint8_t color);
void     gfx_line(int x0, int y0, int x1, int y1, uint8_t color);
void     gfx_circle(int cx, int cy, int r, uint8_t color);
void     gfx_clear(uint8_t color);
void     gfx_text_mode(void);
void     gfx_graphics_mode(void);

#endif
