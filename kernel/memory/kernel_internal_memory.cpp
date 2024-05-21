#include "kernel_internal_memory.hpp"

#include <libk/assert.hpp>
#include <libk/utils.hpp>
#include "boot/mmu_utils.hpp"
#include "contiguous_page_alloc.hpp"
#include "fs/fat/ramdisk.hpp"
#include "hardware/kernel_dt.hpp"
#include "libk/log.hpp"

static inline constexpr PagesAttributes custom_memory_rw = {.sh = Shareability::InnerShareable,
                                                            .exec = ExecutionPermission::NeverExecute,
                                                            .rw = ReadWritePermission::ReadWrite,
                                                            .access = Accessibility::Privileged,
                                                            .type = MemoryType::Normal};

static inline constexpr PagesAttributes buffer_memory_rw = {.sh = Shareability::OuterShareable,
                                                            .exec = ExecutionPermission::NeverExecute,
                                                            .rw = ReadWritePermission::ReadWrite,
                                                            .access = Accessibility::Privileged,
                                                            .type = MemoryType::Device_nGnRnE};

static inline constexpr size_t reserved_contiguous_size = 100 * 1024 * 1024;  // 100 Mio.

static libk::LinearAllocator _mem_alloc;
static PageAllocList _page_alloc;

static ContiguousPageAllocator _contiguous_alloc;

static PhysicalPA _contiguous_start = 0;
static PhysicalPA _contiguous_stop = 0;

static MMUTable _tbl;

static VirtualPA _custom_pages = CUSTOM_PAGES_MEMORY;
static VirtualPA _buffer_pages = BUFFER_MEMORY;

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

void mark_as_used_range(PhysicalPA start, PhysicalPA end) {
  if (start >= end) {
    return;
  }

  if (end > _contiguous_start && start < _contiguous_stop) {
    // We have an intersection in contiguous allocator.
    const auto page_start = libk::max(start, _contiguous_start) - _contiguous_start;
    const auto page_stop = libk::min(end, _contiguous_stop) - _contiguous_start;

    _contiguous_alloc.mark_as_used(page_start, libk::div_round_up(page_stop - page_start + 1, PAGE_SIZE));
  }

  _page_alloc.mark_as_used_range(start, end);
}

