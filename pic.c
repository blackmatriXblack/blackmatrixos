#include "../include/pic.h"
#include "../include/io.h"

void pic_init(void) {
    /* Save masks */
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);

    /* ICW1: Initialize + ICW4 needed */
    outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);
    io_wait();

    /* ICW2: Vector offsets */
    outb(PIC1_DATA, 0x20);  /* IRQ 0-7 -> INT 0x20-0x27 */
    io_wait();
    outb(PIC2_DATA, 0x28);  /* IRQ 8-15 -> INT 0x28-0x2F */
    io_wait();

    /* ICW3: Cascade wiring */
    outb(PIC1_DATA, 0x04);  /* Slave PIC at IRQ2 */
    io_wait();
    outb(PIC2_DATA, 0x02);  /* Slave cascade identity */
    io_wait();

    /* ICW4: 8086 mode */
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    /* Restore masks */
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_CMD, PIC_EOI);
    }
    outb(PIC1_CMD, PIC_EOI);
}

void pic_mask(uint8_t irq) {
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    if (irq >= 8) irq -= 8;
    outb(port, inb(port) | (1 << irq));
}

void pic_unmask(uint8_t irq) {
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    if (irq >= 8) irq -= 8;
    outb(port, inb(port) & ~(1 << irq));
}

uint16_t pic_get_irr(void) {
    outb(PIC1_CMD, 0x0A);
    outb(PIC2_CMD, 0x0A);
    return (inb(PIC2_CMD) << 8) | inb(PIC1_CMD);
}

uint16_t pic_get_isr(void) {
    outb(PIC1_CMD, 0x0B);
    outb(PIC2_CMD, 0x0B);
    return (inb(PIC2_CMD) << 8) | inb(PIC1_CMD);
}
