#include "mmu_utils.hpp"
.text
.global _start // Execution starts here

_start:
    mov x27, x0
    // Check processor ID is zero (executing on main core), else hang
    mrs x0, mpidr_el1
    and x0, x0, #3
    cbz x0, 2f

1: // We're not on the main core, so hang in an infinite wait loop
    wfe
    b 1b

2: // We're on the main core!

    // Setup stack in low memory
    mov x0, #STACK_SIZE
    mov sp, x0

    // Jump to EL1
    bl jump_to_el1

    // Setup the MMU
    mov x0, x27
    bl mmu_init

    // Now, PC, SP & the DTB pointer are moved to high-memory space
    ldr x1, =(KERNEL_STACK_PAGE_TOP(DEFAULT_CORE))
    add sp, sp, x1

    mov x1, #KERNEL_BASE
    add x0, x27, x1

    ldr x2, =_high_memory_jump
    br x2

_high_memory_jump:
    // And low-memory space is deactivated here
    msr ttbr0_el1, xzr
    tlbi vmalle1
    dsb sy
    isb

    // We now jump into C++ World !
    b _startup
