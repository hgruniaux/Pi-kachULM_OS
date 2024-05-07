#include "memory.hpp"
#include "kernel_dt.hpp"
#include "libk/linear_allocator.hpp"
#include "libk/log.hpp"
#include "mmu_utils.hpp"

struct PageAllocList {
  PhysicalPA section_start;
  PhysicalPA section_stop;
  PageAlloc alloc;
  PageAllocList* next;
};

MMUTable _tbl;
PageAllocList* _physical_alloc_list;
libk::LinearAllocator _memory_alloc;

bool alloc_page(PageAllocList* list, PhysicalPA* addr) {
  if (list == nullptr) {
    return false;
  } else if (list->alloc.fresh_page(addr)) {
    *addr += list->section_start;
    return true;
  } else {
    return alloc_page(list->next, addr);
  }
}

bool free_page(PageAllocList* list, PhysicalPA addr) {
  if (list == nullptr) {
    return false;
  } else if (list->section_start <= addr && addr < list->section_stop) {
    list->alloc.free_page(addr - list->section_start);
    return true;
  } else {
    return free_page(list->next, addr);
  }
}

void mark_as_used_range(PageAllocList* list, PhysicalPA start, PhysicalPA end) {
  if (list == nullptr || start >= end) {
    return;
  } else if (end > list->section_start && start < list->section_stop) {
    const auto page_start = libk::max(start, list->section_start);
    const auto page_stop = libk::min(end, list->section_stop);

    for (size_t page = page_start; page < page_stop; page += PAGE_SIZE) {
      list->alloc.mark_as_used(page - list->section_start);
    }
  }

  mark_as_used_range(list->next, start, end);
}

size_t _heap_size;

VirtualPA mmu_resolve_pa(void*, PhysicalPA page_address) {
  return page_address + KERNEL_BASE;
}

PhysicalPA mmu_resolve_va(void*, VirtualPA page_address) {
  return page_address - KERNEL_BASE;
}

VirtualPA mmu_alloc_page(void*) {
  PhysicalPA addr = -1;

  if (!alloc_page(_physical_alloc_list, &addr)) {
    libk::panic("[KernelMemory] Unable to allocate a page for the KERNEL MMU.");
  }

  const VirtualPA va = mmu_resolve_pa(nullptr, addr);
  zero_pages(va, 1);

  return va;
}

void mmu_free_page(void*, VirtualPA page_address) {
  const PhysicalPA addr = mmu_resolve_va(nullptr, page_address);

  if (!free_page(_physical_alloc_list, addr)) {
    LOG_CRITICAL("[KernelMemory] Unable to find the allocator used for this physical page.");
  }
}

bool setup_page_allocators() {
  // 1. Set up the list of page allocators
  Property prop;

  for (const auto& node : KernelDT::get_root().get_children()) {
    if (node.get_name().starts_with("memory@")) {
      // Found a memory node !

      if (!node.find_property("reg", &prop)) {
        return false;
      }

      size_t index = 0;

      while (index < prop.length) {
        uint64_t memory_chunk_start = 0;
        uint64_t memory_chunk_size = 0;

        if (!prop.get_variable_int(&index, &memory_chunk_start, _mem_prop->is_arm_mem_address_u64)) {
          return false;
        }

        if (!prop.get_variable_int(&index, &memory_chunk_size, _mem_prop->is_arm_mem_size_u64)) {
          return false;
        }

        const PhysicalPA page_start = memory_chunk_start;
        const PhysicalPA page_end = libk::align_to_next(memory_chunk_start + memory_chunk_size, PAGE_SIZE);

        const size_t nb_pages = (page_end - page_start) / PAGE_SIZE;

        const size_t nb_used_page = libk::div_round_up(PageAlloc::memory_needed(nb_pages), PAGE_SIZE);

        PhysicalPA page_alloc_physical_memory;
        if (!allocate_pages(_lin_alloc, nb_used_page, &page_alloc_physical_memory)) {
          return false;
        }

        auto* const page_alloc_memory = (void*)(page_alloc_physical_memory + KERNEL_BASE);

        auto* new_elm = _memory_alloc.new_class<PageAllocList>();
        new_elm->section_start = page_start;
        new_elm->section_stop = page_end;
        new_elm->alloc = PageAlloc(nb_pages, page_alloc_memory);
        new_elm->next = _physical_alloc_list;
        _physical_alloc_list = new_elm;
      }
    }
  }

  // 2. Protect the Stack, Kernel, DeviceTree, Page Allocator Memory & MMU Allocated Memory.

  // Stack
  mark_as_used_range(_physical_alloc_list, PHYSICAL_STACK_TOP, PHYSICAL_STACK_TOP + STACK_SIZE);

  // Kernel
  mark_as_used_range(_physical_alloc_list, _init_data.kernel_start, _init_data.kernel_stop);

  // Allocated Pages
  mark_as_used_range(_physical_alloc_list, _lin_alloc->first_page,
                     _lin_alloc->first_page + _lin_alloc->nb_allocated * PAGE_SIZE);

  // DeviceTree
  mark_as_used_range(_physical_alloc_list, _init_data.dtb_page_start, _init_data.dtb_page_end);

  return true;
}

