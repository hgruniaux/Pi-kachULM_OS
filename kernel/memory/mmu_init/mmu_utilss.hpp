#pragma once
#include "../mmu_defs.hpp"
#include "../mmu_utils.hpp"

struct LinearPhysicalAlloc {
  const libk::PhysicalPA first_page;
  const libk::PhysicalPA upper_bound;
  size_t nb_allocated;
};

[[nodiscard]] bool alloc_page(LinearPhysicalAlloc* alloc, libk::VirtualPA* page);

// TODO : Add Doc
/** All bound are *INCLUSIVE* */
[[nodiscard]] bool map_range_in_table(libk::VirtualPA va_start,
                                      libk::VirtualPA va_end,
                                      libk::PhysicalPA pa_start,
                                      PagesAttributes attr,
                                      uint64_t* table,
                                      size_t table_level,
                                      libk::VirtualPA table_first_page_va,
                                      LinearPhysicalAlloc* alloc);

// TODO : Add Doc
/** All bound are *INCLUSIVE* */
[[nodiscard]] bool change_properties_for_range(libk::VirtualPA va_start,
                                               libk::VirtualPA va_end,
                                               PagesAttributes attr,
                                               uint64_t* table,
                                               size_t table_level,
                                               libk::VirtualPA table_first_page_va,
                                               LinearPhysicalAlloc* alloc);

#define resolve_symbol_pa(symbol)                     \
  ({                                                  \
    uintptr_t __dest;                                 \
    asm volatile("adr %x0, " #symbol : "=r"(__dest)); \
    __dest;                                           \
  })

#define resolve_symbol_va(symbol)                      \
  ({                                                   \
    uintptr_t __dest;                                  \
    asm volatile("ldr %x0, =" #symbol : "=r"(__dest)); \
    __dest;                                            \
  })

#define map_range(va_start, va_end, pa_start, attr)                                                     \
  {                                                                                                     \
    if (!map_range_in_table(va_start, va_end, pa_start, attr, (uint64_t*)pgd, 1, KERNEL_BASE, alloc)) { \
      libk::halt();                                                                                     \
    };                                                                                                  \
  }

static inline constexpr PagesAttributes kernel_code = {.sh = Shareability::InnerShareable,
                                                       .exec = ExecutionPermission::PrivilegedExecute,
                                                       .rw = ReadWritePermission::ReadOnly,
                                                       .access = Accessibility::Privileged,
                                                       .type = MemoryType::Normal};

static inline constexpr PagesAttributes rw_memory = {.sh = Shareability::InnerShareable,
                                                     .exec = ExecutionPermission::NeverExecute,
                                                     .rw = ReadWritePermission::ReadWrite,
                                                     .access = Accessibility::Privileged,
                                                     .type = MemoryType::Normal};

static inline constexpr PagesAttributes ro_memory = {.sh = Shareability::InnerShareable,
                                                     .exec = ExecutionPermission::NeverExecute,
                                                     .rw = ReadWritePermission::ReadOnly,
                                                     .access = Accessibility::Privileged,
                                                     .type = MemoryType::Normal};

static inline constexpr PagesAttributes device_memory = {.sh = Shareability::OuterShareable,
                                                         .exec = ExecutionPermission::NeverExecute,
                                                         .rw = ReadWritePermission::ReadWrite,
                                                         .access = Accessibility::Privileged,
                                                         .type = MemoryType::Device_nGnRnE};
