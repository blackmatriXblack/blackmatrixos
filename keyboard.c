#include "../include/keyboard.h"
#include "../include/io.h"
#include "../include/idt.h"
#include "../include/pic.h"
#include "../include/pic.h"
#include "../include/vga.h"

static keyboard_state_t kb_state;

/* US QWERTY scancode to ASCII (no shift) */
static const char scancode_ascii[] = {
    0, KEY_ESCAPE, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', KEY_BACKSPACE,
    KEY_TAB, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', KEY_ENTER,
    KEY_CTRL, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    KEY_LSHIFT, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', KEY_RSHIFT,
    '*', KEY_ALT, ' ', KEY_CAPSLOCK,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,
};

/* Shifted scancode to ASCII */
static const char scancode_shift[] = {
    0, KEY_ESCAPE, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', KEY_BACKSPACE,
    KEY_TAB, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', KEY_ENTER,
    KEY_CTRL, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    KEY_LSHIFT, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', KEY_RSHIFT,
    '*', KEY_ALT, ' ', KEY_CAPSLOCK,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,
};

static void kb_buffer_put(uint8_t scancode) {
    if (kb_state.count < KB_BUFFER_SIZE) {
        kb_state.buffer[kb_state.head] = scancode;
        kb_state.head = (kb_state.head + 1) % KB_BUFFER_SIZE;
        kb_state.count++;
    }
}

static void keyboard_irq_handler(interrupt_frame_t* frame UNUSED) {
    uint8_t scancode = inb(KB_DATA_PORT);

    /* Key release */
    if (scancode & 0x80) {
        uint8_t released = scancode & 0x7F;
        if (released == KEY_LSHIFT || released == KEY_RSHIFT) kb_state.shift = false;
        if (released == KEY_CTRL) kb_state.ctrl = false;
        if (released == KEY_ALT) kb_state.alt = false;
        return;
    }

    /* Key press */
    switch (scancode) {
        case KEY_LSHIFT:
        case KEY_RSHIFT:
            kb_state.shift = true;
            return;
        case KEY_CTRL:
            kb_state.ctrl = true;
            return;
        case KEY_ALT:
            kb_state.alt = true;
            return;
        case KEY_CAPSLOCK:
            kb_state.capslock = !kb_state.capslock;
            return;
    }

    /* Convert scancode to ASCII */
    char c;
    if (kb_state.shift) {
        c = (scancode < sizeof(scancode_shift)) ? scancode_shift[scancode] : 0;
    } else {
        c = (scancode < sizeof(scancode_ascii)) ? scancode_ascii[scancode] : 0;
    }

    /* Caps lock handling */
    if (kb_state.capslock && c >= 'a' && c <= 'z') {
        c -= 32;
    } else if (kb_state.capslock && c >= 'A' && c <= 'Z') {
        c += 32;
    }

    /* Ctrl+C */
    if (kb_state.ctrl && c == 'c') {
        c = 0x03;
    }
    /* Ctrl+L (clear screen) */
    if (kb_state.ctrl && c == 'l') {
        c = 0x0C;
    }

    if (c) {
        kb_buffer_put(c);
    }
}

void keyboard_init(void) {
    kb_state.head = 0;
    kb_state.tail = 0;
    kb_state.count = 0;
    kb_state.shift = false;
    kb_state.ctrl = false;
    kb_state.alt = false;
    kb_state.capslock = false;

    register_interrupt_handler(33, keyboard_irq_handler);
    pic_unmask(1);  /* Unmask IRQ1 (keyboard) */
}

char keyboard_getchar(void) {
    while (kb_state.count == 0) {
        __asm__ volatile ("hlt");
    }
    char c = kb_state.buffer[kb_state.tail];
    kb_state.tail = (kb_state.tail + 1) % KB_BUFFER_SIZE;
    kb_state.count--;
    return c;
}

bool keyboard_has_input(void) {
    return kb_state.count > 0;
}

char keyboard_read_nonblocking(void) {
    if (kb_state.count == 0) return 0;
    char c = kb_state.buffer[kb_state.tail];
    kb_state.tail = (kb_state.tail + 1) % KB_BUFFER_SIZE;
    kb_state.count--;
    return c;
}

uint8_t keyboard_get_scancode(void) {
    return inb(KB_DATA_PORT);
}
