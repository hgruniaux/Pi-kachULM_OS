#pragma once

#include <cstdint>
#include "memory_page.hpp"
#include "mmu_table.hpp"

namespace KernelMemory {
enum class Kind { Invalid, Reserved, VideoCore, Device, CustomPage, Heap, Stack, Process };

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

/** Allocate a new page into @a page
 * @returns - `true` in case of success, @a page is modified.
 *          - `false` in case of failure, @a page is left untouched. */
bool new_page(MemoryPage* page);

};  // namespace KernelMemory
