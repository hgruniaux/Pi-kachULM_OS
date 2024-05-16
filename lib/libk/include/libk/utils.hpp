#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "assert.hpp"
#include "qemu.hpp"

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

/** Returns a mask that covers bit from @a start to @a end *INCLUDED* */
static inline constexpr uint64_t mask_bits(size_t start, size_t end) {
  if (end < start) {
    return 0;
  } else {
    return ((((uint64_t)1) << (end - start + 1)) - 1) << start;
  }
}

/** Return a / b rounded up */
template <class U, class V>
static inline constexpr U div_round_up(U a, V b) {
  return (a + b - 1) / b;
}

/** Return a / b rounded down */
template <class U, class V>
static inline constexpr U div_round_down(U a, V b) {
  return a / b;
}

/** @brief The NOP instruction. */
[[gnu::always_inline]] static inline void nop() {
  asm volatile("nop");
}

/** @brief The YIELD instruction. */
[[gnu::always_inline]] static inline void yield() {
  asm volatile("yield");
}

/** @brief The WFE instruction. */
[[gnu::always_inline]] static inline void wfe() {
  asm volatile("wfe");
}

/** @brief The WFI instruction. */
[[gnu::always_inline]] static inline void wfi() {
  asm volatile("wfi");
}

/** @brief Halt the CPU (enter into a infinite loop). */
[[noreturn, gnu::always_inline]] static inline void halt() {
  // The `asm volatile` is required here to avoid Clang to optimize away the infinite loop.
  // Another possibility is to call nop() inside the loop, but it adds the NOP instruction
  // whereas using an empty asm statement adds nothing (it just avoids the optimization).
#ifdef TARGET_QEMU
  qemu_exit(1);
#else
  while (true) {
    asm volatile("");
  }
#endif
}

/** @brief Checks if @a n is a power of two. */
template <std::unsigned_integral T>
[[nodiscard]] static inline constexpr bool is_power_of_two(T n) {
  return (n > 0 && ((n & (n - 1)) == 0));
}

template <class T>
concept unsigned_or_pointer = std::is_unsigned_v<T> || std::is_pointer_v<T>;

/** @brief Aligns @a value to the requested @a alignment (is a power of two) by adding. */
template <unsigned_or_pointer T>
[[nodiscard]] static inline constexpr T align_to_next(T value, size_t alignment) {
  KASSERT(is_power_of_two(alignment));
  // Expression used by LD for ALIGN(xxx)
  const size_t mask = alignment - 1;
  return (T)(((uintptr_t)value + mask) & ~(mask));
}

/** @brief Aligns @a value to the requested @a alignment (is a power of two) by subtracting. */
template <unsigned_or_pointer T>
[[nodiscard]] static inline constexpr T align_to_previous(T value, size_t alignment) {
  KASSERT(is_power_of_two(alignment));
  // Expression used by LD for ALIGN(xxx)
  const size_t mask = alignment - 1;
  return (T)((uintptr_t)value & ~(mask));
}

/** @brief Wait the specified amount of cycles. */
static inline void wait_cycles(uint64_t count) {
  asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n" : "=r"(count) : [count] "0"(count) : "cc");
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
#if __BYTE_ORDER == __LITTLE_ENDIAN
  return value;
#else
  return bswap(value);
#endif
}

/** @brief Converts a native endianness @a value to little-endian. */
template <std::unsigned_integral T>
[[gnu::always_inline]] static inline T to_le(T value) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
  return value;
#else
  return bswap(value);
#endif
}

/** @brief Converts a big-endian @a value to the native endianness. */
template <std::unsigned_integral T>
[[gnu::always_inline]] static inline T from_be(T value) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
  return bswap(value);
#else
  return value;
#endif
}

/** @brief Converts a native endianness @a value to big-endian. */
template <std::unsigned_integral T>
[[gnu::always_inline]] static inline T to_be(T value) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
  return bswap(value);
#else
  return value;
#endif
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
