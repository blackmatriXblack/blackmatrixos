#ifndef GUI_H
#define GUI_H

#include "types.h"

/* GUI color palette (VGA 256-color mode) */
#define GUI_BLACK       0
#define GUI_BLUE        1
#define GUI_GREEN       2
#define GUI_CYAN        3
#define GUI_RED         4
#define GUI_MAGENTA     5
#define GUI_BROWN       6
#define GUI_LIGHT_GRAY  7
#define GUI_DARK_GRAY   8
#define GUI_LIGHT_BLUE  9
#define GUI_LIME        10
#define GUI_LIGHT_CYAN  11
#define GUI_LIGHT_RED   12
#define GUI_PINK        13
#define GUI_YELLOW      14
#define GUI_WHITE       15
#define GUI_BG_COLOR    0x07  /* Medium gray */
#define GUI_TITLE_BG    0x01  /* Blue */
#define GUI_TITLE_FG    0x0F  /* White */
#define GUI_DESKTOP_BG  0x1B  /* Dark blue */

#define GUI_SCREEN_W    320
#define GUI_SCREEN_H    200
#define GUI_MAX_WINDOWS 16
#define GUI_TITLE_H     14
#define GUI_BORDER_W    1

typedef enum {
    EVT_NONE = 0,
    EVT_KEY_PRESS,
    EVT_MOUSE_MOVE,
    EVT_MOUSE_CLICK,
    EVT_WINDOW_CLOSE,
    EVT_WINDOW_FOCUS,
    EVT_BUTTON_CLICK,
    EVT_TIMER,
} event_type_t;

typedef struct {
    event_type_t type;
    int          key;
    int          mouse_x;
    int          mouse_y;
    int          mouse_btn;
    int          window_id;
    int          widget_id;
} gui_event_t;

/* Widget types */
typedef enum {
    WIDGET_LABEL,
    WIDGET_BUTTON,
    WIDGET_TEXTBOX,
    WIDGET_CHECKBOX,
    WIDGET_PROGRESSBAR,
    WIDGET_LISTBOX,
    WIDGET_IMAGE,
} widget_type_t;

typedef struct widget {
    int             id;
    widget_type_t   type;
    int             x, y, w, h;
    char            text[128];
    bool            visible;
    bool            focused;
    uint8_t         bg_color;
    uint8_t         fg_color;
    bool            pressed;
    int             value;       /* For progressbar, checkbox */
    void (*onclick)(struct widget*);
    struct widget*  next;
} widget_t;

typedef struct {
    int          id;
    char         title[64];
    int          x, y, w, h;
    bool         visible;
    bool         focused;
    bool         minimized;
    bool         dragging;
    int          drag_ox, drag_oy;
    uint8_t      bg_color;
    widget_t*    widgets;
    void (*onpaint)(int win_id);
    void (*onclose)(int win_id);
} window_t;

/* Desktop */
typedef struct {
    char label[32];
    int  x, y;
    int  icon_id;
    void (*onopen)(void);
} desktop_icon_t;

/* Core GUI functions */
void     gui_init(void);
void     gui_run(void);
void     gui_shutdown(void);
void     gui_render(void);

/* Window management */
int      gui_create_window(const char* title, int x, int y, int w, int h);
void     gui_close_window(int win_id);
void     gui_show_window(int win_id, bool show);
void     gui_focus_window(int win_id);
void     gui_move_window(int win_id, int x, int y);
void     gui_resize_window(int win_id, int w, int h);
window_t* gui_get_window(int win_id);

/* Widget management */
int      gui_add_label(int win_id, int x, int y, const char* text);
int      gui_add_button(int win_id, int x, int y, int w, int h, const char* text, void (*cb)(widget_t*));
int      gui_add_textbox(int win_id, int x, int y, int w, char* text);
int      gui_add_checkbox(int win_id, int x, int y, const char* label, bool checked);
int      gui_add_progressbar(int win_id, int x, int y, int w, int h, int value);
void     gui_set_widget_text(int win_id, int widget_id, const char* text);
void     gui_set_progress(int win_id, int widget_id, int value);
widget_t* gui_get_widget(int win_id, int widget_id);

/* Drawing primitives */
void     gui_draw_pixel(int x, int y, uint8_t color);
void     gui_draw_rect(int x, int y, int w, int h, uint8_t color);
void     gui_draw_filled_rect(int x, int y, int w, int h, uint8_t color);
void     gui_draw_line(int x0, int y0, int x1, int y1, uint8_t color);
void     gui_draw_char(int x, int y, char c, uint8_t fg, uint8_t bg);
void     gui_draw_string(int x, int y, const char* s, uint8_t fg, uint8_t bg);
void     gui_draw_button(int x, int y, int w, int h, const char* text, bool pressed);

/* Built-in applications */
void     app_task_manager(void);
void     app_file_browser(void);
void     app_text_editor(void);
void     app_calculator(void);
void     app_terminal(void);
void     app_paint(void);
void     app_system_monitor(void);

/* Desktop */
void     gui_draw_desktop(void);
void     gui_add_desktop_icon(const char* label, int x, int y, void (*cb)(void));
void     gui_draw_taskbar(void);

#endif
