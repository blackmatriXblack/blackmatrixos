#ifndef TIMER_H
#define TIMER_H

#include "types.h"

#define PIT_CH0  0x40
#define PIT_CMD  0x43
#define PIT_FREQ 1193180
#define TICK_HZ  100

void     timer_init(uint32_t frequency);
uint32_t timer_get_ticks(void);
uint32_t timer_get_seconds(void);
void     timer_sleep(uint32_t ms);
void     timer_register_callback(void (*callback)(void));

#endif
