#pragma once

#include <cstdint>

namespace MMIO {
/** The MMIO area base address. Initialized by MMIO::init(); */
extern uint32_t BASE;
/** Initializes the memory-mapped IO system. */
void init();

/** Writes @a value into the memory-mapped IO @a reg register. */
[[gnu::always_inline]] static inline void write(uint32_t reg, uint32_t value) {
  // The GCC warning can be ignored because the address "BASE + reg"
  // can always be represented by a 32-bit pointer. And we need to keep pointers
  // to uint32_t because MMIO registers are 32-bits width.
  // We need to keep volatile to avoid the compiler
  // to optimize out the memory write.
  *(volatile uint32_t*)(uintptr_t)(BASE + reg) = value;
}

/** Reads from the memory-mapped IO @a reg register. */
[[gnu::always_inline]] static inline uint32_t read(uint32_t reg) {
  // The GCC warning can be ignored because the address "BASE + reg"
  // can always be represented by a 32-bit pointer. And we need to keep pointers
  // to uint32_t because MMIO registers are 32-bits width.
  // We need to keep volatile to avoid the compiler
  // to optimize out the memory read.
  return *(volatile uint32_t*)(uintptr_t)(BASE + reg);
}
};  // namespace MMIO
