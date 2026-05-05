#include "../include/gui.h"
#include "../include/vga.h"
#include "../include/memory.h"
#include "../include/string.h"
#include "../include/stdio.h"
#include "../include/timer.h"
#include "../include/keyboard.h"

/* 8x8 bitmap font (ASCII 32-127) - simplified */
static const uint8_t font8x8[96][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, /* space */
    {0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00}, /* ! */
    {0x6C,0x6C,0x24,0x00,0x00,0x00,0x00,0x00}, /* " */
    {0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00}, /* # */
    {0x18,0x7E,0xC0,0x7C,0x06,0xFC,0x18,0x00}, /* $ */
    {0x00,0xC6,0xCC,0x18,0x30,0x66,0xC6,0x00}, /* % */
    {0x38,0x6C,0x38,0x76,0xDC,0xCC,0x76,0x00}, /* & */
    {0x18,0x18,0x30,0x00,0x00,0x00,0x00,0x00}, /* ' */
    {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00}, /* ( */
    {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00}, /* ) */
    {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00}, /* * */
    {0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00}, /* + */
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30}, /* , */
    {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00}, /* - */
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00}, /* . */
    {0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00}, /* / */
    {0x7C,0xC6,0xCE,0xD6,0xE6,0xC6,0x7C,0x00}, /* 0 */
    {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00}, /* 1 */
    {0x7C,0xC6,0x06,0x1C,0x30,0x66,0xFE,0x00}, /* 2 */
    {0x7C,0xC6,0x06,0x3C,0x06,0xC6,0x7C,0x00}, /* 3 */
    {0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x1E,0x00}, /* 4 */
    {0xFE,0xC0,0xFC,0x06,0x06,0xC6,0x7C,0x00}, /* 5 */
    {0x38,0x60,0xC0,0xFC,0xC6,0xC6,0x7C,0x00}, /* 6 */
    {0xFE,0xC6,0x0C,0x18,0x30,0x30,0x30,0x00}, /* 7 */
    {0x7C,0xC6,0xC6,0x7C,0xC6,0xC6,0x7C,0x00}, /* 8 */
    {0x7C,0xC6,0xC6,0x7E,0x06,0x0C,0x78,0x00}, /* 9 */
    {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00}, /* : */
    {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x30}, /* ; */
    {0x06,0x0C,0x18,0x30,0x18,0x0C,0x06,0x00}, /* < */
    {0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00}, /* = */
    {0x60,0x30,0x18,0x0C,0x18,0x30,0x60,0x00}, /* > */
    {0x7C,0xC6,0x0C,0x18,0x18,0x00,0x18,0x00}, /* ? */
    {0x7C,0xC6,0xDE,0xDE,0xDE,0xC0,0x78,0x00}, /* @ */
    {0x38,0x6C,0xC6,0xC6,0xFE,0xC6,0xC6,0x00}, /* A */
    {0xFC,0x66,0x66,0x7C,0x66,0x66,0xFC,0x00}, /* B */
    {0x3C,0x66,0xC0,0xC0,0xC0,0x66,0x3C,0x00}, /* C */
    {0xF8,0x6C,0x66,0x66,0x66,0x6C,0xF8,0x00}, /* D */
    {0xFE,0x62,0x68,0x78,0x68,0x62,0xFE,0x00}, /* E */
    {0xFE,0x62,0x68,0x78,0x68,0x60,0xF0,0x00}, /* F */
    {0x3C,0x66,0xC0,0xC0,0xCE,0x66,0x3E,0x00}, /* G */
    {0xC6,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0x00}, /* H */
    {0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00}, /* I */
    {0x1E,0x0C,0x0C,0x0C,0xCC,0xCC,0x78,0x00}, /* J */
    {0xE6,0x66,0x6C,0x78,0x6C,0x66,0xE6,0x00}, /* K */
    {0xF0,0x60,0x60,0x60,0x62,0x66,0xFE,0x00}, /* L */
    {0xC6,0xEE,0xFE,0xD6,0xC6,0xC6,0xC6,0x00}, /* M */
    {0xC6,0xE6,0xF6,0xDE,0xCE,0xC6,0xC6,0x00}, /* N */
    {0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00}, /* O */
    {0xFC,0x66,0x66,0x7C,0x60,0x60,0xF0,0x00}, /* P */
    {0x7C,0xC6,0xC6,0xC6,0xD6,0xDE,0x7C,0x0E}, /* Q */
    {0xFC,0x66,0x66,0x7C,0x6C,0x66,0xE6,0x00}, /* R */
    {0x7C,0xC6,0xC0,0x7C,0x06,0xC6,0x7C,0x00}, /* S */
    {0x7E,0x5A,0x18,0x18,0x18,0x18,0x3C,0x00}, /* T */
    {0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00}, /* U */
    {0xC6,0xC6,0xC6,0xC6,0x6C,0x38,0x10,0x00}, /* V */
    {0xC6,0xC6,0xC6,0xD6,0xFE,0xEE,0xC6,0x00}, /* W */
    {0xC6,0xC6,0x6C,0x38,0x6C,0xC6,0xC6,0x00}, /* X */
    {0x66,0x66,0x66,0x3C,0x18,0x18,0x3C,0x00}, /* Y */
    {0xFE,0xC6,0x8C,0x18,0x32,0x66,0xFE,0x00}, /* Z */
    {0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00}, /* [ */
    {0xC0,0x60,0x30,0x18,0x0C,0x06,0x02,0x00}, /* backslash */
    {0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00}, /* ] */
    {0x10,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00}, /* ^ */
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE}, /* _ */
    {0x30,0x18,0x0C,0x00,0x00,0x00,0x00,0x00}, /* ` */
    {0x00,0x00,0x78,0x0C,0x7C,0xCC,0x76,0x00}, /* a */
    {0xE0,0x60,0x7C,0x66,0x66,0x66,0xDC,0x00}, /* b */
    {0x00,0x00,0x7C,0xC6,0xC0,0xC6,0x7C,0x00}, /* c */
    {0x1C,0x0C,0x7C,0xCC,0xCC,0xCC,0x76,0x00}, /* d */
    {0x00,0x00,0x7C,0xC6,0xFE,0xC0,0x7C,0x00}, /* e */
    {0x1C,0x36,0x30,0x78,0x30,0x30,0x78,0x00}, /* f */
    {0x00,0x00,0x76,0xCC,0xCC,0x7C,0x0C,0x78}, /* g */
    {0xE0,0x60,0x6C,0x76,0x66,0x66,0xE6,0x00}, /* h */
    {0x18,0x00,0x38,0x18,0x18,0x18,0x3C,0x00}, /* i */
    {0x06,0x00,0x0E,0x06,0x06,0x66,0x66,0x3C}, /* j */
    {0xE0,0x60,0x66,0x6C,0x78,0x6C,0xE6,0x00}, /* k */
    {0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00}, /* l */
    {0x00,0x00,0xEC,0xFE,0xD6,0xC6,0xC6,0x00}, /* m */
    {0x00,0x00,0xDC,0x66,0x66,0x66,0x66,0x00}, /* n */
    {0x00,0x00,0x7C,0xC6,0xC6,0xC6,0x7C,0x00}, /* o */
    {0x00,0x00,0xDC,0x66,0x66,0x7C,0x60,0xF0}, /* p */
    {0x00,0x00,0x76,0xCC,0xCC,0x7C,0x0C,0x1E}, /* q */
    {0x00,0x00,0xDC,0x76,0x60,0x60,0xF0,0x00}, /* r */
    {0x00,0x00,0x7C,0xC0,0x7C,0x06,0xFC,0x00}, /* s */
    {0x30,0x30,0x7C,0x30,0x30,0x36,0x1C,0x00}, /* t */
    {0x00,0x00,0xCC,0xCC,0xCC,0xCC,0x76,0x00}, /* u */
    {0x00,0x00,0xC6,0xC6,0xC6,0x6C,0x38,0x00}, /* v */
    {0x00,0x00,0xC6,0xC6,0xD6,0xFE,0x6C,0x00}, /* w */
    {0x00,0x00,0xC6,0x6C,0x38,0x6C,0xC6,0x00}, /* x */
    {0x00,0x00,0xC6,0xC6,0xC6,0x7E,0x06,0xFC}, /* y */
    {0x00,0x00,0xFE,0x8C,0x18,0x32,0xFE,0x00}, /* z */
    {0x1C,0x30,0x30,0x60,0x30,0x30,0x1C,0x00}, /* { */
    {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00}, /* | */
    {0x38,0x0C,0x0C,0x06,0x0C,0x0C,0x38,0x00}, /* } */
    {0x00,0x76,0xDC,0x00,0x00,0x00,0x00,0x00}, /* ~ */
};

