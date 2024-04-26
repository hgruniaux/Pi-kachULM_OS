#pragma once

#include <cstddef>
#include <cstdint>

#include "mmu_defs.hpp"

using AllocFun = VirtualPA (*)(void *); //<! This function fail internally if the allocation fail
using FreeFun = void (*)(void *, VirtualPA);

using ResolvePA = VirtualPA (*)(void *, PhysicalPA); //<! This function fail internally if the conversion fail
using ResolveVA = PhysicalPA (*)(void *, VirtualPA); //<! This function fail internally if the conversion fail

struct MMUTable {
  enum class Kind : uintptr_t { Kernel = KERNEL_BASE, Process = PROCESS_BASE };

  const Kind kind;      //<! The kind of Memory mapping we're dealing with.
  const VirtualPA pgd;  //<! The top level of the MMU table
  const uint8_t asid;   //<! The Address Space Identifier, used to differentiate between process memory spaces.

  void *handle;          //<! The handle passed to the following function, in case they need some context of theirs
  const AllocFun alloc;  //<! The function used to allocate a page
  const FreeFun free;    //<! The function used to free a page
  const ResolvePA resolve_pa;  //<! The function used to convert a physical address to a virtual one
  const ResolveVA resolve_va;  //<! The function used to convert a virtual address to a physical one
};

/** Finds, if it exists, the entry specific to the virtual address @a va.
 * In case @a attr is not null, it will contain the parameters associated with the entry.
 *
 * @returns - `true` if the entry exists, @a attr is modified if not null. @n
 *          - `false` if the entry is not found, @a attr is not modified.
 */
[[nodiscard]] bool get_attr(const MMUTable *table, VirtualPA va, PagesAttributes *attr);

/** Change parameters associated with the virtual address @a va.
 *
 * @returns - `true` if the operation was completed successfully. @n
 *          - `false` if the entry is not found.
 */
[[nodiscard]] bool change_attr_va(MMUTable *table, VirtualPA va, PagesAttributes attr);

/** Maps the virtual address range from @a va_start to @a va_end *INCLUSIVE*
 * to the physical addresses @a pa_start and following, using the attributes @a attr.
 * Be careful, this operation *will overwrite* any already existing mapping for the specified range.
 * Mapping is done in a way to minimize the number of tables used.
 *
 * @returns - `true` if the full operation was completed successfully. @n
 *          - `false` if certain prerequisites are not checked, the table is not modified.
 */
[[nodiscard]] bool map_range(MMUTable *table,
                             VirtualPA va_start,
                             VirtualPA va_end,
                             PhysicalPA pa_start,
                             PagesAttributes attr);

/** Unmaps the virtual address range from @a va_start to @a va_end *INCLUSIVE*.
 *
 * @returns - `true` if the full operation was completed successfully. @n
 *          - `false` if certain prerequisites are not checked. The table is not modified in this case.
 */
[[nodiscard]] bool unmap_range(MMUTable *table, VirtualPA va_start, VirtualPA va_end);

/** Change parameters associated with the virtual address range from @a va_start to @a va_end *INCLUSIVE*.
 *
 * @returns - `true` if the full operation was completed successfully. @n
 *          - `false` if certain prerequisites are not checked. The table is not modified in this case.
 */
[[nodiscard]] bool change_attr_range(MMUTable *table, VirtualPA va_start, VirtualPA va_end, PagesAttributes attr);

/** Clear the whole table, deallocating all used pages and unmapping everything. */
void clear_all(MMUTable *table);
