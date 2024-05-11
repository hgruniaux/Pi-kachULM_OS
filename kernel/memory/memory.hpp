#pragma once

#include <cstdint>
#include <cstddef>

using PhysicalPA = uintptr_t;
using VirtualPA = uintptr_t;

using PhysicalAddress = uintptr_t;
using VirtualAddress = uintptr_t;

namespace KernelMemory {
void init();

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

/** Convert a virtual address referring to the VideoCore memory to its physical address. */
PhysicalAddress get_physical_vc_address(VirtualAddress vc_addr);

/** Convert a physical address referring to the VideoCore memory to a virtual one. */
VirtualAddress get_virtual_vc_address(PhysicalAddress vc_addr);

/** Convert a virtual address to its physical one. Only suitable for general use memory. */
PhysicalAddress resolve_physical_address(VirtualAddress);
};  // namespace KernelMemory
