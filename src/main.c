#include <stdint.h>
#include "stm32f407.h"
#include "clock.h"
#include "systick.h"
#include "uart.h"
#include "printf.h"

#define LED_PIN  12   /* PD12, the green LED on the Discovery kit */
#define BLINK_MS 500
#define BAUD     115200UL

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

static void led_set(int on)
{
    GPIOD->BSRR = on ? GPIO_BSRR_SET(LED_PIN) : GPIO_BSRR_RESET(LED_PIN);
}

int main(void)
{
    led_init();
    systick_init();
    uart_init(BAUD);

    uart_printf("\nbare-metal-stm32: sysclk %lu Hz, usart2 at %lu baud\n", SYSCLK_HZ, BAUD);

    uint32_t last = millis();
    int led_on = 0;

    /*
     * no delay_ms here; a blocking delay would leave typed characters unread for half
     * a second. instead every pass checks both, then sleeps until the next interrupt
     */
    while (1) {
        int c = uart_try_getc();
        if (c >= 0) {
            /* only echo printable ASCII as itself; anything else shows up as a dot */
            uart_printf("rx '%c' 0x%02x\n", (c >= 0x20 && c < 0x7F) ? c : '.', c);
        }

        if (millis() - last >= BLINK_MS) {
            /* step by the period instead of resetting to now, so time spent printing never makes the blink drift */
            last += BLINK_MS;
            led_on = !led_on;
            led_set(led_on);
            uart_printf("[%7lu ms] led %s\n", last, led_on ? "on" : "off");
        }

        /*
         * SysTick wakes the core every 1 ms, so RX still gets polled a thousand times a second.
         * a received byte waits in DR until it's read, but one that finishes arriving before
         * then is lost (ORE), so this keeps up with typing; a pasted burst needs an RX interrupt
         */
        __asm volatile ("wfi");
    }
}
