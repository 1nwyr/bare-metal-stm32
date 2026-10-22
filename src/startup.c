#include <stdint.h>

/* all of these come from linker/stm32f407.ld; only their addresses mean anything */
extern uint32_t _estack;
extern uint32_t _sidata, _sdata, _edata;
extern uint32_t _sbss, _ebss;

int main(void);

void Reset_Handler(void);
void Default_Handler(void);

/* weak, so any driver can define the real one later and the linker picks that instead */
void NMI_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)     __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)    __attribute__((weak, alias("Default_Handler")));

typedef void (*vector_t)(void);

/* 16 core entries + 82 STM32F407 interrupts (IRQ 0 to 81) */
#define NUM_VECTORS (16 + 82)

__attribute__((section(".isr_vector"), used))
const vector_t vector_table[NUM_VECTORS] = {
    [0]  = (vector_t)&_estack,  /* not a function; the core loads this into SP */
    [1]  = Reset_Handler,
    [2]  = NMI_Handler,
    [3]  = HardFault_Handler,
    [4]  = MemManage_Handler,
    [5]  = BusFault_Handler,
    [6]  = UsageFault_Handler,
    /* 7 to 10 reserved, left as 0 */
    [11] = SVC_Handler,
    [12] = DebugMon_Handler,
    /* 13 reserved */
    [14] = PendSV_Handler,
    [15] = SysTick_Handler,
    /* gcc range syntax; every peripheral IRQ goes to the default for now */
    [16 ... NUM_VECTORS - 1] = Default_Handler,
};

void Reset_Handler(void)
{
    /* .data: copy initial values from flash into RAM */
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    /* .bss: C says globals without an initializer start at 0, so make that true */
    dst = &_sbss;
    while (dst < &_ebss) {
        *dst++ = 0;
    }

    main();

    /* main shouldn't return on a microcontroller; if it does, park here */
    while (1) {
    }
}

/* any interrupt nobody handles lands here; hang so a debugger can see where we are */
void Default_Handler(void)
{
    while (1) {
    }
}
