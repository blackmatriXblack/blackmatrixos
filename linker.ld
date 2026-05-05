/*
 * BlackMatrixOS Linker Script
 * Produces a flat binary loaded at 0x10000
 */

OUTPUT_FORMAT("binary")
ENTRY(_start)

SECTIONS
{
    . = 0x10000;

    .text : {
        *(.text)
        *(.text.*)
    }

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }

    .data : {
        *(.data)
        *(.data.*)
    }

    .bss : {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }

    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.eh_frame*)
        *(.gnu.hash*)
        *(.dynsym*)
        *(.dynstr*)
        *(.got*)
        *(.plt*)
        *(.rel*)
        *(.rela*)
    }
}
