#ifndef STDIO_H
#define STDIO_H

#include "types.h"
#include "vga.h"

int  printf(const char* fmt, ...);
int  sprintf(char* buf, const char* fmt, ...);
void kprintf(const char* fmt, ...);
void kprint(const char* s);
void kprint_hex(uint32_t val);
void kprint_dec(int val);
void kclear(void);
void kset_color(uint8_t color);

#endif
