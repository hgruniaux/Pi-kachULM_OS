#include "memory/page_builder.hpp"
#include <libk/log.hpp>
#include "boot/mmu_utils.hpp"

static inline constexpr PagesAttributes kernel_rw_memory = {.sh = Shareability::InnerShareable,
                                                            .exec = ExecutionPermission::NeverExecute,
                                                            .rw = ReadWritePermission::ReadWrite,
                                                            .access = Accessibility::Privileged,
                                                            .type = MemoryType::Normal};

MemoryPageBuilder::MemoryPageBuilder(PageAllocList page_alloc, MMUTable* table) : _alloc(page_alloc), _tbl(table) {}

VirtualPA MemoryPageBuilder::change_heap_end(long byte_offset) {
  const uintptr_t target_heap_size = get_heap_end() + byte_offset;

  while (get_heap_end() < target_heap_size) {
    // Increase heap here.

    PhysicalPA new_heap_pa_page = -1;

    if (!_alloc.fresh_page(&new_heap_pa_page)) {
      return 0;
    }

    if (!map_range(_tbl, get_heap_end(), get_heap_end(), new_heap_pa_page, kernel_rw_memory)) {
      return 0;
    }

    _heap_size += PAGE_SIZE;
  }

  while (get_heap_end() - PAGE_SIZE > target_heap_size) {
    // Decrease heap here.

    const VirtualPA va_to_del = get_heap_end() - PAGE_SIZE;
    const PhysicalPA pa_to_del = mmu_resolve_va(va_to_del);

    if (!unmap_range(_tbl, va_to_del, va_to_del)) {
      return 0;
    }

    _alloc.free_page(pa_to_del);

    _heap_size -= PAGE_SIZE;
  }

  return get_heap_end();
}

PhysicalPA MemoryPageBuilder::mmu_resolve_va(VirtualAddress va) {
  asm volatile("at s1e1w, %x0" ::"r"(va));

  uint64_t par_el1;
  asm volatile("mrs %x0, PAR_EL1" : "=r"(par_el1));

  if (par_el1 & 0x1) {
    libk::panic("[KernelMemory] Unable to resolve physical address from MMU.");
  }

  return par_el1 & libk::mask_bits(12, 47);
}

VirtualPA MemoryPageBuilder::get_heap_end() const {
  return HEAP_MEMORY + get_heap_size();
}

size_t MemoryPageBuilder::get_heap_size() const {
  return _heap_size;
}