/* Window management */
static window_t windows[GUI_MAX_WINDOWS];
static int focused_window = -1;
static int window_count = 0;
static int next_widget_id = 1;

/* Desktop icons */
static desktop_icon_t desktop_icons[16];
static int icon_count = 0;

/* ==============================================================================
 * Drawing Primitives
 * ============================================================================== */

void gui_draw_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < GUI_SCREEN_W && y >= 0 && y < GUI_SCREEN_H) {
        GFX_MEM[y * GUI_SCREEN_W + x] = color;
    }
}

void gui_draw_rect(int x, int y, int w, int h, uint8_t color) {
    for (int i = 0; i < w; i++) {
        gui_draw_pixel(x + i, y, color);
        gui_draw_pixel(x + i, y + h - 1, color);
    }
    for (int i = 0; i < h; i++) {
        gui_draw_pixel(x, y + i, color);
        gui_draw_pixel(x + w - 1, y + i, color);
    }
}

void gui_draw_filled_rect(int x, int y, int w, int h, uint8_t color) {
    for (int dy = 0; dy < h; dy++) {
        for (int dx = 0; dx < w; dx++) {
            gui_draw_pixel(x + dx, y + dy, color);
        }
    }
}

void gui_draw_line(int x0, int y0, int x1, int y1, uint8_t color) {
    int dx = (x1 > x0) ? x1 - x0 : x0 - x1;
    int dy = (y1 > y0) ? y1 - y0 : y0 - y1;
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    for (;;) {
        gui_draw_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }
}

