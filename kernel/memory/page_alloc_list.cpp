#include "page_alloc_list.hpp"
#include <libk/utils.hpp>
#include "boot/mmu_utils.hpp"
#include "hardware/kernel_dt.hpp"

bool PageAllocList::parse_memory_reg(libk::LinearAllocator& mem_alloc,
                                     Property prop,
                                     PageAllocList* list,
                                     size_t contiguous_res_bytes) {
  bool reserved_section = false;
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

    // Memory Chunk start
    const PhysicalPA chunk_start = memory_chunk_start;

    // Memory Chunk end
    const PhysicalPA chunk_end = libk::align_to_previous(memory_chunk_start + memory_chunk_size, PAGE_SIZE);

    // Number of page of the Chunk.
    const size_t chunk_nb_pages = (chunk_end - chunk_start) / PAGE_SIZE;

    // Number of asked reserved pages.
    const size_t nb_reserved_pages = libk::div_round_up(contiguous_res_bytes, PAGE_SIZE);

    const PhysicalPA page_start = chunk_start;  // Starting page of the allocator.
    PhysicalPA page_end;                        // Stopping page of the allocator.

    if (!reserved_section && chunk_nb_pages > 2 * nb_reserved_pages) {
      page_end = chunk_end - contiguous_res_bytes - PAGE_SIZE;

      contiguous_res_start = page_end;
      contiguous_res_stop = chunk_end;
      reserved_section = true;
    } else {
      page_end = chunk_end;
    }

    const size_t nb_pages = libk::div_round_down(page_end - page_start + 1, PAGE_SIZE);
    const size_t nb_used_page = libk::div_round_up(PageAlloc::memory_needed(nb_pages), PAGE_SIZE);

    PhysicalPA page_alloc_physical_memory;
    if (!allocate_pages(_lin_alloc, nb_used_page, &page_alloc_physical_memory)) {
      return false;
    }

    const uintptr_t page_alloc_memory = (page_alloc_physical_memory + KERNEL_BASE);
    list->add_allocator(mem_alloc, page_start, page_end, nb_pages, page_alloc_memory);
  }

  return reserved_section;
}

PageAllocList::PageAllocList(libk::LinearAllocator& mem_alloc, size_t contiguous_res_bytes)
    : contiguous_res_start(0), contiguous_res_stop(0), _list_beg(nullptr), _list_end(nullptr) {
  // Set up the list of page allocators
  Property prop;

  bool reserved_ok = false;
  for (const auto& node : KernelDT::get_root().get_children()) {
    if (node.get_name().starts_with("memory@")) {
      // Found a memory node !

      if (!node.find_property("reg", &prop)) {
        continue;
      }

      reserved_ok = parse_memory_reg(mem_alloc, prop, this, reserved_ok ? 0 : contiguous_res_bytes);
    }
  }
}

bool PageAllocList::fresh_page(PhysicalPA* addr) {
  AllocList* cur = _list_beg;

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
  AllocList* cur = _list_beg;

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

  AllocList* cur = _list_beg;

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

void PageAllocList::add_allocator(libk::LinearAllocator& mem_alloc,
                                  PhysicalPA page_start,
                                  PhysicalPA page_end,
                                  size_t nb_pages,
                                  uintptr_t array) {

  auto* new_elm = mem_alloc.new_class<AllocList>();
  new_elm->section_start = page_start;
  new_elm->section_stop = page_end;
  new_elm->alloc = PageAlloc(nb_pages, array);
  new_elm->next = nullptr;

  if (_list_end != nullptr){
    _list_end->next = new_elm;
  }

  if (_list_beg == nullptr) {
    _list_beg = new_elm;
  }

  _list_end = new_elm;
}
