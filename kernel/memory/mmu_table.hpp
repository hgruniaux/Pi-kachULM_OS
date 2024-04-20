#pragma once

#include <cstddef>
#include <cstdint>

#include "libk/utils.hpp"
#include "mmu_defs.hpp"

using AllocFun = bool (*)(void*, VirtualPA*);
using FreeFun = void (*)(void*, VirtualPA);

using ResolvePA = bool (*)(void*, PhysicalPA, VirtualPA*);
using ResolveVA = bool (*)(void*, VirtualPA, PhysicalPA*);

struct MMUTable {
  enum class Kind : uint8_t { Kernel, Process };

  const Kind kind;            //<! The kind of Memory mapping we're dealing with.
  const VirtualPA pgd;  //<! The top level of the MMU table
  const uint8_t asid;         //<! The Address Space Identifier, used to differentiate between process memory spaces.

  void* handle;          //<! The handle passed to the following function, in case they need some context of theirs
  const AllocFun alloc;  //<! The function used to allocate a page
  const FreeFun free;    //<! The function used to free a page
  const ResolvePA resolve_pa;  //<! The function used to convert a physical address to a virtual one
  const ResolveVA resolve_va;  //<! The function used to convert a virtual address to a physical one
};

///** Maps a chunk of @a amount memory page from @a va to @a pa with attributes @a attr
// * @returns - `true` iff the specified chunk is now mapped. @n
// *          - `false` iff @a va and @a amount does not refer to a atomic chunk of memory
// *          or an error occurred during a page allocation. */
//[[nodiscard]] bool map_chunk(MMUTable* table,
//                             VirtualPA va,
//                             PhysicalPA pa,
//                             PageSize amount,
//                             PagesAttributes attr);
//
///** Unmap the chunk of @a amount memory starting from @a va.
// * @returns - `true` iff the specified chunk is now unmapped. @n
// *          - `false` iff @a va and @a amount does not refer to a atomic chunk of memory. */
// bool unmap_chunk(MMUTable* table, VirtualPA va, PageSize amount);

/** Maps the virtual address range from @a va_start to @a va_end *INCLUDED*
 * to the physical addresses @a pa_start and following, using @a attr attributes.
 * Mapping is performed to minimize the number of tables used.
 *
 * @returns - `true` if complete mapping has been performed successfully. @n
 *          - `false` if some or all of the mapping was not successful.
 *          The table is left in an unknown, possibly invalid, state.
 */
[[nodiscard]] bool map_range(MMUTable* table,
                             VirtualPA va_start,
                             VirtualPA va_end,
                             PhysicalPA pa_start,
                             PagesAttributes attr);

// TODO : DOC
bool unmap_range(MMUTable* table, VirtualPA va_start, VirtualPA va_end);

/** Checks if a there is an entry mapping @a amount of memory starting from @a va. */
[[nodiscard]] bool has_entry_at(const MMUTable* const table, VirtualPA va, PageSize amount);

/** Clear the whole table, deallocating all used pages and unmapping everything. */
void clear_all(MMUTable* table);

/** Address Translate instruction wrapper.
 * With the help of @a acc and @a rw, call the correct `at` instruction with @a va. */
static inline void at_instr(VirtualPA va, Accessibility acc, ReadWritePermission rw) {
  switch (acc) {
    case Accessibility::Privileged: {
      switch (rw) {
        case ReadWritePermission::ReadWrite:
          asm volatile("at s1e1w, %x0" ::"r"(va));
          return;

        case ReadWritePermission::ReadOnly:
          asm volatile("at s1e1r, %x0" ::"r"(va));
          return;
      }
    }
    case Accessibility::AllProcess: {
      switch (rw) {
        case ReadWritePermission::ReadWrite:
          asm volatile("at s1e0w, %x0" ::"r"(va));
          return;

        case ReadWritePermission::ReadOnly:
          asm volatile("at s1e0r, %x0" ::"r"(va));
          return;
      }
    }
  }
}

/** Retrieve a physical address from PAR_EL1.
 * @returns - `true` if conversion successful, in this case, @a pa is modified @n
 *          - `false` if conversion cannot be operated. @a pa is unmodified. */
static inline bool parse_PAR_EL1(PhysicalPA* pa) {
  uint64_t physical_addr;
  asm volatile("mrs %x0, PAR_EL1" : "=r"(physical_addr));
  if ((physical_addr & 0b1) == 0) {
    *pa = PhysicalPA(physical_addr & libk::mask_bits(47, 12));
    return true;
  } else {
    return false;
  }
}