void gui_draw_char(int x, int y, char c, uint8_t fg, uint8_t bg) {
    if (c < 32 || c > 127) c = '?';
    const uint8_t* glyph = font8x8[c - 32];
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (glyph[row] & (1 << col)) {
                gui_draw_pixel(x + col, y + row, fg);
            } else if (bg != 0xFF) {
                gui_draw_pixel(x + col, y + row, bg);
            }
        }
    }
}

void gui_draw_string(int x, int y, const char* s, uint8_t fg, uint8_t bg) {
    while (*s) {
        gui_draw_char(x, y, *s, fg, bg);
        x += 8;
        s++;
    }
}

void gui_draw_button(int x, int y, int w, int h, const char* text, bool pressed) {
    uint8_t color = pressed ? GUI_DARK_GRAY : GUI_LIGHT_GRAY;
    gui_draw_filled_rect(x, y, w, h, color);
    gui_draw_rect(x, y, w, h, GUI_WHITE);
    gui_draw_line(x, y, x + w - 1, y, pressed ? GUI_DARK_GRAY : GUI_WHITE);
    gui_draw_line(x, y, x, y + h - 1, pressed ? GUI_DARK_GRAY : GUI_WHITE);

    int text_x = x + (w - strlen(text) * 8) / 2;
    int text_y = y + (h - 8) / 2;
    gui_draw_string(text_x, text_y, text, GUI_BLACK, 0xFF);
}

/* ==============================================================================
 * Window Management
 * ============================================================================== */

int gui_create_window(const char* title, int x, int y, int w, int h) {
    for (int i = 0; i < GUI_MAX_WINDOWS; i++) {
        if (!windows[i].visible) {
            window_t* win = &windows[i];
            win->id = i;
            strncpy(win->title, title, sizeof(win->title) - 1);
            win->x = x; win->y = y;
            win->w = w; win->h = h;
            win->visible = true;
            win->focused = true;
            win->minimized = false;
            win->dragging = false;
            win->bg_color = GUI_BG_COLOR;
            win->widgets = NULL;
            win->onpaint = NULL;
            win->onclose = NULL;
            if (focused_window >= 0) windows[focused_window].focused = false;
            focused_window = i;
            window_count++;
            return i;
        }
    }
    return -1;
}

void gui_close_window(int win_id) {
    if (win_id < 0 || win_id >= GUI_MAX_WINDOWS) return;
    window_t* win = &windows[win_id];
    if (win->onclose) win->onclose(win_id);
    widget_t* w = win->widgets;
    while (w) {
        widget_t* next = w->next;
        kfree(w);
        w = next;
    }
    win->visible = false;
    win->widgets = NULL;
    window_count--;
}

void gui_focus_window(int win_id) {
    if (win_id < 0 || win_id >= GUI_MAX_WINDOWS) return;
    if (focused_window >= 0) windows[focused_window].focused = false;
    windows[win_id].focused = true;
    focused_window = win_id;
}

window_t* gui_get_window(int win_id) {
    if (win_id < 0 || win_id >= GUI_MAX_WINDOWS) return NULL;
    return &windows[win_id];
}

