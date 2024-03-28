#pragma once

#include <cstddef>
#include <cstdint>

static constexpr uint32_t DTB_MAGIC = 0xd00dfeed;
static constexpr uint32_t DTB_BEGIN_NODE = 0x01;
static constexpr uint32_t DTB_END_NODE = 0x02;
static constexpr uint32_t DTB_PROP = 0x03;
static constexpr uint32_t DTB_NOP = 0x04;
static constexpr uint32_t DTB_END = 0x09;

/** Align a T* pointer with the specified byte boundary. */
template <typename T>
static inline T align_pointer(T ptr, size_t alignment) {
  const auto addr = (uintptr_t)ptr;
  const uintptr_t new_addr = addr ^ (addr & (alignment - 1));
  if (new_addr < addr) {
    return (T)(new_addr + alignment);
  }
  return (T)new_addr;
}
