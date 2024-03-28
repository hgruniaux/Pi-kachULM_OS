#pragma once
#include "dtb/dtb.hpp"

#include <cstdint>

namespace MMIO {
enum class DeviceType : uint8_t {
  RaspberryPi3,
  RaspberryPi4,
};

/** The MMIO area base address. Initialized by MMIO::init(); */
extern uint64_t BASE;
extern DeviceType device;

/** Initializes the memory-mapped IO system. */
void init(const DeviceTree& dt);

/** Writes @a value into the memory-mapped IO @a reg register. */
[[gnu::always_inline]] static inline void write(uint32_t reg, uint32_t value) {
  // We need to keep volatile to avoid the compiler
  // to optimize out the memory write.

  asm volatile("str %w1, [%x0]" : : "r"(reg + BASE), "r"(value));
}

/** Reads from the memory-mapped IO @a reg register. */
[[gnu::always_inline]] static inline uint32_t read(uint32_t reg) {
  // We need to keep volatile to avoid the compiler
  // to optimize out the memory read.

  uint32_t value;
  asm volatile("ldr %w1, [%x0]" : "=r"(value) : "r"(reg + BASE));
  return value;
}
};  // namespace MMIO