void gui_show_window(int win_id, bool show) {
    if (win_id >= 0 && win_id < GUI_MAX_WINDOWS) windows[win_id].visible = show;
}

void gui_move_window(int win_id, int x, int y) {
    if (win_id >= 0 && win_id < GUI_MAX_WINDOWS) {
        windows[win_id].x = x;
        windows[win_id].y = y;
    }
}

/* ==============================================================================
 * Widget Management
 * ============================================================================== */

int gui_add_label(int win_id, int x, int y, const char* text) {
    window_t* win = gui_get_window(win_id);
    if (!win) return -1;
    widget_t* w = (widget_t*)kmalloc(sizeof(widget_t));
    if (!w) return -1;
    memset(w, 0, sizeof(widget_t));
    w->id = next_widget_id++;
    w->type = WIDGET_LABEL;
    w->x = x; w->y = y;
    w->w = strlen(text) * 8; w->h = 8;
    strncpy(w->text, text, sizeof(w->text) - 1);
    w->visible = true;
    w->fg_color = GUI_BLACK;
    w->next = win->widgets;
    win->widgets = w;
    return w->id;
}

int gui_add_button(int win_id, int x, int y, int w, int h, const char* text, void (*cb)(widget_t*)) {
    window_t* win = gui_get_window(win_id);
    if (!win) return -1;
    widget_t* wg = (widget_t*)kmalloc(sizeof(widget_t));
    if (!wg) return -1;
    memset(wg, 0, sizeof(widget_t));
    wg->id = next_widget_id++;
    wg->type = WIDGET_BUTTON;
    wg->x = x; wg->y = y;
    wg->w = w; wg->h = h;
    strncpy(wg->text, text, sizeof(wg->text) - 1);
    wg->visible = true;
    wg->bg_color = GUI_LIGHT_GRAY;
    wg->fg_color = GUI_BLACK;
    wg->onclick = cb;
    wg->next = win->widgets;
    win->widgets = wg;
    return wg->id;
}

int gui_add_textbox(int win_id, int x, int y, int w, char* text) {
    window_t* win = gui_get_window(win_id);
    if (!win) return -1;
    widget_t* wg = (widget_t*)kmalloc(sizeof(widget_t));
    if (!wg) return -1;
    memset(wg, 0, sizeof(widget_t));
    wg->id = next_widget_id++;
    wg->type = WIDGET_TEXTBOX;
    wg->x = x; wg->y = y;
    wg->w = w; wg->h = 12;
    if (text) strncpy(wg->text, text, sizeof(wg->text) - 1);
    wg->visible = true;
    wg->bg_color = GUI_WHITE;
    wg->fg_color = GUI_BLACK;
    wg->next = win->widgets;
    win->widgets = wg;
    return wg->id;
}

int gui_add_checkbox(int win_id, int x, int y, const char* label, bool checked) {
    window_t* win = gui_get_window(win_id);
    if (!win) return -1;
    widget_t* wg = (widget_t*)kmalloc(sizeof(widget_t));
    if (!wg) return -1;
    memset(wg, 0, sizeof(widget_t));
    wg->id = next_widget_id++;
    wg->type = WIDGET_CHECKBOX;
    wg->x = x; wg->y = y;
    wg->w = 12 + strlen(label) * 8; wg->h = 12;
    strncpy(wg->text, label, sizeof(wg->text) - 1);
    wg->visible = true;
    wg->value = checked ? 1 : 0;
    wg->next = win->widgets;
    win->widgets = wg;
    return wg->id;
}

int gui_add_progressbar(int win_id, int x, int y, int w, int h, int value) {
    window_t* win = gui_get_window(win_id);
    if (!win) return -1;
    widget_t* wg = (widget_t*)kmalloc(sizeof(widget_t));
    if (!wg) return -1;
    memset(wg, 0, sizeof(widget_t));
    wg->id = next_widget_id++;
    wg->type = WIDGET_PROGRESSBAR;
    wg->x = x; wg->y = y;
    wg->w = w; wg->h = h;
    wg->visible = true;
    wg->value = value;
    wg->bg_color = GUI_DARK_GRAY;
    wg->fg_color = GUI_LIME;
    wg->next = win->widgets;
    win->widgets = wg;
    return wg->id;
}

void gui_set_widget_text(int win_id, int widget_id, const char* text) {
    widget_t* w = gui_get_widget(win_id, widget_id);
    if (w) strncpy(w->text, text, sizeof(w->text) - 1);
}

