#ifndef TYPES_H
#define TYPES_H

typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;
typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed int          int32_t;
typedef signed long long    int64_t;
typedef uint32_t            size_t;
typedef int32_t             ssize_t;
typedef int32_t             pid_t;
typedef uint32_t            addr_t;
typedef unsigned char       bool;
#define true  1
#define false 0
#define NULL  ((void*)0)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ALIGN_UP(x, a)   (((x) + (a) - 1) & ~((a) - 1))
#define ALIGN_DOWN(x, a) ((x) & ~((a) - 1))
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define UNUSED __attribute__((unused))
#define PACKED __attribute__((packed))
#define NORETURN __attribute__((noreturn))
#define INLINE static inline

#endif
