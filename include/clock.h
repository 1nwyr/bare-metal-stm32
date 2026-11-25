#ifndef CLOCK_H
#define CLOCK_H

/*
 * nothing touches the PLL or the bus prescalers, so everything still runs off
 * the 16 MHz internal RC oscillator it starts on after reset
 */
#define HSI_HZ    16000000UL
#define SYSCLK_HZ HSI_HZ
#define HCLK_HZ   SYSCLK_HZ   /* AHB prescaler is /1 after reset */
#define PCLK1_HZ  HCLK_HZ     /* so is APB1's; USART2 is clocked from here */

#endif
