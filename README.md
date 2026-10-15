# bare-metal-stm32

Firmware for an STM32F407, written straight from the reference manual; no HAL, no CubeMX, no vendor IDE. Just C, a linker script I wrote myself, and a Makefile.

I don't own the board, so everything runs in [Renode](https://renode.io), which emulates the STM32F4 Discovery kit well enough to run real firmware on it. The same binary would go on the actual chip.

## Why

Most STM32 tutorials start with the HAL, which is fine, but it hides everything. I wanted to know what actually happens between reset and `main()`, and what "turn on the clock for GPIOD" means when it's just a bit in a register.

## What's going in here

1. linker script and startup code; vector table, copying `.data`, zeroing `.bss`
2. blink an LED by writing GPIO registers directly
3. SysTick so the delay is real milliseconds and not a guess
4. a polling UART driver on USART2
5. a small printf on top of that UART

## Tools

* `arm-none-eabi-gcc` and `make` (I'm running them in Ubuntu on WSL2)
* Renode

## Build

```bash
make
```

## Status

Just started; the Makefile is in but there's no code yet, so it won't build until the startup code lands.

Register addresses and the memory map I'm working from are in [docs/notes.md](docs/notes.md).
