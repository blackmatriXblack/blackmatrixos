#include "../include/stdio.h"
#include "../include/string.h"
#include "../include/vga.h"
#include "../include/stdarg.h"

/* Simple vsprintf implementation */
static int vsprintf_helper(char* buf, const char* fmt, va_list args) {
    char* start = buf;
    char temp[64];
    int pad_zero = 0;
    int pad_width = 0;
    int long_flag = 0;

    while (*fmt) {
        if (*fmt != '%') {
            *buf++ = *fmt++;
            continue;
        }
        fmt++;

        /* Parse flags */
        pad_zero = 0;
        pad_width = 0;
        long_flag = 0;

        if (*fmt == '0') { pad_zero = 1; fmt++; }
        while (*fmt >= '0' && *fmt <= '9') {
            pad_width = pad_width * 10 + (*fmt - '0');
            fmt++;
        }
        if (*fmt == 'l') { long_flag = 1; fmt++; }

        switch (*fmt) {
            case 'd':
            case 'i': {
                int val = long_flag ? va_arg(args, long) : va_arg(args, int);
                itoa(val, temp, 10);
                int len = strlen(temp);
                if (val < 0) {
                    *buf++ = '-';
                    len--;
                    memmove(temp, temp + 1, len + 1);
                }
                for (int i = len; i < pad_width; i++) *buf++ = pad_zero ? '0' : ' ';
                strcpy(buf, temp);
                buf += len;
                break;
            }
            case 'u': {
                uint32_t val = long_flag ? va_arg(args, unsigned long) : va_arg(args, uint32_t);
                utoa(val, temp, 10);
                int len = strlen(temp);
                for (int i = len; i < pad_width; i++) *buf++ = pad_zero ? '0' : ' ';
                strcpy(buf, temp);
                buf += len;
                break;
            }
            case 'x': {
                uint32_t val = long_flag ? va_arg(args, unsigned long) : va_arg(args, uint32_t);
                utoa(val, temp, 16);
                int len = strlen(temp);
                for (int i = len; i < pad_width; i++) *buf++ = pad_zero ? '0' : ' ';
                strcpy(buf, temp);
                buf += len;
                break;
            }
            case 'X': {
                uint32_t val = long_flag ? va_arg(args, unsigned long) : va_arg(args, uint32_t);
                utoa(val, temp, 16);
                for (int i = 0; temp[i]; i++) {
                    if (temp[i] >= 'a' && temp[i] <= 'f') temp[i] -= 32;
                }
                int len = strlen(temp);
                for (int i = len; i < pad_width; i++) *buf++ = pad_zero ? '0' : ' ';
                strcpy(buf, temp);
                buf += len;
                break;
            }
            case 'o': {
                uint32_t val = va_arg(args, uint32_t);
                utoa(val, temp, 8);
                int len = strlen(temp);
                strcpy(buf, temp);
                buf += len;
                break;
            }
            case 'p': {
                uint32_t val = va_arg(args, uint32_t);
                *buf++ = '0'; *buf++ = 'x';
                utoa(val, temp, 16);
                int len = strlen(temp);
                for (int i = len; i < 8; i++) *buf++ = '0';
                strcpy(buf, temp);
                buf += len;
                break;
            }
            case 'c': {
                char c = (char)va_arg(args, int);
                *buf++ = c;
                break;
            }
            case 's': {
                const char* s = va_arg(args, const char*);
                if (!s) s = "(null)";
                int len = strlen(s);
                for (int i = len; i < pad_width; i++) *buf++ = ' ';
                strcpy(buf, s);
                buf += len;
                break;
            }
            case '%': {
                *buf++ = '%';
                break;
            }
            default:
                *buf++ = '%';
                *buf++ = *fmt;
                break;
        }
        fmt++;
    }
    *buf = '\0';
    return buf - start;
}

int printf(const char* fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    int len = vsprintf_helper(buf, fmt, args);
    va_end(args);
    vga_puts(buf);
    return len;
}

int sprintf(char* buf, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int len = vsprintf_helper(buf, fmt, args);
    va_end(args);
    return len;
}

void kprintf(const char* fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsprintf_helper(buf, fmt, args);
    va_end(args);
    vga_puts(buf);
}

void kprint(const char* s) {
    vga_puts(s);
}

void kprint_hex(uint32_t val) {
    char buf[16];
    utoa(val, buf, 16);
    vga_puts("0x");
    vga_puts(buf);
}

void kprint_dec(int val) {
    char buf[16];
    itoa(val, buf, 10);
    vga_puts(buf);
}

void kclear(void) {
    vga_clear();
}

void kset_color(uint8_t color) {
    vga_set_color(color);
}
