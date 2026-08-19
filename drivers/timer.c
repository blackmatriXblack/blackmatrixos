#include "../include/timer.h"
#include "../include/io.h"
#include "../include/idt.h"
#include "../include/pic.h"
#include "../include/vga.h"

static volatile uint32_t tick_count = 0;
static void (*timer_callback)(void) = NULL;

static void timer_irq_handler(interrupt_frame_t* frame UNUSED) {
    tick_count++;
    if (timer_callback) {
        timer_callback();
    }
}

void timer_init(uint32_t frequency) {
    tick_count = 0;

    register_interrupt_handler(32, timer_irq_handler);

    /* Configure PIT channel 0 */
    uint32_t divisor = PIT_FREQ / frequency;
    outb(PIT_CMD, 0x36);  /* Channel 0, lobyte/hibyte, square wave */
    outb(PIT_CH0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CH0, (uint8_t)((divisor >> 8) & 0xFF));

    pic_unmask(0);  /* Unmask IRQ0 (timer) */
}

uint32_t timer_get_ticks(void) {
    return tick_count;
}

uint32_t timer_get_seconds(void) {
    return tick_count / TICK_HZ;
}

void timer_sleep(uint32_t ms) {
    uint32_t target = tick_count + (ms * TICK_HZ / 1000);
    while (tick_count < target) {
        __asm__ volatile ("hlt");
    }
}

void timer_register_callback(void (*callback)(void)) {
    timer_callback = callback;
}
