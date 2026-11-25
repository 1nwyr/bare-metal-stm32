#include <stdarg.h>
#include <stdint.h>
#include "printf.h"
#include "uart.h"

/* what came between the % and the conversion letter */
struct spec {
    int left;    /* '-' flag */
    int zero;    /* '0' flag */
    int width;
    int is_long; /* 'l' */
};

/* every character goes through here, so the count and the \n to \r\n fixup only happen in one place */
static void out(char c, int *count)
{
    if (c == '\n') {
        uart_putc('\r');
    }
    uart_putc(c);
    (*count)++;
}

static void out_pad(char c, int n, int *count)
{
    while (n-- > 0) {
        out(c, count);
    }
}

static void out_str(const char *s, const struct spec *sp, int *count)
{
    int len = 0;
    while (s[len]) {
        len++;
    }

    if (!sp->left) {
        out_pad(' ', sp->width - len, count);
    }
    while (*s) {
        out(*s++, count);
    }
    if (sp->left) {
        out_pad(' ', sp->width - len, count);
    }
}

static void out_num(uint32_t value, unsigned base, int upper, int negative,
                    const struct spec *sp, int *count)
{
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";

    /* digits come out lowest first, so collect them backwards; 32 bits is at most 10 decimal digits */
    char buf[10];
    int len = 0;
    do {
        buf[len++] = digits[value % base];
        value /= base;
    } while (value);

    int pad = sp->width - len - negative;

    if (sp->left) {
        /* -5 in %-4d is "-5  " */
        if (negative) {
            out('-', count);
        }
        while (len) {
            out(buf[--len], count);
        }
        out_pad(' ', pad, count);
    } else if (sp->zero) {
        /* zeros go between the sign and the digits: -5 in %04d is "-005" */
        if (negative) {
            out('-', count);
        }
        out_pad('0', pad, count);
        while (len) {
            out(buf[--len], count);
        }
    } else {
        /* spaces go in front of the sign: -5 in %4d is "  -5" */
        out_pad(' ', pad, count);
        if (negative) {
            out('-', count);
        }
        while (len) {
            out(buf[--len], count);
        }
    }
}

int uart_vprintf(const char *fmt, va_list ap)
{
    int count = 0;

    while (*fmt) {
        if (*fmt != '%') {
            out(*fmt++, &count);
            continue;
        }
        fmt++;

        struct spec sp = { 0, 0, 0, 0 };

        for (;; fmt++) {
            if (*fmt == '-') {
                sp.left = 1;
            } else if (*fmt == '0') {
                sp.zero = 1;
            } else {
                break;
            }
        }

        while (*fmt >= '0' && *fmt <= '9') {
            sp.width = sp.width * 10 + (*fmt++ - '0');
        }

        if (*fmt == 'l') {
            sp.is_long = 1;
            fmt++;
        }

        switch (*fmt) {
        case 'd':
        case 'i': {
            long v = sp.is_long ? va_arg(ap, long) : va_arg(ap, int);
            /* flip the sign as unsigned; -v on INT32_MIN would overflow, this wraps to the right magnitude */
            uint32_t mag = v < 0 ? 0u - (uint32_t)v : (uint32_t)v;
            out_num(mag, 10, 0, v < 0, &sp, &count);
            break;
        }
        case 'u':
        case 'x':
        case 'X': {
            uint32_t v = sp.is_long ? va_arg(ap, unsigned long) : va_arg(ap, unsigned int);
            out_num(v, *fmt == 'u' ? 10 : 16, *fmt == 'X', 0, &sp, &count);
            break;
        }
        case 'p': {
            /* always 0x and all 8 digits, so addresses line up */
            uint32_t v = (uint32_t)(uintptr_t)va_arg(ap, void *);
            struct spec p = { 0, 1, 8, 0 };
            out('0', &count);
            out('x', &count);
            out_num(v, 16, 0, 0, &p, &count);
            break;
        }
        case 'c': {
            /* char gets promoted to int when it goes through ... */
            char c[2] = { (char)va_arg(ap, int), '\0' };
            out_str(c, &sp, &count);
            break;
        }
        case 's': {
            const char *s = va_arg(ap, const char *);
            out_str(s ? s : "(null)", &sp, &count);
            break;
        }
        case '%':
            out('%', &count);
            break;
        case '\0':
            /* a lone % at the very end; stop here instead of walking past the terminator */
            return count;
        default:
            /* something I don't support; print it as is so the mistake shows up on the terminal */
            out('%', &count);
            out(*fmt, &count);
            break;
        }
        fmt++;
    }

    return count;
}

int uart_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int n = uart_vprintf(fmt, ap);
    va_end(ap);
    return n;
}
