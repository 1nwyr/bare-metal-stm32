#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

/* 1 ms tick off the core's SysTick timer */
void systick_init(void);

/* milliseconds since systick_init; wraps after about 49.7 days */
uint32_t millis(void);

/* busy waits at least ms milliseconds */
void delay_ms(uint32_t ms);

#endif
