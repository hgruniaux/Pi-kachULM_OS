#pragma once

#include <cstdint>

/**
 * General purposes registers on Aarch64.
 * This structure only contains the registers that must be saved and restored during
 * a context switch (so no the callee-saved registers).
 */
struct GPRegisters {
  uint64_t x0, x1, x2, x3, x4, x5, x6;
  uint64_t x7, x8, x9, x10, x11, x12, x13;
  uint64_t x14, x15, x16, x17, x18, x19, x20;
  uint64_t x21, x22, x23, x24, x25, x26, x27;
  uint64_t x28, x29, x30, xzr;

  // No save() or restore() functions here because the registers are saved and restored
  // directly in the interrupts handler assembly code.
};  // struct GPRegisters

/** A single floating-point/SIMD register in Aarch64.
 * It should takes 128 bits on aarch64. */
typedef struct fpu_reg_t {
  uint8_t data[16];
} fpu_reg_t;  // struct fpu_reg_t

static_assert(sizeof(fpu_reg_t) == 16);

/**
 * Floating-point and SIMD registers on Aarch64.
 */
struct FPURegisters {
  fpu_reg_t v0, v1, v2, v3, v4, v5, v6, v7;
  fpu_reg_t v8, v9, v10, v11, v12, v13, v14;
  fpu_reg_t v15, v16, v17, v18, v19, v20;
  fpu_reg_t v21, v22, v23, v24, v25, v26;
  fpu_reg_t v27, v28, v29, v30, v31;

  /** Saves the current floating-point/SIMD registers into this structure. */
  void save() {
    #ifdef __ARM_NEON__
    // Save each of the 32 vector registers (v0-v31) into the structure using the "st1" instruction (store one 128-bit register)
    asm volatile (
      "st1 {v0.2d, v1.2d, v2.2d, v3.2d}, [%0]\n"
      "st1 {v4.2d, v5.2d, v6.2d, v7.2d}, [%0], #64\n"
      "st1 {v8.2d, v9.2d, v10.2d, v11.2d}, [%1]\n"
      "st1 {v12.2d, v13.2d, v14.2d, v15.2d}, [%1], #64\n"
      "st1 {v16.2d, v17.2d, v18.2d, v19.2d}, [%2]\n"
      "st1 {v20.2d, v21.2d, v22.2d, v23.2d}, [%2], #64\n"
      "st1 {v24.2d, v25.2d, v26.2d, v27.2d}, [%3]\n"
      "st1 {v28.2d, v29.2d, v30.2d, v31.2d}, [%3], #64\n"
      : // No output operands
      : "r" (&this->v0), "r" (&this->v8), "r" (&this->v16), "r" (&this->v24)
      : "memory"
    );
    #endif // __ARM_NEON__
  }

  /** Restores the saved floating-point/SIMD registers of this structure. */
  void restore() {
    #ifdef __ARM_NEON__
    // Restore each of the 32 vector registers (v0-v31) from the structure using the "ld1" instruction (load one 128-bit register)
    asm volatile (
      "ld1 {v0.2d, v1.2d, v2.2d, v3.2d}, [%0]\n"
      "ld1 {v4.2d, v5.2d, v6.2d, v7.2d}, [%0], #64\n"
      "ld1 {v8.2d, v9.2d, v10.2d, v11.2d}, [%1]\n"
      "ld1 {v12.2d, v13.2d, v14.2d, v15.2d}, [%1], #64\n"
      "ld1 {v16.2d, v17.2d, v18.2d, v19.2d}, [%2]\n"
      "ld1 {v20.2d, v21.2d, v22.2d, v23.2d}, [%2], #64\n"
      "ld1 {v24.2d, v25.2d, v26.2d, v27.2d}, [%3]\n"
      "ld1 {v28.2d, v29.2d, v30.2d, v31.2d}, [%3], #64\n"
      : // No output operands
      : "r" (&this->v0), "r" (&this->v8), "r" (&this->v16), "r" (&this->v24)
      : "memory"
    );
    #endif // __ARM_NEON__
  }
};  // struct FPURegisters

/**
 * Registers saved and restored during an interrupt. This represent the structure
 * stored in the stack by the assembly interrupt handler.
 */
struct InterruptRegisters {
  GPRegisters gp_regs;
  // FPU registers are not saved/restored by the assembly interrupt handler.
  // They are handled by the TaskSavedState structure.

  uint64_t elr;   // Exception Link Register
  uint64_t spsr;  // Saved Program Status Register
  uint64_t esr;   // Exception Syndrome Register
  uint64_t far;   // Fault Address Register
};  // struct InterruptRegisters
