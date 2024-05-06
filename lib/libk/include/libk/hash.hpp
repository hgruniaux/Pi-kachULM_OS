#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>

#include "string.hpp"

namespace libk {
[[nodiscard]] constexpr inline uint64_t hash(uint64_t x) {
  // See https://stackoverflow.com/a/12996028/8959979
  // This function is fast and bijective (so no collisions).
  x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
  x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
  x = x ^ (x >> 31);
  return x;
}

template<std::integral T>
[[nodiscard]] uint64_t hash(T x) {
  return hash(static_cast<uint64_t>(x));
}

template<class T>
[[nodiscard]] constexpr inline uint64_t hash(const T *x) {
  return hash(static_cast<uintptr_t>(x));
}

[[nodiscard]] constexpr inline uint64_t hash(const uint8_t *data, size_t data_len) {
  // Constants for 64-bits hash.
  constexpr uint64_t FNV_OFFSET_BASIS = UINT64_C(0xcbf29ce484222325);
  constexpr uint64_t FNV_PRIME = UINT64_C(0x100000001b3);

  // We use the FNV-1a hash algorithm.
  uint64_t h = FNV_OFFSET_BASIS;
  for (size_t i = 0; i < data_len; ++i) {
    h ^= data[i];
    h *= FNV_PRIME;
  }

  return h;
}

[[nodiscard]] inline uint64_t hash(const char *data) {
  return hash(reinterpret_cast<const uint8_t *>(data), strlen(data));
}

template<typename T, typename... Rest>
constexpr inline void hash_combine(uint64_t &seed, const T &v, const Rest &... rest) {
  seed ^= hash(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  (hash_combine(seed, rest), ...);
}

template<typename... Args>
[[nodiscard]] constexpr inline uint64_t hash_multiple(const Args &... args) {
  uint64_t seed = 0;
  (hash_combine(seed, args), ...);
  return seed;
}
} // namespace libk
