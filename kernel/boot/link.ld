#include "mmu_defs.hpp"

ENTRY(_start);

SECTIONS
{
    . = KERNEL_BASE;

    /* Kernel load address for AArch64 */
    . += KERNEL_AARCH64_LOAD_ADDRESS;

    _stext = .;
    .text : { *(.text) }
    . = ALIGN(PAGE_SIZE);

    _srodata = .;
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
    . = ALIGN(PAGE_SIZE);

    _srwdata = .;
    PROVIDE(_data = .);
    .data : { *(.data) }
    .bss (NOLOAD) : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss)
        *(COMMON)
        __bss_end = .;
    }

    . = ALIGN(PAGE_SIZE);
    _kend = .;
}
