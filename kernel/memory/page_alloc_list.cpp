#include "page_alloc_list.hpp"
#include "boot/kernel_dt.hpp"
#include "libk/log.hpp"
#include "libk/utils.hpp"

void parse_memory_reg(Property prop, PageAllocList* list) {
  size_t index = 0;

  while (index < prop.length) {
    uint64_t memory_chunk_start = 0;
    uint64_t memory_chunk_size = 0;

    if (!prop.get_variable_int(&index, &memory_chunk_start, _mem_prop->is_arm_mem_address_u64)) {
      return;
    }

    if (!prop.get_variable_int(&index, &memory_chunk_size, _mem_prop->is_arm_mem_size_u64)) {
      return;
    }

    const PhysicalPA page_start = memory_chunk_start;
    const PhysicalPA page_end = libk::align_to_next(memory_chunk_start + memory_chunk_size, PAGE_SIZE);

    const size_t nb_pages = (page_end - page_start) / PAGE_SIZE;

    const size_t nb_used_page = libk::div_round_up(PageAlloc::memory_needed(nb_pages), PAGE_SIZE);

    PhysicalPA page_alloc_physical_memory;
    if (!allocate_pages(_lin_alloc, nb_used_page, &page_alloc_physical_memory)) {
      return;
    }

    const uintptr_t page_alloc_memory = (page_alloc_physical_memory + KERNEL_BASE);
    list->add_allocator(page_start, page_end, nb_pages, page_alloc_memory);
  }
}

PageAllocList::PageAllocList(size_t internal_memory_size) : _list(nullptr) {
  // 1. Set up the internal memory

  PhysicalPA linear_alloc_physical_memory;
  if (!allocate_pages(_lin_alloc, internal_memory_size, &linear_alloc_physical_memory)) {
    libk::panic("[PageAllocList] Unable to set up internal memory.");
  }

  const uintptr_t linear_alloc_memory = (linear_alloc_physical_memory + KERNEL_BASE);
  _mem_alloc = libk::LinearAllocator(linear_alloc_memory, internal_memory_size * PAGE_SIZE);

  // 2. Set up the list of page allocators
  Property prop;

  for (const auto& node : KernelDT::get_root().get_children()) {
    if (node.get_name().starts_with("memory@")) {
      // Found a memory node !

      if (!node.find_property("reg", &prop)) {
        continue;
      }

      parse_memory_reg(prop, this);
    }
  }

  // 3. Protect the Stack, Kernel, DeviceTree, Page Allocator Memory & MMU Allocated Memory.

  // Stack
  mark_as_used_range(PHYSICAL_STACK_TOP, PHYSICAL_STACK_TOP + STACK_SIZE);

  // Kernel
  mark_as_used_range(_init_data.kernel_start, _init_data.kernel_stop);

  // Allocated Pages
  mark_as_used_range(_lin_alloc->first_page, _lin_alloc->first_page + _lin_alloc->nb_allocated * PAGE_SIZE);

  // DeviceTree
  mark_as_used_range(_init_data.dtb_page_start, _init_data.dtb_page_end);
}

bool PageAllocList::fresh_page(PhysicalPA* addr) {
  AllocList* cur = _list;

  while (cur != nullptr) {
    if (cur->alloc.fresh_page(addr)) {
      *addr += cur->section_start;
      return true;
    }

    cur = cur->next;
  }

  return false;
}

void PageAllocList::free_page(PhysicalPA addr) {
  AllocList* cur = _list;

  while (cur != nullptr) {
    if (cur->section_start <= addr && addr < cur->section_stop) {
      cur->alloc.free_page(addr - cur->section_start);
    }

    cur = cur->next;
  }
}

void PageAllocList::mark_as_used_range(PhysicalPA start, PhysicalPA end) {
  if (start >= end) {
    return;
  }

  AllocList* cur = _list;

  while (cur != nullptr) {
    if (end > cur->section_start && start < cur->section_stop) {
      // We have an intersection
      const auto page_start = libk::max(start, cur->section_start);
      const auto page_stop = libk::min(end, cur->section_stop);

      for (size_t page = page_start; page < page_stop; page += PAGE_SIZE) {
        cur->alloc.mark_as_used(page - cur->section_start);
      }
    }

    cur = cur->next;
  }
}

void PageAllocList::add_allocator(PhysicalPA page_start, PhysicalPA page_end, size_t nb_pages, uintptr_t array) {
  auto* new_elm = _mem_alloc.new_class<AllocList>();
  new_elm->section_start = page_start;
  new_elm->section_stop = page_end;
  new_elm->alloc = PageAlloc(nb_pages, array);
  new_elm->next = _list;
  _list = new_elm;
}
