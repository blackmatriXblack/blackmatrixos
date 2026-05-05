#ifndef STDARG_H
#define STDARG_H

/* Minimal stdarg.h for freestanding environment */
typedef char* va_list;

#define __va_rounded_size(type) \
    (((sizeof(type) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#define va_start(ap, last) \
    (ap = (va_list)&(last) + __va_rounded_size(last))

#define va_arg(ap, type) \
    (*(type*)((ap += __va_rounded_size(type)) - __va_rounded_size(type)))

#define va_end(ap) \
    (ap = (va_list)0)

#endif
