#include "memory/mmu.h"

.section ".text.boot" // Make sure the linker puts this at the start of the kernel image

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
    adr x0, _start
    mov sp, x0

    // Temporary Exception Table (TODO : Remove this)
    adr x0, inter_tabl
    msr VBAR_EL1, x0

    // Jump to EL1
    bl jump_to_el1

    // Setup the MMU
    adr x0, _kend
    bl mmu_init

    // Now, PC, SP & the DTB pointer are moved to high-memory space
    mov x1, #KERNEL_BASE
    add sp, sp, x1
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

// Temporary Debug Function (TODO : Remove this)
// Registers used:
// x0, x1, x9, x10, x11, x12
.global debug
debug:
    adr x9, test
    str x0, [x9]
    mov x0, x9
    mov x12, #8

    ldr x9, =(0x3F000000+0x201000+0x0)
    cmp x1, #0
    b.eq 1f
    mov x1, #KERNEL_BASE
    add x9, x9, x1

1:
    ldrb w10, [x0]
    lsr w11, w10, #4
    add w11, w11, 65
    strb w11, [x9]
    and w11, w10, #0xf
    add w11, w11, 65
    strb w11, [x9]
    add x0, x0, #1
    sub x12, x12, #1
    cmp x12, #0
    b.ne 1b
    mov w10, #13
    strb w10, [x9]
    mov w10, #10
    strb w10, [x9]
    ret

test:
    .quad 0xffff0000ffff0000

// Temporary Debug Function (TODO : Remove this)
.macro KERNEL_ENTRY source, kind
.balign 0x80
    str x0, [sp, #-16]!
    mov x0, #0xffffffffffffffff
    mov x1, #1
    bl debug
    mrs x0, elr_el1
    mov x1, #1
    bl debug
    ldr x0, [sp], #16
    mov x1, #1
    bl debug

999:
    b 999b
.endm

.balign 2048
inter_tabl:
// The four first entries are for when we are using SP_EL0. This should never happen
// as we requested that the CPU use SP_ELx for each exception level.
    KERNEL_ENTRY 0, 0
    KERNEL_ENTRY 0, 1
    KERNEL_ENTRY 0, 2
    KERNEL_ENTRY 0, 3
// The following entries are for interrupts and exceptions coming from the
// same exception level (that is from the kernel code). These entries will use
// the EL1 stack pointer (the stack pointer of the kernel, and therefore the
// same as the interrupt code).
    KERNEL_ENTRY 1, 0
    KERNEL_ENTRY 1, 1
    KERNEL_ENTRY 1, 2
    KERNEL_ENTRY 1, 3
// The following entries are for interrupts and exceptions coming from lower
// exception levels (like EL0, the user space).
// From AArch64 mode:
    KERNEL_ENTRY 2, 0
    KERNEL_ENTRY 2, 1
    KERNEL_ENTRY 2, 2
    KERNEL_ENTRY 2, 3
// From AArch32 mode (NOT SUPPORTED):
    KERNEL_ENTRY 3, 0
    KERNEL_ENTRY 3, 1
    KERNEL_ENTRY 3, 2
    KERNEL_ENTRY 3, 3
