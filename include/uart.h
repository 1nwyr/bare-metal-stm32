#ifndef UART_H
#define UART_H

#include <stdint.h>

/* USART2 on PA2 (TX) and PA3 (RX), 8N1, everything polled; no interrupts, no buffers */
void uart_init(uint32_t baud);

/* one raw byte; blocks until there's room in the data register */
void uart_putc(char c);

/* a whole string, turning \n into \r\n on the way out */
void uart_puts(const char *s);

/* blocks until a byte arrives */
int uart_getc(void);

/* the next byte if one is waiting, otherwise -1 straight away */
int uart_try_getc(void);

#endif
