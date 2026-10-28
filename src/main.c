#include <stdint.h>
#include "stm32f407.h"

#define LED_PIN 12  /* PD12, the green LED on the Discovery kit */

/* rough delay; how long it really takes depends on the clock and -O level, SysTick fixes that next */
static void delay(volatile uint32_t count)
{
    while (count--) {
    }
}

int main(void)
{
    /* GPIOD has no clock after reset; writes to it would just get dropped */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

    /* the RM says wait a couple of cycles after enabling a clock; reading it back does that */
    (void)RCC->AHB1ENR;

    /* PD12 as general purpose output; push pull and low speed are already the reset defaults */
    GPIOD->MODER = (GPIOD->MODER & ~GPIO_MODER_MASK(LED_PIN))
                 | GPIO_MODER_VAL(LED_PIN, GPIO_MODE_OUTPUT);

    while (1) {
        GPIOD->BSRR = GPIO_BSRR_SET(LED_PIN);
        delay(500000);
        GPIOD->BSRR = GPIO_BSRR_RESET(LED_PIN);
        delay(500000);
    }
}
