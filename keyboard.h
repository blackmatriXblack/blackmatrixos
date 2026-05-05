#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"

#define KB_DATA_PORT   0x60
#define KB_STATUS_PORT 0x64
#define KB_BUFFER_SIZE 256

/* Key codes */
#define KEY_ESCAPE     0x1B
#define KEY_BACKSPACE  0x08
#define KEY_TAB        0x09
#define KEY_ENTER      0x0A
#define KEY_CTRL       0x1D
#define KEY_LSHIFT     0x2A
#define KEY_RSHIFT     0x36
#define KEY_ALT        0x38
#define KEY_CAPSLOCK   0x3A
#define KEY_F1         0x80
#define KEY_F2         0x81
#define KEY_F3         0x82
#define KEY_F4         0x83
#define KEY_F5         0x84
#define KEY_F6         0x85
#define KEY_F7         0x86
#define KEY_F8         0x87
#define KEY_F9         0x88
#define KEY_F10        0x89
#define KEY_F11        0x8A
#define KEY_F12        0x8B
#define KEY_UP         0x90
#define KEY_DOWN       0x91
#define KEY_LEFT       0x92
#define KEY_RIGHT      0x93
#define KEY_DELETE     0x94

typedef struct {
    uint8_t buffer[KB_BUFFER_SIZE];
    int head;
    int tail;
    int count;
    bool shift;
    bool ctrl;
    bool alt;
    bool capslock;
} keyboard_state_t;

void     keyboard_init(void);
char     keyboard_getchar(void);
bool     keyboard_has_input(void);
char     keyboard_read_nonblocking(void);
uint8_t  keyboard_get_scancode(void);

#endif
