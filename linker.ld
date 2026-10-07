ENTRY(kernel_main)

SECTIONS
{
    . = 1M;

    .text :
    {
        KEEP(*(.text.kernel_main))
        *(.text*)
    }

    .rodata :
    {
        *(.rodata*)
    }

    .data :
    {
        *(.data*)
    }

    .bss :
    {
        *(COMMON)
        *(.bss*)
    }
}

