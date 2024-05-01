#pragma once

#include <cstdint>
#include "mmu_defs.hpp"

namespace Memory {
enum class Kind { Normal, VC, Device, Heap, Stack };

/** Returns the kind of @a address. */
Kind get_address_kind(uintptr_t address);

/** Converts the Virtual Address from the VideoCore reserved memory @a vc_va to a Physical one. */
uintptr_t get_vc_pa(uintptr_t vc_va);

/** Converts the Physical Address from the VideoCore reserved memory @a vc_pa to a Virtual one. */
uintptr_t get_vc_va(uintptr_t vc_pa);

/** Returns the start of the Kernel Heap. */
uintptr_t get_heap_start();

/** Returns the end of the Kernel Heap. */
uintptr_t get_heap_end();

/** Change Kernel Heap end by adding @a offsets *pages**/
uintptr_t change_heap_end(long offset);

}  // namespace Memory
