# Notes

Everything here is from RM0090 (the STM32F405/407 reference manual) and the STM32F407 datasheet; page numbers drift between revisions so I'm not writing them down.

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

## Registers I'll need first

* `RCC_AHB1ENR` at RCC + 0x30; bit 0 turns on GPIOA, bit 3 turns on GPIOD
* `RCC_APB1ENR` at RCC + 0x40; bit 17 turns on USART2
* `GPIOx_MODER` at + 0x00; two bits per pin, 01 is output, 10 is alternate function
* `GPIOx_ODR` at + 0x14; output data
* `GPIOx_BSRR` at + 0x18; set and reset bits in one write, so no read, modify, write race
* `GPIOx_AFRL` at + 0x20; alternate function select for pins 0 to 7

## Board stuff

* The four LEDs on the Discovery kit are PD12 to PD15 (green, orange, red, blue)
* USART2 is PA2 (TX) and PA3 (RX), alternate function 7
* After reset the core runs on HSI, 16 MHz; I'm leaving the PLL alone for now so the baud math stays simple

## Things I didn't know before

* Peripheral clocks are off after reset, so writing to GPIOD before enabling it in RCC just does nothing; no fault, no warning
* BSRR exists so you don't need to disable interrupts around a pin toggle

## What happens between reset and main

1. The core reads word 0 of the vector table (at 0x0800 0000, aliased to 0) and puts it in SP; that's `_estack`, the top of RAM
2. It reads word 1 and jumps there; that's `Reset_Handler`
3. `Reset_Handler` copies `.data` from flash (`_sidata`) into RAM (`_sdata` to `_edata`), because initialized globals have to live in RAM but RAM is garbage at power up
4. It zeroes `.bss` (`_sbss` to `_ebss`), because C promises uninitialized globals start at 0
5. It calls `main()`; if that ever returns it just spins

The linker script is where those symbols come from. `.data` has two addresses: where it runs (VMA, in RAM) and where it's stored (LMA, in flash); `> RAM AT > FLASH` is how you say that, and `LOADADDR(.data)` gets the flash one.

Vector table entries need bit 0 set because Cortex M only runs Thumb code; gcc does that for me when I take the address of a Thumb function, so I don't have to add 1 by hand.