bool KernelMemory::init() {
  /* Set up the Linear Allocator */
  {
    constexpr size_t linear_allocator_size = 1;  //<! Number of page reserved by the internal Linear Memory Allocator

    PhysicalPA linear_alloc_physical_memory;
    if (!allocate_pages(_lin_alloc, linear_allocator_size, &linear_alloc_physical_memory)) {
      return false;
    }
    auto* const linear_alloc_memory = (void*)(linear_alloc_physical_memory + KERNEL_BASE);

    _memory_alloc = libk::LinearAllocator(linear_alloc_memory, linear_allocator_size * PAGE_SIZE);
  }

  /* Set up the pages allocators */
  if (!setup_page_allocators()) {
    return false;
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

VirtualPA KernelMemory::get_heap_start() {
  return HEAP_MEMORY;
}

VirtualPA KernelMemory::get_heap_end() {
  return HEAP_MEMORY + _heap_size;
}

size_t KernelMemory::get_heap_byte_size() {
  return _heap_size;
}

static inline constexpr PagesAttributes heap_memory = {.sh = Shareability::InnerShareable,
                                                       .exec = ExecutionPermission::NeverExecute,
                                                       .rw = ReadWritePermission::ReadWrite,
                                                       .access = Accessibility::Privileged,
                                                       .type = MemoryType::Normal};

VirtualPA KernelMemory::change_heap_end(long byte_offset) {
  if (byte_offset > 0) {
    // Increase heap here.
    const size_t nb_pages_to_add = libk::div_round_up(byte_offset, PAGE_SIZE);  // Round up here

    const VirtualPA current_heap_end = get_heap_end();

    for (size_t i = 0; i < nb_pages_to_add; ++i) {
      PhysicalPA new_heap_pa_page = -1;

      if (!alloc_page(_physical_alloc_list, &new_heap_pa_page)) {
        return 0;
      }
      const VirtualPA new_heap_va_page = current_heap_end + i * PAGE_SIZE;

      if (!map_range(&_tbl, new_heap_va_page, new_heap_va_page, new_heap_pa_page, heap_memory)) {
        return 0;
      }
    }

    _heap_size += nb_pages_to_add * PAGE_SIZE;
  } else if (byte_offset < 0) {
    // Decrease heap here.
    const size_t nb_pages_to_remove = libk::div_round_down(-byte_offset, PAGE_SIZE);  // Round down here

    const VirtualPA current_heap_end = get_heap_end();

    if (!unmap_range(&_tbl, current_heap_end - nb_pages_to_remove * PAGE_SIZE + PAGE_SIZE, current_heap_end)) {
      return 0;
    }

    _heap_size -= nb_pages_to_remove * PAGE_SIZE;
  }

  return get_heap_end();
}

size_t KernelMemory::get_memory_overhead() {
  return _lin_alloc->nb_allocated * PAGE_SIZE;
}
