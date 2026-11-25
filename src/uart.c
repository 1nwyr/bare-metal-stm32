#include <stdint.h>
#include "stm32f407.h"
#include "clock.h"
#include "uart.h"

#define TX_PIN 2   /* PA2 */
#define RX_PIN 3   /* PA3 */

void uart_init(uint32_t baud)
{
    /* the pins are on GPIOA (AHB1), the USART itself is on APB1; both clocks are off after reset */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    (void)RCC->APB1ENR;

    /* hand PA2 and PA3 to the alternate function mux; AF7 on these two is USART2 */
    GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MASK(TX_PIN) | GPIO_MODER_MASK(RX_PIN)))
                 | GPIO_MODER_VAL(TX_PIN, GPIO_MODE_AF)
                 | GPIO_MODER_VAL(RX_PIN, GPIO_MODE_AF);
    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~(GPIO_AFR_MASK(TX_PIN) | GPIO_AFR_MASK(RX_PIN)))
                  | GPIO_AFR_VAL(TX_PIN, GPIO_AF7_USART2)
                  | GPIO_AFR_VAL(RX_PIN, GPIO_AF7_USART2);

    /* pull RX up so a pin with nothing plugged in idles high like a real line instead of floating into framing errors */
    GPIOA->PUPDR = (GPIOA->PUPDR & ~GPIO_PUPDR_MASK(RX_PIN))
                 | GPIO_PUPDR_VAL(RX_PIN, GPIO_PUPD_UP);

    /*
     * with 16x oversampling the divider is USARTDIV = fck / (16 * baud), and BRR stores it
     * as 12.4 fixed point: mantissa in the top 12 bits, 16ths in the bottom 4. that makes
     * BRR = 16 * USARTDIV = fck / baud, so one rounded division does the whole thing.
     * 16 MHz / 115200 = 138.9, rounds to 139 = 0x8B, which is 8 + 11/16; 115108 baud, 0.08% slow
     */
    USART2->BRR = (PCLK1_HZ + baud / 2) / baud;

    /* 8 data bits, no parity, 1 stop bit are all reset defaults; just turn on TX, RX and the USART */
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void uart_putc(char c)
{
    /* TXE only means DR is free; the previous byte may still be shifting out, which is fine */
    while (!(USART2->SR & USART_SR_TXE)) {
    }
    USART2->DR = (uint8_t)c;
}

void uart_puts(const char *s)
{
    while (*s) {
        /* terminals expect CR LF; C strings only have the LF */
        if (*s == '\n') {
            uart_putc('\r');
        }
        uart_putc(*s++);
    }
}

int uart_getc(void)
{
    while (!(USART2->SR & USART_SR_RXNE)) {
    }
    /* reading SR and then DR clears RXNE, and clears ORE too if a byte got lost in between */
    return (uint8_t)USART2->DR;
}

int uart_try_getc(void)
{
    if (!(USART2->SR & USART_SR_RXNE)) {
        return -1;
    }
    return (uint8_t)USART2->DR;
}
