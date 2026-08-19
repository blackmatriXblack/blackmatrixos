/*
 * BlackMatrixOS Linker Script (PE version)
 * Produces a PE binary loaded at 0x10000
 */

OUTPUT_FORMAT("pei-i386")
ENTRY(_start)

SECTIONS
{
    . = 0x10000;

    .text : {
        *(.text)
        *(.text.*)
    }

    .rdata : {
        *(.rdata)
        *(.rdata.*)
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
        *(.reloc)
        *(.pdata)
        *(.xdata)
    }
}
