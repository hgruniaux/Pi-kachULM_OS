SECTIONS
{
    /* Kernel load address for AArch64 */
    . = 0xffff000000000000 + 0x80000;

    _stext = .;
    .text.boot : { KEEP(*(.text.boot)) }
    .text : { *(.text) }
    _etext = .;

    .rodata : { *(.rodata) }

    /* For clang to compile the project, .init_array and .fini_array must be contiguous with rodata */
    /* Include the list of initialization functions sorted. */
    .init_array ALIGN(8):
    {
        __init_array_start = .;
        /* If we use constructor/destructor priorities, the compiler will append
           these priorities to the section name. We need to sort them.  */
        KEEP (*(SORT(EXCLUDE_FILE(crti.o crtn.o) .init_array.*)))
        KEEP (*(EXCLUDE_FILE(crti.o crtn.o) .init_array))
        __init_array_end = .;
    }

    /* Include the list of termination functions sorted. */
    .fini_array ALIGN(8):
    {
        __fini_array_start = .;
        KEEP (*(SORT(EXCLUDE_FILE(crti.o crtn.o) .fini_array.*)))
        KEEP (*(EXCLUDE_FILE(crti.o crtn.o) .fini_array))
        __fini_array_end = .;
    }

    PROVIDE(_data = .);
    .data : { *(.data) }
    .bss (NOLOAD) : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss)
        *(COMMON)
        __bss_end = .;
    }

    . = ALIGN(0x1000);
    _kend = .;
}
