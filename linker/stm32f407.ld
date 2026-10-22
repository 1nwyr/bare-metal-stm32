/* STM32F407VG: 1M flash, 128K of SRAM1 + SRAM2 (CCM left out on purpose) */

ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

/* stack grows down from the very top of RAM */
_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    /* has to be first in flash; the core reads SP and the reset vector from here */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    /* libgcc unwind tables; empty for my code but the linker complains without a home for them */
    .ARM.exidx :
    {
        *(.ARM.exidx*)
    } > FLASH

    /* lives in RAM at run time, but the initial values are stored in flash right after .text */
    _sidata = LOADADDR(.data);

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH

    /* nothing stored in flash for this one; startup just zeroes it */
    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    /* first free byte after bss; a heap would start here later */
    _end = .;
    end = .;
}