void gui_set_progress(int win_id, int widget_id, int value) {
    widget_t* w = gui_get_widget(win_id, widget_id);
    if (w) w->value = value;
}

widget_t* gui_get_widget(int win_id, int widget_id) {
    window_t* win = gui_get_window(win_id);
    if (!win) return NULL;
    widget_t* w = win->widgets;
    while (w) {
        if (w->id == widget_id) return w;
        w = w->next;
    }
    return NULL;
}

/* ==============================================================================
 * Window Drawing
 * ============================================================================== */

static void draw_window(window_t* win) {
    if (!win->visible || win->minimized) return;

    /* Shadow */
    gui_draw_filled_rect(win->x + 2, win->y + 2, win->w, win->h, GUI_BLACK);

    /* Window body */
    gui_draw_filled_rect(win->x, win->y, win->w, win->h, win->bg_color);

    /* Title bar */
    uint8_t title_color = win->focused ? GUI_TITLE_BG : GUI_DARK_GRAY;
    gui_draw_filled_rect(win->x, win->y, win->w, GUI_TITLE_H, title_color);

    /* Title text */
    gui_draw_string(win->x + 4, win->y + 3, win->title, GUI_TITLE_FG, 0xFF);

    /* Close button */
    gui_draw_filled_rect(win->x + win->w - 14, win->y + 2, 10, 10, GUI_RED);
    gui_draw_string(win->x + win->w - 12, win->y + 3, "x", GUI_WHITE, 0xFF);

    /* Border */
    gui_draw_rect(win->x, win->y, win->w, win->h, GUI_WHITE);

    /* Draw widgets */
    widget_t* w = win->widgets;
    while (w) {
        if (w->visible) {
            int wx = win->x + w->x;
            int wy = win->y + GUI_TITLE_H + w->y;

            switch (w->type) {
                case WIDGET_LABEL:
                    gui_draw_string(wx, wy, w->text, w->fg_color, 0xFF);
                    break;
                case WIDGET_BUTTON:
                    gui_draw_button(wx, wy, w->w, w->h, w->text, w->pressed);
                    break;
                case WIDGET_TEXTBOX:
                    gui_draw_filled_rect(wx, wy, w->w, w->h, w->bg_color);
                    gui_draw_rect(wx, wy, w->w, w->h, GUI_DARK_GRAY);
                    gui_draw_string(wx + 2, wy + 2, w->text, w->fg_color, 0xFF);
                    break;
                case WIDGET_CHECKBOX:
                    gui_draw_rect(wx, wy, 10, 10, GUI_BLACK);
                    if (w->value) {
                        gui_draw_line(wx + 2, wy + 2, wx + 4, wy + 6, GUI_BLACK);
                        gui_draw_line(wx + 4, wy + 6, wx + 8, wy + 2, GUI_BLACK);
                    }
                    gui_draw_string(wx + 14, wy + 1, w->text, GUI_BLACK, 0xFF);
                    break;
                case WIDGET_PROGRESSBAR:
                    gui_draw_filled_rect(wx, wy, w->w, w->h, w->bg_color);
                    int bar_w = (w->w * w->value) / 100;
                    if (bar_w > 0) gui_draw_filled_rect(wx, wy, bar_w, w->h, w->fg_color);
                    gui_draw_rect(wx, wy, w->w, w->h, GUI_BLACK);
                    break;
                default: break;
            }
        }
        w = w->next;
    }
}

/* ==============================================================================
 * Desktop
 * ============================================================================== */

void gui_draw_desktop(void) {
    /* Desktop background */
    gui_draw_filled_rect(0, 0, GUI_SCREEN_W, GUI_SCREEN_H, GUI_DESKTOP_BG);

    /* Desktop icons */
    for (int i = 0; i < icon_count; i++) {
        int ix = desktop_icons[i].x;
        int iy = desktop_icons[i].y;
        gui_draw_filled_rect(ix, iy, 32, 32, GUI_LIGHT_GRAY);
        gui_draw_rect(ix, iy, 32, 32, GUI_WHITE);
        gui_draw_string(ix + 2, iy + 36, desktop_icons[i].label, GUI_WHITE, 0xFF);
    }
}

void gui_add_desktop_icon(const char* label, int x, int y, void (*cb)(void)) {
    if (icon_count >= 16) return;
    strncpy(desktop_icons[icon_count].label, label, sizeof(desktop_icons[0].label) - 1);
    desktop_icons[icon_count].x = x;
    desktop_icons[icon_count].y = y;
    desktop_icons[icon_count].onopen = cb;
    icon_count++;
}

