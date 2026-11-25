#include <stdint.h>
#include "stm32f407.h"
#include "systick.h"

#define LED_PIN  12   /* PD12, the green LED on the Discovery kit */
#define BLINK_MS 500

static void led_init(void)
{
    /* GPIOD has no clock after reset; writes to it would just get dropped */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

    /* the RM says wait a couple of cycles after enabling a clock; reading it back does that */
    (void)RCC->AHB1ENR;

    /* PD12 as general purpose output; push pull and low speed are already the reset defaults */
    GPIOD->MODER = (GPIOD->MODER & ~GPIO_MODER_MASK(LED_PIN))
                 | GPIO_MODER_VAL(LED_PIN, GPIO_MODE_OUTPUT);
}

int main(void)
{
    led_init();
    systick_init();

    while (1) {
        GPIOD->BSRR = GPIO_BSRR_SET(LED_PIN);
        delay_ms(BLINK_MS);
        GPIOD->BSRR = GPIO_BSRR_RESET(LED_PIN);
        delay_ms(BLINK_MS);
    }
}
