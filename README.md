# bare-metal-stm32

Firmware for an STM32F407, written straight from the reference manual; no HAL, no CubeMX, no vendor IDE. Just C, a linker script I wrote myself, and a Makefile.

I don't own the board, so everything runs in [Renode](https://renode.io), which emulates the STM32F4 Discovery kit well enough to run real firmware on it. The same binary would go on the actual chip.

## Why

Most STM32 tutorials start with the HAL, which is fine, but it hides everything. I wanted to know what actually happens between reset and `main()`, and what "turn on the clock for GPIOD" means when it's just a bit in a register.

## What's in here

1. linker script and startup code; vector table, copying `.data`, zeroing `.bss`
2. blink an LED by writing GPIO registers directly
3. SysTick so the delay is real milliseconds and not a guess
4. a polling UART driver on USART2
5. a small printf on top of that UART

All five are done.

| File | What it is |
|---|---|
| `linker/stm32f407.ld` | memory map, section placement, the symbols startup needs |
| `src/startup.c` | vector table and `Reset_Handler` |
| `include/stm32f407.h` | RCC, GPIO, USART and SysTick registers as structs, with offset checks |
| `include/clock.h` | the clock frequencies everything else does math with (16 MHz HSI, no PLL) |
| `src/systick.c` | 1 ms tick, `millis()`, `delay_ms()` |
| `src/uart.c` | USART2 on PA2/PA3, 115200 8N1, polled TX and RX |
| `src/printf.c` | `uart_printf`; `%d %i %u %x %X %c %s %p %%`, width, `-` and `0` flags, `l` |
| `src/main.c` | blinks PD12 every 500 ms, prints a line each time, reports anything typed |

## Tools

* `arm-none-eabi-gcc` and `make` (I'm running them in Ubuntu on WSL2)
* Renode

## Build

```bash
make
```

## Status

Everything on the list works in Renode. On boot the firmware turns on SysTick at 1 ms, brings up USART2 at 115200 baud and prints a banner, then toggles the green LED every 500 ms and prints a line each time. Anything typed into the UART gets reported back with its hex value:

```
bare-metal-stm32: sysclk 16000000 Hz, usart2 at 115200 baud
[    500 ms] led on
[   1000 ms] led off
rx 'h' 0x68
[   1500 ms] led on
```

The main loop polls RX and the clock, then sleeps with `wfi` until the next SysTick, so RX gets checked once a millisecond. That keeps up with typing, but a pasted burst would overrun the one byte data register; that needs an RX interrupt and a buffer, which is past what this was for. The core is still on the 16 MHz HSI with no PLL, and the build is still soft float.

The whole image is about 2.5 KB of flash; the only RAM it uses besides the stack is the 4 byte tick counter, and nothing gets linked in from libc.

## Run

```bash
make
renode renode/run.resc
```

The script loads `build/firmware.elf` on Renode's Discovery kit, opens a terminal window on USART2 and turns on logging for the green LED, so you see lines like `gpioPortD.UserLED: LED state changed to True` in the log. Typing in the USART2 window sends bytes to the firmware.

Without a GUI, `renode --console --disable-gui renode/run.resc` prints the UART lines into the log instead of a window.

The script also fixes two things about Renode's STM32F4 model that would otherwise make the timing wrong: it clocks SysTick at 72 MHz instead of the 16 MHz the chip actually runs at after reset, and it assumes the core does 100 MIPS. Details in [docs/notes.md](docs/notes.md#renode-quirks).

Register addresses and the memory map I'm working from are in [docs/notes.md](docs/notes.md).