bool memory_impl::init() {
  /* Set up the internal memory */
  {
    constexpr size_t linear_allocator_size = 1;  //<! Number of page reserved by the internal Linear Memory Allocator
    PhysicalPA linear_alloc_physical_memory;
    if (!allocate_pages(_lin_alloc, linear_allocator_size, &linear_alloc_physical_memory)) {
      libk::panic("[PageAllocList] Unable to set up internal memory.");
    }

    const uintptr_t linear_alloc_memory = (linear_alloc_physical_memory + KERNEL_BASE);
    _mem_alloc = libk::LinearAllocator(linear_alloc_memory, linear_allocator_size * PAGE_SIZE);
  }

  /* Set up the PageAllocList */
  _page_alloc = PageAllocList(_mem_alloc, reserved_contiguous_size);

  /* Set up the ContiguousPageAllocator */
  {
    _contiguous_start = _page_alloc.get_reserved_start();
    _contiguous_stop = _page_alloc.get_reserved_stop();

    if (_contiguous_start + reserved_contiguous_size > _contiguous_stop) {
      return false;  // reservation failed.
    }

    const size_t nb_pages = libk::div_round_down(_contiguous_stop - _contiguous_start + 1, PAGE_SIZE);
    const size_t nb_used_page = libk::div_round_up(PageAlloc::memory_needed(nb_pages), PAGE_SIZE);

    PhysicalPA contiguous_physical_memory;
    if (!allocate_pages(_lin_alloc, nb_used_page, &contiguous_physical_memory)) {
      return false;
    }

    const uintptr_t contiguous_memory = (contiguous_physical_memory + KERNEL_BASE);
    _contiguous_alloc = ContiguousPageAllocator(nb_pages, contiguous_memory);
  }

  // Protect the Stack, Kernel, DeviceTree, Page Allocator Memory, MMU Allocated Memory & Reserved Memory.
  {
    // Stack
    mark_as_used_range(PHYSICAL_STACK_TOP, PHYSICAL_STACK_TOP + KERNEL_STACK_SIZE);

    // Kernel
    mark_as_used_range(_init_data.kernel_start, _init_data.kernel_stop);

    // Allocated Pages
    mark_as_used_range(_lin_alloc->first_page, _lin_alloc->first_page + _lin_alloc->nb_allocated * PAGE_SIZE);

    // DeviceTree
    mark_as_used_range(_init_data.dtb_page_start, _init_data.dtb_page_end);

    // Reserved Memory
    {
      for (const auto& sec : KernelDT::get_reserved_sections()) {
        const PhysicalPA start = libk::align_to_previous(sec.address, PAGE_SIZE);
        const PhysicalPA end = libk::align_to_next(sec.address + sec.size, PAGE_SIZE);
        mark_as_used_range(start, end);
      }
    }

    // RamFs
    mark_as_used_range(RAM_FS_PHYSICAL_LOAD_ADDRESS, RAM_FS_PHYSICAL_LOAD_ADDRESS + RAM_FS_BYTE_SIZE);
  }

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

  return true;
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

PhysicalPA memory_impl::resolve_kernel_va(VirtualAddress va, bool read_only) {
  if (read_only) {
    asm volatile("at s1e1r, %x0" ::"r"(va));
  } else {
    asm volatile("at s1e1w, %x0" ::"r"(va));
  }
  uint64_t par_el1;
  asm volatile("mrs %x0, PAR_EL1" : "=r"(par_el1));

  if (par_el1 & 0x1) {
    libk::panic("[MemoryImpl] Unable to resolve physical address.");
  }

  return (par_el1 & libk::mask_bits(12, 47)) | (va & libk::mask_bits(1, 11));
}

bool memory_impl::allocate_buffer_pa(size_t nb_pages, PhysicalPA* buffer_start, PhysicalPA* buffer_end) {
  if (!_contiguous_alloc.fresh_pages(nb_pages, buffer_start, buffer_end)) {
    return false;
  }

  //  LOG_DEBUG("Allocated {:#x} -> {:#x}", *buffer_start, *buffer_end);
  //  LOG_DEBUG("Offset {:#x}", _contiguous_start);

  *buffer_start += _contiguous_start;
  *buffer_end += _contiguous_start;

  //  LOG_DEBUG("Target Range {:#x} -> {:#x}", *buffer_start, *buffer_end);

  return true;
}

void memory_impl::free_buffer_pa(PhysicalPA buffer_start, PhysicalPA buffer_end) {
  //  LOG_DEBUG("Freeing page {:#x} -> {:#x}", buffer_start, buffer_end);
  //  LOG_DEBUG("Offset {:#x}", _contiguous_start);

  const PhysicalPA target_start = buffer_start - _contiguous_start;
  const PhysicalPA target_end = buffer_end - _contiguous_start;

  //  LOG_DEBUG("Target {:#x} -> {:#x}", target_start, target_end);

  _contiguous_alloc.free_pages(target_start, target_end);
}

VirtualPA memory_impl::map_buffer(PhysicalPA buffer_start, PhysicalPA buffer_end) {
  VirtualPA buffer_va_start = _buffer_pages;
  VirtualPA buffer_va_end = _buffer_pages + buffer_end - buffer_start;

  //  LOG_DEBUG("Mapping buffer {:#x} -> {:#x} to {:#x} -> {:#x}", buffer_start, buffer_end, buffer_va_start,
  //            buffer_va_end);
  if (!map_range(&_tbl, buffer_va_start, buffer_va_end, buffer_start, buffer_memory_rw)) {
    libk::panic("Failed to map buffer memory in kernel space.");
  }
  //  LOG_DEBUG("Done.");

  _buffer_pages = buffer_va_end + PAGE_SIZE;

  return buffer_va_start;
}

void memory_impl::unmap_buffer(VirtualPA buffer_start, VirtualPA buffer_end) {
  //  LOG_DEBUG("Unmapping buffer range {:#x} -> {:#x}", buffer_start, buffer_end);

  if (!unmap_range(&_tbl, buffer_start, buffer_end)) {
    libk::panic("Failed to unmap buffer memory!");
  }
}