void gui_draw_taskbar(void) {
    /* Taskbar background */
    gui_draw_filled_rect(0, GUI_SCREEN_H - 20, GUI_SCREEN_W, 20, GUI_DARK_GRAY);

    /* Start button */
    gui_draw_button(2, GUI_SCREEN_H - 18, 40, 16, "Start", false);

    /* Window buttons */
    int tx = 46;
    for (int i = 0; i < GUI_MAX_WINDOWS; i++) {
        if (windows[i].visible && !windows[i].minimized) {
            int tw = strlen(windows[i].title) * 8 + 8;
            if (tw > 80) tw = 80;
            gui_draw_button(tx, GUI_SCREEN_H - 18, tw, 16, windows[i].title, windows[i].focused);
            tx += tw + 2;
            if (tx > GUI_SCREEN_W - 50) break;
        }
    }

    /* Clock */
    gui_draw_string(GUI_SCREEN_W - 45, GUI_SCREEN_H - 14, "12:00", GUI_WHITE, 0xFF);
}

/* ==============================================================================
 * Built-in Applications
 * ============================================================================== */

void app_task_manager(void) {
    int win_id = gui_create_window("Task Manager", 40, 30, 200, 150);
    if (win_id < 0) return;
    gui_add_label(win_id, 10, 10, "Process List:");
    /* Would populate with process list */
}

void app_calculator(void) {
    int win_id = gui_create_window("Calculator", 80, 40, 160, 180);
    if (win_id < 0) return;
    gui_add_textbox(win_id, 10, 10, 140, "0");
    /* Add calculator buttons */
    const char* labels[] = {"7","8","9","+","4","5","6","-","1","2","3","*","0","C","=","/"};
    int bx = 10, by = 30;
    for (int i = 0; i < 16; i++) {
        gui_add_button(win_id, bx, by, 30, 20, labels[i], NULL);
        bx += 35;
        if (bx > 130) { bx = 10; by += 25; }
    }
}

void app_terminal(void) {
    int win_id = gui_create_window("Terminal", 20, 20, 260, 160);
    if (win_id < 0) return;
    gui_add_label(win_id, 10, 10, "CosmosOS Terminal v1.0");
    gui_add_label(win_id, 10, 25, "Type 'exit' to close.");
}

void app_system_monitor(void) {
    int win_id = gui_create_window("System Monitor", 50, 50, 200, 120);
    if (win_id < 0) return;
    gui_add_label(win_id, 10, 10, "CPU Usage:");
    gui_add_progressbar(win_id, 10, 25, 180, 12, 25);
    gui_add_label(win_id, 10, 45, "Memory:");
    gui_add_progressbar(win_id, 10, 60, 180, 12, 60);
}

/* ==============================================================================
 * Core GUI Loop
 * ============================================================================== */

void gui_init(void) {
    memset(windows, 0, sizeof(windows));
    memset(desktop_icons, 0, sizeof(desktop_icons));
    window_count = 0;
    icon_count = 0;
    focused_window = -1;
    next_widget_id = 1;

    /* Add desktop icons */
    gui_add_desktop_icon("Files", 20, 20, NULL);
    gui_add_desktop_icon("Terminal", 20, 70, NULL);
    gui_add_desktop_icon("Settings", 20, 120, NULL);

    /* Switch to graphics mode */
    gfx_graphics_mode();
}

void gui_render(void) {
    gui_draw_desktop();

    /* Draw windows from bottom to top */
    for (int i = 0; i < GUI_MAX_WINDOWS; i++) {
        if (windows[i].visible && !windows[i].minimized) {
            draw_window(&windows[i]);
        }
    }

    gui_draw_taskbar();
}

void gui_run(void) {
    gui_init();

    /* Create default windows */
    app_system_monitor();

    bool running = true;
    while (running) {
        gui_render();

        /* Handle input */
        if (keyboard_has_input()) {
            char c = keyboard_getchar();
            if (c == KEY_ESCAPE) {
                running = false;
            }
        }

        timer_sleep(33); /* ~30 FPS */
    }

    gui_shutdown();
}

void gui_shutdown(void) {
    for (int i = 0; i < GUI_MAX_WINDOWS; i++) {
        if (windows[i].visible) gui_close_window(i);
    }
    gfx_text_mode();
}
