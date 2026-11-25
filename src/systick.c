#include <stdint.h>
#include "stm32f407.h"
#include "clock.h"
#include "systick.h"

/* written by the interrupt, read by everyone else; volatile so delay_ms rereads it every pass */
static volatile uint32_t ticks;

/* overrides the weak alias in startup.c, so the vector table points here now */
void SysTick_Handler(void)
{
    ticks++;
}

void systick_init(void)
{
    /* it counts LOAD down to 0 and reloads, so one period is LOAD + 1 cycles; 16000 cycles at 16 MHz is 1 ms */
    _Static_assert(SYSCLK_HZ / 1000 - 1 <= SYSTICK_LOAD_MAX, "1 ms doesn't fit in SysTick's 24 bits");
    SYSTICK->LOAD = SYSCLK_HZ / 1000 - 1;

    /* any write clears the current count, so the first tick is a full one */
    SYSTICK->VAL = 0;

    SYSTICK->CTRL = SYSTICK_CTRL_CLKSOURCE | SYSTICK_CTRL_TICKINT | SYSTICK_CTRL_ENABLE;
}

/* a 32 bit load is a single instruction on the M4, so no need to mask interrupts around it */
uint32_t millis(void)
{
    return ticks;
}

void delay_ms(uint32_t ms)
{
    uint32_t start = ticks;

    /* start can land anywhere inside a tick, so wait one extra to guarantee at least ms */
    ms++;

    /* unsigned subtraction still gives the right answer after ticks wraps past 0 */
    while ((uint32_t)(ticks - start) < ms) {
    }
}
