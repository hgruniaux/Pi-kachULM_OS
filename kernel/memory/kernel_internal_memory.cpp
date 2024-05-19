#include "kernel_internal_memory.hpp"

#include <libk/assert.hpp>
#include "boot/mmu_utils.hpp"

static inline constexpr PagesAttributes custom_memory_rw = {.sh = Shareability::InnerShareable,
                                                            .exec = ExecutionPermission::NeverExecute,
                                                            .rw = ReadWritePermission::ReadWrite,
                                                            .access = Accessibility::Privileged,
                                                            .type = MemoryType::Normal};

static MMUTable _tbl;
static PageAllocList _page_alloc;
static VirtualPA _custom_pages = CUSTOM_PAGES_MEMORY;

VirtualPA mmu_resolve_pa(void*, PhysicalPA page_address) {
  return page_address + KERNEL_BASE;
}

PhysicalPA mmu_resolve_va(void*, VirtualPA page_address) {
  return page_address - KERNEL_BASE;
}

VirtualPA mmu_alloc_page(void*) {
  PhysicalPA addr = -1;

  if (!_page_alloc.fresh_page(&addr)) {
    libk::panic("[KernelInternalMemory] Unable to allocate a page for a MMU Table.");
  }

  const VirtualPA va = mmu_resolve_pa(nullptr, addr);
  zero_pages(va, 1);

  return va;
}

void mmu_free_page(void*, VirtualPA page_address) {
  const PhysicalPA addr = mmu_resolve_va(nullptr, page_address);

  _page_alloc.free_page(addr);
}

void memory_impl::init() {
  /* Set up the PageAllocList */
  constexpr size_t linear_allocator_size = 1;  //<! Number of page reserved by the internal Linear Memory Allocator
  _page_alloc = PageAllocList(linear_allocator_size);

  /* Set up the MMUTable */
  _tbl = {
      .kind = MMUTable::Kind::Kernel,
      .pgd = _init_data.pgd,
      .asid = 0,
      .handle = nullptr,
      .alloc = &mmu_alloc_page,
      .free = &mmu_free_page,
      .resolve_pa = &mmu_resolve_pa,
      .resolve_va = &mmu_resolve_va,
  };
}

PageAllocList* memory_impl::get_kernel_alloc() {
  return &_page_alloc;
}

MMUTable* memory_impl::get_kernel_tbl() {
  return &_tbl;
}

MMUTable memory_impl::new_process_tbl(uint8_t asid) {
  const VirtualPA process_pgd = mmu_alloc_page(nullptr);

  return {
      .kind = MMUTable::Kind::Process,
      .pgd = process_pgd,
      .asid = asid,
      .handle = nullptr,
      .alloc = &mmu_alloc_page,
      .free = &mmu_free_page,
      .resolve_pa = &mmu_resolve_pa,
      .resolve_va = &mmu_resolve_va,
  };
}

PhysicalPA memory_impl::resolve_table_pgd(const MMUTable& tbl) {
  return mmu_resolve_va(nullptr, tbl.pgd);
}

void memory_impl::delete_process_tbl(MMUTable& tbl) {
  clear_all(&tbl);
  mmu_free_page(nullptr, tbl.pgd);
  tbl.pgd = 0;
}

VirtualPA memory_impl::allocate_pages_section(const size_t nb_pages, PhysicalPA* pages_ptr) {
  const VirtualPA section_start = _custom_pages;

  for (size_t page_id = 0; page_id < nb_pages; ++page_id) {
    if (!_page_alloc.fresh_page(&(pages_ptr[page_id]))) {
      return 0;
    }

    if (!map_range(&_tbl, _custom_pages, _custom_pages, pages_ptr[page_id], custom_memory_rw)) {
      return 0;
    }

    zero_pages(_custom_pages, 1);
    _custom_pages += PAGE_SIZE;
  }

  // Create a gap between each custom segment.
  _custom_pages += PAGE_SIZE;

  return section_start;
}

void memory_impl::free_section(size_t nb_pages, VirtualPA kernel_va, PhysicalPA* pages_ptr) {
  if (!unmap_range(&_tbl, kernel_va, kernel_va + (nb_pages - 1) * PAGE_SIZE)) {
    libk::panic("Failed to free a custom memory chunk!");
  }

  for (size_t page_id = 0; page_id < nb_pages; ++page_id) {
    _page_alloc.free_page(pages_ptr[page_id]);
  }
}
