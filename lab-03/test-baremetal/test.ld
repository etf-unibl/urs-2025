MEMORY
{
  program     (rx)  : ORIGIN = 0xffff0000, LENGTH = 16K
  data        (rw)  : ORIGIN = 0xffff4000, LENGTH = 16K
}

/* Reserve 8K for stack */
__stack_size = 0x2000;

/* Section Definitions */
SECTIONS
{
    .text :
    {
        *(.text*)
        *(.rodata*)
        __text_end__ = .;
    } > program

    /* .bss section which is used for uninitialized data */
    .bss (NOLOAD) :
    {
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        __bss_end__ = .;
    } > data

    /* .data section which is used for initialized data */
    .data :
    {
        __data_start__ = .;
        *(.data*);
        __data_end__ = .;
    } > data AT > program

    /* stack section */
    .stack (NOLOAD):
    {
        . = ALIGN(8);
        . = . + __stack_size;
        . = ALIGN(8);
        __stack_start__ = .;
    } > data

    _end = . ;
}
