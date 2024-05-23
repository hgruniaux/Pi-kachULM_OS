#pragma once

#include <cstddef>
#include <cstdint>

#include <cstdint>
#include "memory/memory.hpp"

enum class Shareability : uint8_t {
  NonShareable = 0b00,    //<! Memory not shared at all
  OuterShareable = 0b10,  //<! Memory shared across multiples peripheral
  InnerShareable = 0b11,  //<! Memory shared across CPU cores
};

enum class ExecutionPermission : uint8_t {
  // {UXN, PXN} = {0, 0}
  // {UXN, PXN} = {0, 1}
  // {UXN, PXN} = {1, 0}
  // {UXN, PXN} = {1, 1}

  AllExecute = 0b00,         //<! Kernel and Process can execute the memory
  ProcessExecute = 0b01,     //<! Only the Process can execute the memory
  PrivilegedExecute = 0b10,  //<! Only the Kernel can execute the memory
  NeverExecute = 0b11,       //<! No one can execute the memory
};

enum class ReadWritePermission : uint8_t {
  ReadWrite = 0b0,  //<! Read and write allowed on the memory
  ReadOnly = 0b1,   //<! Only read allowed on the memory
};

enum class Accessibility : uint8_t {
  Privileged = 0b0,  //<! Read and write (if allowed) only allowed by the Kernel
  AllProcess = 0b1,  //<! Read and write (if allowed) allowed by everyone (Kernel included)
};

enum class MemoryType : uint8_t {
  Normal = 0,          //<! Normal memory cached
  Device_nGnRnE = 1,   //<! Device memory without any Gathering, Reordering nor Elimination
  Device_nGRE = 2,     //<! Device memory without any Gathering but Reordering and Elimination allowed
  Normal_NoCache = 3,  //<! Normal memory not cached
};

struct PagesAttributes {
  Shareability sh;
  ExecutionPermission exec;
  ReadWritePermission rw;
  Accessibility access;
  MemoryType type;
};

using AllocFun = VirtualPA (*)(void*);  //<! This function fail internally if the allocation fail
using FreeFun = void (*)(void*, VirtualPA);

using ResolvePA = VirtualPA (*)(void*, PhysicalPA);  //<! This function fail internally if the conversion fail
using ResolveVA = PhysicalPA (*)(void*, VirtualPA);  //<! This function fail internally if the conversion fail

struct MMUTable {
  enum class Kind : uintptr_t { Kernel, Process };

  Kind kind;      //<! The kind of Memory mapping we're dealing with.
  VirtualPA pgd;  //<! The top level of the MMU table
  uint8_t asid;   //<! The Address Space Identifier, used to differentiate between process memory spaces.

  void* handle;          //<! The handle passed to the following function, in case they need some context of theirs
  AllocFun alloc;        //<! The function used to allocate a page
  FreeFun free;          //<! The function used to free a page
  ResolvePA resolve_pa;  //<! The function used to convert a physical address to a virtual one
  ResolveVA resolve_va;  //<! The function used to convert a virtual address to a physical one
};

/** Finds, if it exists, the entry specific to the virtual address @a va.
 * In case @a attr is not null, it will contain the parameters associated with the entry.
 *
 * @returns - `true` if the entry exists, @a attr is modified if not null. @n
 *          - `false` if the entry is not found, @a attr is not modified.
 */
[[nodiscard]] bool get_attr(const MMUTable* table, VirtualPA va, PagesAttributes* attr);

/** Change parameters associated with the virtual address @a va.
 *
 * @returns - `true` if the operation was completed successfully. @n
 *          - `false` if the entry is not found.
 */
[[nodiscard]] bool change_attr_va(MMUTable* table, VirtualPA va, PagesAttributes attr);

/** Maps the virtual address range from @a va_start to @a va_end *INCLUSIVE*
 * to the physical addresses @a pa_start and following, using the attributes @a attr.
 * Be careful, this operation *will overwrite* any already existing mapping for the specified range.
 * Mapping is done in a way to minimize the number of tables used.
 *
 * @returns - `true` if the full operation was completed successfully. @n
 *          - `false` if certain prerequisites are not checked, the table is not modified.
 */
[[nodiscard]] bool map_range(MMUTable* table,
                             VirtualPA va_start,
                             VirtualPA va_end,
                             PhysicalPA pa_start,
                             PagesAttributes attr);

/** Unmaps the virtual address range from @a va_start to @a va_end *INCLUSIVE*.
 *
 * @returns - `true` if the full operation was completed successfully. @n
 *          - `false` if certain prerequisites are not checked. The table is not modified in this case.
 */
[[nodiscard]] bool unmap_range(MMUTable* table, VirtualPA va_start, VirtualPA va_end);

/** Change parameters associated with the virtual address range from @a va_start to @a va_end *INCLUSIVE*.
 *
 * @returns - `true` if the full operation was completed successfully. @n
 *          - `false` if certain prerequisites are not checked. The table is not modified in this case.
 */
[[nodiscard]] bool change_attr_range(MMUTable* table, VirtualPA va_start, VirtualPA va_end, PagesAttributes attr);

/** Clear the whole table, deallocating all used pages and unmapping everything. */
void clear_all(MMUTable* table);

/** Refresh the TLB for this page mapping. */
void reload_tlb();
