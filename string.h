#ifndef STRING_H
#define STRING_H

#include "types.h"

void*  memcpy(void* dst, const void* src, size_t n);
void*  memset(void* dst, int val, size_t n);
void*  memmove(void* dst, const void* src, size_t n);
int    memcmp(const void* s1, const void* s2, size_t n);

size_t strlen(const char* s);
int    strcmp(const char* s1, const char* s2);
int    strncmp(const char* s1, const char* s2, size_t n);
char*  strcpy(char* dst, const char* src);
char*  strncpy(char* dst, const char* src, size_t n);
char*  strcat(char* dst, const char* src);
char*  strchr(const char* s, int c);
char*  strrchr(const char* s, int c);
char*  strstr(const char* haystack, const char* needle);
int    atoi(const char* s);
void   itoa(int value, char* str, int base);
void   utoa(uint32_t value, char* str, int base);

#endif
