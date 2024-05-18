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
};  // struct GPRegisters

/** A single floating-point/SIMD register in Aarch64.
 * It should takes 128 bits on aarch64. */
struct fpu_reg_t {
  uint64_t low;
  uint64_t high;
};  // struct fpu_reg_t

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
#if 0
#define SAVE_REG(number)          \
  auto* ptr##number = &v##number; \
  asm volatile("str v" #number ", %0" : "=m"(ptr##number))

    SAVE_REG(0);
    SAVE_REG(1);
    SAVE_REG(2);
    SAVE_REG(3);
    SAVE_REG(4);
    SAVE_REG(5);
    SAVE_REG(6);
    SAVE_REG(7);
    SAVE_REG(8);
    SAVE_REG(9);
    SAVE_REG(10);
    SAVE_REG(11);
    SAVE_REG(12);
    SAVE_REG(13);
    SAVE_REG(14);
    SAVE_REG(15);
    SAVE_REG(16);
    SAVE_REG(17);
    SAVE_REG(18);
    SAVE_REG(19);
    SAVE_REG(20);
    SAVE_REG(21);
    SAVE_REG(22);
    SAVE_REG(23);
    SAVE_REG(24);
    SAVE_REG(25);
    SAVE_REG(26);
    SAVE_REG(27);
    SAVE_REG(28);
    SAVE_REG(29);
    SAVE_REG(30);
    SAVE_REG(31);

#undef SAVE_REG
#endif
  }

  /** Restores the saved floating-point/SIMD registers of this structure. */
  void restore() {
#if 0
#define RESTORE_REG(number)       \
  auto* ptr##number = &v##number; \
  asm volatile("ldr v" #number ", %0" : "=m"(ptr##number))

    RESTORE_REG(0);
    RESTORE_REG(1);
    RESTORE_REG(2);
    RESTORE_REG(3);
    RESTORE_REG(4);
    RESTORE_REG(5);
    RESTORE_REG(6);
    RESTORE_REG(7);
    RESTORE_REG(8);
    RESTORE_REG(9);
    RESTORE_REG(10);
    RESTORE_REG(11);
    RESTORE_REG(12);
    RESTORE_REG(13);
    RESTORE_REG(14);
    RESTORE_REG(15);
    RESTORE_REG(16);
    RESTORE_REG(17);
    RESTORE_REG(18);
    RESTORE_REG(19);
    RESTORE_REG(20);
    RESTORE_REG(21);
    RESTORE_REG(22);
    RESTORE_REG(23);
    RESTORE_REG(24);
    RESTORE_REG(25);
    RESTORE_REG(26);
    RESTORE_REG(27);
    RESTORE_REG(28);
    RESTORE_REG(29);
    RESTORE_REG(30);
    RESTORE_REG(31);

#undef RESTORE_REG
#endif
  }
};  // struct FPURegisters

/**
 * Registers saved and restored during an interrupt.
 */
struct InterruptRegisters {
  GPRegisters gp_regs;

  uint64_t elr;   // Exception Link Register
  uint64_t spsr;  // Saved Program Status Register
  uint64_t esr;
  uint64_t far;
};  // struct InterruptRegisters
