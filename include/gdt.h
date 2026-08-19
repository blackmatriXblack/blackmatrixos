#ifndef GDT_H
#define GDT_H

#include "types.h"

#define GDT_ENTRIES 6

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} PACKED gdt_entry_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} PACKED gdt_ptr_t;

typedef struct {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1, ss1;
    uint32_t esp2, ss2;
    uint32_t cr3, eip, eflags;
    uint32_t eax, ecx, edx, ebx;
    uint32_t esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} PACKED tss_entry_t;

void gdt_init(void);
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
void tss_init(void);
void tss_set_stack(uint32_t kernel_esp);

#endif
