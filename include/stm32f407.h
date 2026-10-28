#ifndef STM32F407_H
#define STM32F407_H

/* hand written from RM0090; only the peripherals I actually use so far */

#include <stdint.h>
#include <stddef.h>

/* volatile so every access really goes to the bus; the compiler can't cache or drop it */
#define __IO volatile

/* RCC, RM0090 section 7.3 */
typedef struct {
    __IO uint32_t CR;          /* 0x00 */
    __IO uint32_t PLLCFGR;     /* 0x04 */
    __IO uint32_t CFGR;        /* 0x08 */
    __IO uint32_t CIR;         /* 0x0C */
    __IO uint32_t AHB1RSTR;    /* 0x10 */
    __IO uint32_t AHB2RSTR;    /* 0x14 */
    __IO uint32_t AHB3RSTR;    /* 0x18 */
    uint32_t      RESERVED0;   /* 0x1C */
    __IO uint32_t APB1RSTR;    /* 0x20 */
    __IO uint32_t APB2RSTR;    /* 0x24 */
    uint32_t      RESERVED1[2];/* 0x28, 0x2C */
    __IO uint32_t AHB1ENR;     /* 0x30 */
    __IO uint32_t AHB2ENR;     /* 0x34 */
    __IO uint32_t AHB3ENR;     /* 0x38 */
    uint32_t      RESERVED2;   /* 0x3C */
    __IO uint32_t APB1ENR;     /* 0x40 */
    __IO uint32_t APB2ENR;     /* 0x44 */
    uint32_t      RESERVED3[2];/* 0x48, 0x4C */
    __IO uint32_t AHB1LPENR;   /* 0x50 */
    __IO uint32_t AHB2LPENR;   /* 0x54 */
    __IO uint32_t AHB3LPENR;   /* 0x58 */
    uint32_t      RESERVED4;   /* 0x5C */
    __IO uint32_t APB1LPENR;   /* 0x60 */
    __IO uint32_t APB2LPENR;   /* 0x64 */
    uint32_t      RESERVED5[2];/* 0x68, 0x6C */
    __IO uint32_t BDCR;        /* 0x70 */
    __IO uint32_t CSR;         /* 0x74 */
    uint32_t      RESERVED6[2];/* 0x78, 0x7C */
    __IO uint32_t SSCGR;       /* 0x80 */
    __IO uint32_t PLLI2SCFGR;  /* 0x84 */
} RCC_TypeDef;

/* GPIO, RM0090 section 8.4; every port has the same layout */
typedef struct {
    __IO uint32_t MODER;       /* 0x00 */
    __IO uint32_t OTYPER;      /* 0x04 */
    __IO uint32_t OSPEEDR;     /* 0x08 */
    __IO uint32_t PUPDR;       /* 0x0C */
    __IO uint32_t IDR;         /* 0x10 */
    __IO uint32_t ODR;         /* 0x14 */
    __IO uint32_t BSRR;        /* 0x18 */
    __IO uint32_t LCKR;        /* 0x1C */
    __IO uint32_t AFR[2];      /* 0x20 AFRL, 0x24 AFRH */
} GPIO_TypeDef;

/* if I miscount a reserved gap, the build fails here instead of the LED silently not blinking */
_Static_assert(offsetof(RCC_TypeDef, AHB1ENR) == 0x30, "RCC AHB1ENR offset");
_Static_assert(offsetof(RCC_TypeDef, APB1ENR) == 0x40, "RCC APB1ENR offset");
_Static_assert(offsetof(RCC_TypeDef, BDCR) == 0x70, "RCC BDCR offset");
_Static_assert(offsetof(RCC_TypeDef, PLLI2SCFGR) == 0x84, "RCC PLLI2SCFGR offset");
_Static_assert(offsetof(GPIO_TypeDef, BSRR) == 0x18, "GPIO BSRR offset");
_Static_assert(offsetof(GPIO_TypeDef, AFR) == 0x20, "GPIO AFR offset");

/* base addresses, see docs/notes.md */
#define PERIPH_BASE     0x40000000UL
#define AHB1PERIPH_BASE (PERIPH_BASE + 0x00020000UL)

#define GPIOA_BASE      (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOD_BASE      (AHB1PERIPH_BASE + 0x0C00UL)
#define RCC_BASE        (AHB1PERIPH_BASE + 0x3800UL)

#define RCC             ((RCC_TypeDef *)RCC_BASE)
#define GPIOA           ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOD           ((GPIO_TypeDef *)GPIOD_BASE)

/* RCC_AHB1ENR bits */
#define RCC_AHB1ENR_GPIOAEN  (1UL << 0)
#define RCC_AHB1ENR_GPIODEN  (1UL << 3)

/* RCC_APB1ENR bits */
#define RCC_APB1ENR_USART2EN (1UL << 17)

/* MODER is two bits per pin */
#define GPIO_MODE_INPUT  0x0UL
#define GPIO_MODE_OUTPUT 0x1UL
#define GPIO_MODE_AF     0x2UL
#define GPIO_MODE_ANALOG 0x3UL

#define GPIO_MODER_MASK(pin)       (0x3UL << ((pin) * 2))
#define GPIO_MODER_VAL(pin, mode)  ((mode) << ((pin) * 2))

/* BSRR: low half sets a pin, high half resets it */
#define GPIO_BSRR_SET(pin)   (1UL << (pin))
#define GPIO_BSRR_RESET(pin) (1UL << ((pin) + 16))

#endif
