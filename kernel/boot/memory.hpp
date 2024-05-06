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

/** Converts the Virtual Address from the VideoCore reserved memory @a vc_va to a Physical one. */
[[nodiscard]] uintptr_t get_vc_pa(uintptr_t vc_va);

/** Converts the Physical Address from the VideoCore reserved memory @a vc_pa to a Virtual one. */
[[nodiscard]] uintptr_t get_vc_va(uintptr_t vc_pa);

/** Returns the start of the Kernel Heap. */
[[nodiscard]] uintptr_t get_heap_start() {
  return HEAP_MEMORY;
}

/** Returns the end of the Kernel Heap. */
[[nodiscard]] uintptr_t get_heap_end();

/** Returns the end of the Kernel Heap. */
[[nodiscard]] size_t get_heap_byte_size();

/** Change Kernel Heap end by adding @a page_offset *pages*
 * @a returns the new end of the heap, or zero in case of no free page left */
uintptr_t change_heap_end(long page_offset);

};  // namespace KernelMemory
