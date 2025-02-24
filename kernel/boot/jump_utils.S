// Jumps from EL3 level to EL2 (Non-secure) level.
// It is expected that the CPU is in EL3.
// Signature: void jump_from_el3_to_el2();
.global jump_from_el3_to_el2
jump_from_el3_to_el2:
    // Initialize the SCTLR_EL2 register before entering EL2.
    msr SCTLR_EL2, xzr
    msr HCR_EL2, xzr

    // Enable FPU
    mrs x0, CPTR_EL2
    orr x0, x0, #(3<<20)
    msr CPTR_EL2, x0

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

    // Enable FPU
    mrs x0, CPACR_EL1
    orr x0, x0, #(3<<20)
    msr CPACR_EL1, x0

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
// Signature: void jump_to_el0(uintptr_t elr, uintptr_t stack);
.global jump_to_el0
jump_to_el0:
    msr SPSR_EL1, xzr

    // Set EL0 stack pointer.
    msr SP_EL0, x1

    // Set EL0 entry point.
    msr ELR_EL1, x0

    eret
