/*
 * BlackMatrixOS - 64-bit division helpers
 * Replaces libgcc runtime functions required by 32-bit code that
 * divides 64-bit values (freestanding build has no libgcc).
 */

#include "../include/types.h"

static uint64_t udiv64(uint64_t n, uint64_t d, uint64_t* rem) {
    uint64_t q = 0;
    uint64_t r = 0;
    if (d == 0) return 0xFFFFFFFFFFFFFFFF;
    for (int i = 63; i >= 0; i--) {
        r = (r << 1) | ((n >> i) & 1);
        if (r >= d) {
            r -= d;
            q |= ((uint64_t)1 << i);
        }
    }
    if (rem) *rem = r;
    return q;
}

int64_t __divdi3(int64_t n, int64_t d) {
    int neg = (n < 0) ^ (d < 0);
    uint64_t u = (n < 0) ? (uint64_t)(-n) : (uint64_t)n;
    uint64_t v = (d < 0) ? (uint64_t)(-d) : (uint64_t)d;
    uint64_t q = udiv64(u, v, NULL);
    return neg ? -(int64_t)q : (int64_t)q;
}

int64_t __moddi3(int64_t n, int64_t d) {
    int neg = (n < 0);
    uint64_t u = (n < 0) ? (uint64_t)(-n) : (uint64_t)n;
    uint64_t v = (d < 0) ? (uint64_t)(-d) : (uint64_t)d;
    uint64_t r = 0;
    udiv64(u, v, &r);
    return neg ? -(int64_t)r : (int64_t)r;
}

uint64_t __udivdi3(uint64_t n, uint64_t d) {
    return udiv64(n, d, NULL);
}

uint64_t __umoddi3(uint64_t n, uint64_t d) {
    uint64_t r = 0;
    udiv64(n, d, &r);
    return r;
}