#pragma once
#include <dtb/dtb.hpp>

#include <cstdint>

namespace MMIO {
enum class DeviceType : uint8_t {
  RaspberryPi3,
  RaspberryPi4,
};

/** The MMIO area base address. Initialized by MMIO::init(); */
extern uintptr_t BASE;
extern DeviceType device;

/** Initializes the memory-mapped IO system. */
void init();

/** Writes @a value into the memory-mapped IO @a reg register. */
[[gnu::always_inline]] static inline void write(uintptr_t reg, uint32_t value) {
  // We need to keep volatile to avoid the compiler
  // to optimize out the memory write.
  libk::write32(reg + BASE, value);
}

/** Reads from the memory-mapped IO @a reg register. */
[[gnu::always_inline]] static inline uint32_t read(uintptr_t reg) {
  // We need to keep volatile to avoid the compiler
  // to optimize out the memory read.
  return libk::read32(reg + BASE);
}
};  // namespace MMIO
