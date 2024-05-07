#pragma once

#include <cstdint>
#include "dtb/reserved_sections.hpp"

#include "mmu_table.hpp"
#include "page_alloc.hpp"

namespace KernelMemory {
enum class Kind { Invalid, Reserved, VC, Device, Special, Heap, Stack, Process };

[[nodiscard]] bool init();

/** Returns the kind of @a address. */
[[nodiscard]] Kind get_address_kind(uintptr_t address);

/** Returns the start of the Kernel Heap. */
[[nodiscard]] VirtualPA get_heap_start();

/** Returns the end of the Kernel Heap. */
[[nodiscard]] VirtualPA get_heap_end();

/** Returns the end of the Kernel Heap. */
[[nodiscard]] size_t get_heap_byte_size();

/** Change Kernel Heap end by adding @a page_offset *bytes*
 * @a returns the new end of the heap, or zero in case of no free page left */
VirtualPA change_heap_end(long byte_offset);

/** @returns the amount of memory used to manage the memory */
size_t get_memory_overhead();
};  // namespace KernelMemory
