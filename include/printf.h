#ifndef PRINTF_H
#define PRINTF_H

#include <stdarg.h>

/*
 * a small printf over USART2, no libc involved
 *
 * conversions: %d %i %u %x %X %c %s %p %%
 * flags:       - (left align) and 0 (pad with zeros), then a field width
 * length:      l; uint32_t is unsigned long on arm-none-eabi, so it prints with %lu
 *
 * returns the number of characters printed, not counting the \r added before each \n
 */
int uart_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
int uart_vprintf(const char *fmt, va_list ap);

#endif
