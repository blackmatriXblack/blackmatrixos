#include "../include/gdt.h"
#include "../include/string.h"
#include "../include/io.h"

static gdt_entry_t gdt_entries[GDT_ENTRIES];
static gdt_ptr_t   gdt_ptr;
static tss_entry_t tss;

extern void gdt_flush(uint32_t);
extern void tss_flush(void);

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low    = base & 0xFFFF;
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;
    gdt_entries[num].limit_low   = limit & 0xFFFF;
    gdt_entries[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt_entries[num].access      = access;
}

static void write_tss(int num, uint16_t ss0, uint32_t esp0) {
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = base + sizeof(tss_entry_t);
    gdt_set_gate(num, base, limit, 0xE9, 0x00);
    memset(&tss, 0, sizeof(tss_entry_t));
    tss.ss0 = ss0;
    tss.esp0 = esp0;
    tss.cs = 0x08 | 0x03;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x10 | 0x03;
}

void gdt_init(void) {
    gdt_ptr.limit = sizeof(gdt_entry_t) * GDT_ENTRIES - 1;
    gdt_ptr.base  = (uint32_t)&gdt_entries;

    gdt_set_gate(0, 0, 0, 0, 0);                /* Null segment */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); /* Kernel code */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); /* Kernel data */
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); /* User code */
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); /* User data */
    write_tss(5, 0x10, 0);

    gdt_flush((uint32_t)&gdt_ptr);
    tss_flush();
}

void tss_init(void) {
    /* Already initialized in gdt_init */
}

void tss_set_stack(uint32_t kernel_esp) {
    tss.esp0 = kernel_esp;
}
