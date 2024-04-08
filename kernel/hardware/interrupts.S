// Enables the FPU and Neon unit in EL0 and EL1.
// To be called before the CPU enter EL1.
// Signature: void enable_fpu_and_neon()
.global enable_fpu_and_neon
enable_fpu_and_neon:
    mov x1, #(0x3 << 20)
    msr CPACR_EL1, x1
    isb
    ret

// Jumps from EL3 level to EL2 (Non-secure) level.
// It is expected that the CPU is in EL3.
// Signature: void jump_from_el3_to_el2();
.global jump_from_el3_to_el2
jump_from_el3_to_el2:
    // Initialize the SCTLR_EL2 register before entering EL2.
    msr SCTLR_EL2, xzr
    msr HCR_EL2, xzr

    // Set EL2 stack pointer.
    mov x0, sp
    msr SP_EL2, x0

    mrs x0, SCR_EL3
    orr x0, x0, #(1<<10) // RW EL2 Execution state is AArch64.
    orr x0, x0, #(1<<0) // NS EL1 is Non-secure world.
    msr SCR_EL3, x0
    mov x0, #0b01001 // DAIF=0000
    msr SPSR_EL3, x0 // M[4:0]=01001 EL2h must match SCR_EL3.RW

    // Set EL2 entry point.
    adr x0, el2_entry
    msr ELR_EL3, x0

    eret

el2_entry:  // EL2 entry point
    ret

// Jumps from EL2 level to EL1 (Non-secure) level.
// It is expected that the CPU is in EL2.
// Signature: void jump_from_el2_to_el1();
.global jump_from_el2_to_el1
jump_from_el2_to_el1:
    // Initialize the SCTLR_EL1 register before entering EL1.
    msr SCTLR_EL1, xzr

    // Set EL1 stack pointer.
    mov x0, sp
    msr SP_EL1, x0

    // Set EL1 entry point.
    adr x0, el1_entry
    msr ELR_EL2, x0

    mrs x0, HCR_EL2
    orr x0, x0, #(1<<31) // RW=1 EL1 Execution state is AArch64.
    msr HCR_EL2, x0

    mov x0, #0b00101 // DAIF=0000
    msr SPSR_EL2, x0 // M[4:0]=00101 EL1h must match HCR_EL2.RW.

    eret

el1_entry: // EL1 entry point
    ret

// Jumps from EL1 level to EL0 (Non-secure) level.
// It is expected that the CPU is in EL1.
// Signature: void jump_from_el1_to_el0();
.global jump_from_el1_to_el0
jump_from_el1_to_el0:
    msr SPSR_EL1, xzr

    // Set EL0 stack pointer.
    mov x0, sp
    msr SP_EL0, x0

    // Set EL0 entry point.
    adr x0, el0_entry
    msr ELR_EL1, x0

    eret

el0_entry: // EL0 entry point
    ret

.global interrupts_vector_table
.balign 2048
interrupts_vector_table:
// The four first entries are for when we are using SP_EL0. This should never happen
// as we requested that the CPU use SP_ELx for each exception level. So we just enter an infinite loop.
    b exception_entry
.balign 0x80
    b .
.balign 0x80
    b .
.balign 0x80
    b exception_entry

// The following entries are for interrupts and exceptions coming from the
// same exception level (that is from the kernel code). These entries will use
// the EL1 stack pointer (the stack pointer of the kernel, and therefore the
// same as the interrupt code).
.balign 0x80
    // The exception handler for a synchronous exception from the current EL
    // using the current SP.
    b exception_entry
.balign 0x80
    // The exception handler for an IRQ exception from the current EL
    // using the current SP.
    b .
.balign 0x80
    // The exception handler for an FIQ from the current EL
    // using the current SP.
    b .
.balign 0x80
    // The exception handler for a System Error exception from the current EL
    // using the current SP.
    b exception_entry

// The following entries are for interrupts and exceptions coming from lower
// exception levels (like EL0, the user space).
// From AArch64 mode:
.balign 0x80
    // The exception handler for a synchronous exception from a lower EL (AArch64).
    b exception_entry
.balign 0x80
    // The exception handler for an IRQ from a lower EL (AArch64).
    b .
.balign 0x80
    // The exception handler for an FIQ from a lower EL (AArch64).
    b .
.balign 0x80
    // The exception handler for a System Error exception from a lower EL(AArch64).
    b exception_entry
// From AArch32 mode:
.balign 0x80
    // The exception handler for a synchronous exception from a lower EL(AArch32).
    b exception_entry
.balign 0x80
    // The exception handler for an IRQ exception from a lower EL (AArch32).
    b .
.balign 0x80
    // The exception handler for an FIQ exception from a lower EL (AArch32).
    b .
.balign 0x80
    // The exception handler for a System Error exception from a lower EL(AArch32).
    b exception_entry

.global exception_handler

exception_entry:
    sub sp, sp, #176 // sizeof(Registers)
    stp x0, x1, [sp, #16 * 0]
    stp x2, x3, [sp, #16 * 1]
    stp x4, x5, [sp, #16 * 2]
    stp x6, x7, [sp, #16 * 3]
    stp x8, x9, [sp, #16 * 4]
    stp x10, x11, [sp, #16 * 5]
    stp x12, x13, [sp, #16 * 6]
    stp x14, x15, [sp, #16 * 7]
    stp x16, x17, [sp, #16 * 8]
    stp x18, x30, [sp, #16 * 9]

    mrs x0, ESR_EL1
    mrs x1, FAR_EL1
    stp x0, x1, [sp, #16 * 10]

    mov x0, sp
    bl exception_handler

    ldp x0, x1, [sp, #16 * 0]
    ldp x2, x3, [sp, #16 * 1]
    ldp x4, x5, [sp, #16 * 2]
    ldp x6, x7, [sp, #16 * 3]
    ldp x8, x9, [sp, #16 * 4]
    ldp x10, x11, [sp, #16 * 5]
    ldp x12, x13, [sp, #16 * 6]
    ldp x14, x15, [sp, #16 * 7]
    ldp x16, x17, [sp, #16 * 8]
    ldp x18, x30, [sp, #16 * 9]
    add sp, sp, #176 // sizeof(Registers)

    eret

// Registers the interrupts vector table to the CPU.
// To be called early in kernel initialization and when CPU is in EL1.
// Signature: void init_interrupts_vector_table();
.global init_interrupts_vector_table
init_interrupts_vector_table:
    adr x0, interrupts_vector_table
    msr VBAR_EL1, x0
    ret
