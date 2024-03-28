.section ".text.boot"  // Make sure the linker puts this at the start of the kernel image

.global _start  // Execution starts here

_start:
    // Check processor ID is zero (executing on main core), else hang
    mrs     x10, mpidr_el1
    and     x10, x10, #3
    cbz     x10, 2f
    // We're not on the main core, so hang in an infinite wait loop
1:  wfe
    b       1b
2:  // We're on the main core!

    // Set stack to start below our code
    ldr     x10, =_start
    mov     sp, x10

    // Enter the C and C++ world!
4:  bl      _startup
    // In case it does return, halt the master core too
    b       1b
