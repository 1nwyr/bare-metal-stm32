# Notes

Everything here is from RM0090 (the STM32F405/407 reference manual) and the STM32F407 datasheet, plus PM0214 (the Cortex-M4 programming manual) for SysTick, since that's part of the ARM core and not ST's; page numbers drift between revisions so I'm not writing them down.

## Memory map (the parts I care about)

| What | Start | Size |
|---|---|---|
| Flash | 0x0800 0000 | 1 MB |
| SRAM1 + SRAM2 | 0x2000 0000 | 128 KB (112 + 16, back to back) |
| CCM RAM | 0x1000 0000 | 64 KB; CPU only, DMA can't see it, so I'm not using it yet |

The chip boots from flash, which gets aliased to 0x0000 0000. The first word is the initial stack pointer, the second is the address of the reset handler; that's the whole vector table idea.

## Peripherals

| Peripheral | Base address | Bus |
|---|---|---|
| RCC | 0x4002 3800 | AHB1 |
| GPIOA | 0x4002 0000 | AHB1 |
| GPIOD | 0x4002 0C00 | AHB1 |
| USART2 | 0x4000 4400 | APB1 |
| SysTick | 0xE000 E010 | Cortex-M4 private peripheral bus; same address on every Cortex-M |

## Registers I'll need first

* `RCC_AHB1ENR` at RCC + 0x30; bit 0 turns on GPIOA, bit 3 turns on GPIOD
* `RCC_APB1ENR` at RCC + 0x40; bit 17 turns on USART2
* `GPIOx_MODER` at + 0x00; two bits per pin, 01 is output, 10 is alternate function
* `GPIOx_ODR` at + 0x14; output data
* `GPIOx_BSRR` at + 0x18; set and reset bits in one write, so no read, modify, write race
* `GPIOx_AFRL` at + 0x20; alternate function select for pins 0 to 7
* `GPIOx_PUPDR` at + 0x0C; two bits per pin, 01 is pull up

## SysTick

* `STK_CTRL` at + 0x00; bit 0 enable, bit 1 raise the SysTick exception at 0, bit 2 clock source (1 is the core clock, 0 is core clock / 8 on the STM32F4)
* `STK_LOAD` at + 0x04; the value it reloads from, 24 bits wide
* `STK_VAL` at + 0x08; the current count, any write clears it
* It counts LOAD down to 0 and then reloads, so a period is LOAD + 1 cycles; 1 ms at 16 MHz is LOAD = 15999
* SysTick is exception 15, so it's entry 15 in the vector table; defining `SysTick_Handler` in `systick.c` beats the weak alias in `startup.c`, no table edit needed
* `ticks` is one 32 bit word, and a 32 bit load is a single instruction on the M4, so `millis()` can read it without masking interrupts
* `(uint32_t)(now - start) < ms` still works when the counter wraps (after about 49.7 days); comparing `now < start + ms` wouldn't

## USART2

* `USART_SR` at + 0x00; bit 7 TXE (data register empty), bit 6 TC (transmission complete), bit 5 RXNE (byte received), bit 3 ORE (overrun)
* `USART_DR` at + 0x04; write to send, read to receive
* `USART_BRR` at + 0x08; baud divider
* `USART_CR1` at + 0x0C; bit 13 UE (USART enable), bit 3 TE, bit 2 RE; word length, parity and stop bits default to 8N1
* PA2 and PA3 need MODER = 10 (alternate function) and AF7 in AFRL; that's bits 8 to 11 for PA2 and 12 to 15 for PA3
* TXE only means DR is free; the previous byte can still be shifting out. TC is the one to wait on before turning the USART off
* Reading SR and then DR clears RXNE, and clears ORE too; a polling read does exactly that, so overruns clear themselves

### Baud math

With 16x oversampling (OVER8 = 0) the divider is `USARTDIV = fck / (16 * baud)`, stored in BRR as 12.4 fixed point: mantissa in bits 4 to 15, sixteenths in bits 0 to 3. That makes `BRR = 16 * USARTDIV = fck / baud`, so the driver does one rounded division:

* fck is PCLK1, which is 16 MHz because the APB1 prescaler is /1 after reset
* 16 000 000 / 115 200 = 138.9, rounds to 139 = 0x8B
* mantissa 8, fraction 11, so USARTDIV = 8.6875 and the real rate is 16 000 000 / 139 = 115 108 baud, 0.08% slow; UARTs tolerate a couple of percent

## printf

* `<stdarg.h>`, `<stdint.h>` and `<stddef.h>` are compiler headers, not libc, so they're fine with `-ffreestanding`
* `uint32_t` is `unsigned long` on arm-none-eabi, not `unsigned int`, so it prints with `%lu`; the `format(printf, 1, 2)` attribute on `uart_printf` makes gcc check that like it would for the real printf
* Negating a negative `int` overflows for INT_MIN; casting to `uint32_t` first and doing `0u - x` gives the right magnitude
* Zero padding goes between the sign and the digits (`%05d` of -42 is `-0042`), space padding goes in front of the sign (`%5d` is `  -42`)
* `char` arguments get promoted to `int` when they go through `...`, so `%c` has to `va_arg(ap, int)`
* No division helpers get pulled in from libgcc because the M4 has a hardware divide instruction

## Board stuff

* The four LEDs on the Discovery kit are PD12 to PD15 (green, orange, red, blue)
* USART2 is PA2 (TX) and PA3 (RX), alternate function 7
* After reset the core runs on HSI, 16 MHz; I'm leaving the PLL alone for now so the baud math stays simple

## Things I didn't know before

* Peripheral clocks are off after reset, so writing to GPIOD before enabling it in RCC just does nothing; no fault, no warning
* BSRR exists so you don't need to disable interrupts around a pin toggle
* Register structs only work if every reserved gap is the right size; I put `_Static_assert(offsetof(...))` checks in the header so a miscount breaks the build instead of writing to the wrong register
* `volatile` on every register field matters; without it the compiler is allowed to merge or drop writes it thinks nobody reads
* Renode prints `PC = 0x80001F9` at start; the 1 at the end is the Thumb bit from the vector table, the real reset handler is at 0x080001F8 (it moves as code gets added; `arm-none-eabi-nm -n build/firmware.elf` shows where it is now)

## What happens between reset and main

1. The core reads word 0 of the vector table (at 0x0800 0000, aliased to 0) and puts it in SP; that's `_estack`, the top of RAM
2. It reads word 1 and jumps there; that's `Reset_Handler`
3. `Reset_Handler` copies `.data` from flash (`_sidata`) into RAM (`_sdata` to `_edata`), because initialized globals have to live in RAM but RAM is garbage at power up
4. It zeroes `.bss` (`_sbss` to `_ebss`), because C promises uninitialized globals start at 0
5. It calls `main()`; if that ever returns it just spins

The linker script is where those symbols come from. `.data` has two addresses: where it runs (VMA, in RAM) and where it's stored (LMA, in flash); `> RAM AT > FLASH` is how you say that, and `LOADADDR(.data)` gets the flash one.

Vector table entries need bit 0 set because Cortex M only runs Thumb code; gcc does that for me when I take the address of a Thumb function, so I don't have to add 1 by hand.

## Renode quirks

* Renode's `stm32f4.repl` builds the NVIC with `systickFrequency: 72000000`, but the real chip runs off the 16 MHz HSI after reset and this firmware never starts the PLL. Left alone, LOAD = 15999 is 0.22 ms in Renode instead of 1 ms, so every "millisecond" is 4.5x too short. `nvic Frequency 16000000` in `run.resc` fixes it; the firmware itself is right for the real chip
* Renode's virtual time comes from counting instructions, and the default is 100 MIPS. A 16 MHz M4 does about one instruction per cycle, so `cpu PerformanceInMips 16` is closer to the real thing
* Every peripheral register read in Renode goes through its C# model, which is slow next to translated code. A loop that hammers `USART2->SR` ran about 6x slower than real time at the default 100 MIPS (still 2.7x at 16). With `wfi` the core sleeps between SysTicks, and while it sleeps Renode lets virtual time run at wall clock speed, so the emulation keeps pace with real time
* Renode's UART model hands bytes over instantly and queues them, so it won't show an overrun the way the real USART would; RX timing limits only show up on hardware
