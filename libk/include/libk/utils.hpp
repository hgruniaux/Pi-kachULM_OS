#pragma once

#include <concepts>
#include <cstdint>
#include <type_traits>

#define KUNUSED(var) ((void)(var))

namespace libk {
/** @brief Returns the minimum of @a a and @a b. */
template <class T>
static inline constexpr T min(T a, T b) {
  return (a < b) ? a : b;
}

/** @brief Returns the maximum of @a a and @a b. */
template <class T>
static inline constexpr T max(T a, T b) {
  return (a < b) ? b : a;
}

/** @brief Clamps the given @a val into the range [@a min_val, @a max_val]. */
template <class T>
static inline constexpr T clamp(T val, T min_val, T max_val) {
  return min(max_val, max(min_val, val));
}

/** @brief The NOP instruction. */
[[gnu::always_inline]] static inline void nop() {
  asm volatile("nop");
}

/** @brief The YIELD instruction. */
[[gnu::always_inline]] static inline void yield() {
  asm volatile("yield");
}

/** @brief Halt the CPU (enter into a infinite loop). */
[[noreturn, gnu::always_inline]] static inline void halt() {
  // The `asm volatile` is required here to avoid Clang to optimize away the infinite loop.
  // Another possibility is to call nop() inside the loop, but it adds the NOP instruction
  // whereas using an empty asm statement adds nothing (it just avoids the optimization).
  while (true)
    asm volatile("");
}

/** @brief Reverses the bytes in @a value. */
[[gnu::always_inline]] static inline uint8_t bswap(uint8_t value) {
  // No bytes to reverse, but provided for the sake of completeness.
  return value;
}

/** @brief Reverses the bytes in @a value. */
[[gnu::always_inline]] static inline uint16_t bswap(uint16_t value) {
  return __builtin_bswap16(value);
}

/** @brief Reverses the bytes in @a value. */
[[gnu::always_inline]] static inline uint32_t bswap(uint32_t value) {
  return __builtin_bswap32(value);
}

/** @brief Reverses the bytes in @a value. */
[[gnu::always_inline]] static inline uint64_t bswap(uint64_t value) {
  return __builtin_bswap64(value);
}

/** @brief Converts a little-endian @a value to the native endianness. */
template <std::unsigned_integral T>
[[gnu::always_inline]] static inline T from_le(T value) {
  // ARM is natively little-endian, no conversion needed.
  return value;
}

/** @brief Converts a big-endian @a value to the native endianness. */
template <std::unsigned_integral T>
[[gnu::always_inline]] static inline T from_be(T value) {
  // ARM is natively little-endian, byte swapping needed.
  return bswap(value);
}

/** @brief Reads a 8-bit word at the given memory @a address. */
[[gnu::always_inline]] static inline uint8_t read8(uintptr_t address) {
  return *(const volatile uint8_t*)address;
}

/** @brief Reads a 16-bit word at the given memory @a address. */
[[gnu::always_inline]] static inline uint16_t read16(uintptr_t address) {
  return *(const volatile uint16_t*)address;
}

/** @brief Reads a 32-bit word at the given memory @a address. */
[[gnu::always_inline]] static inline uint32_t read32(uintptr_t address) {
  return *(const volatile uint32_t*)address;
}

/** @brief Reads a 64-bit word at the given memory @a address. */
[[gnu::always_inline]] static inline uint64_t read64(uintptr_t address) {
  return *(const volatile uint64_t*)address;
}

/** @brief Writes the 8-bit word @a value at the given memory @a address. */
[[gnu::always_inline]] static inline void write8(uintptr_t address, uint8_t value) {
  *(volatile uint8_t*)address = value;
}

/** @brief Writes the 16-bit word @a value at the given memory @a address. */
[[gnu::always_inline]] static inline void write16(uintptr_t address, uint16_t value) {
  *(volatile uint16_t*)address = value;
}

/** @brief Writes the 32-bit word @a value at the given memory @a address. */
[[gnu::always_inline]] static inline void write32(uintptr_t address, uint32_t value) {
  *(volatile uint32_t*)address = value;
}

/** @brief Writes the 64-bit word @a value at the given memory @a address. */
[[gnu::always_inline]] static inline void write64(uintptr_t address, uint64_t value) {
  *(volatile uint64_t*)address = value;
}
}  // namespace libk
