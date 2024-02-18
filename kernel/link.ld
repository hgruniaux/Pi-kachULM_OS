SECTIONS
{
    . = 0x80000;     /* Kernel load address for AArch64 */
    .text : { KEEP(*(.text.boot)) *(.text .text.* .gnu.linkonce.t*) }
    .rodata : { *(.rodata .rodata.* .gnu.linkonce.r*) }
    PROVIDE(_data = .);
    .data : { *(.data .data.* .gnu.linkonce.d*) }
    .bss (NOLOAD) : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }
    _end = .;

    /* Include the list of initialization functions sorted. */
    .init_array :
    {
        crti.o(.init_array)
        /* If we use constructor/desctructor priorities, the compiler will append
           these priorities to the section name. So, we sort them as crti.cxx _init
           and _fini calls them in the order found in the binary.  */
        KEEP (*(SORT(EXCLUDE_FILE(crti.o crtn.o) .init_array.*)))
        KEEP (*(EXCLUDE_FILE(crti.o crtn.o) .init_array))
        crtn.o(.init_array)
    }

    /* Include the list of termination functions sorted. */
    .fini_array :
    {
        crti.o(.fini_array)
        KEEP (*(SORT(EXCLUDE_FILE(crti.o crtn.o) .fini_array.*)))
        KEEP (*(EXCLUDE_FILE(crti.o crtn.o) .fini_array))
        crtn.o(.fini_array)
    }

   /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}
__bss_size = (__bss_end - __bss_start)>>3;
