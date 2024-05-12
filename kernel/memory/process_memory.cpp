#include "process_memory.hpp"
#include <libk/log.hpp>
#include "boot/mmu_utils.hpp"
#include "memory/kernel_internal_memory.hpp"

#include <algorithm>

uint8_t ProcessMemory::_new_asid = 1;

static inline PagesAttributes get_properties(bool read_only, bool executable) {
  return {.sh = Shareability::InnerShareable,
          .exec = executable ? ExecutionPermission::ProcessExecute : ExecutionPermission::NeverExecute,
          .rw = read_only ? ReadWritePermission::ReadOnly : ReadWritePermission::ReadWrite,
          .access = Accessibility::AllProcess,
          .type = MemoryType::Normal};
}

ProcessMemory::ProcessMemory()
    : _tbl(memory_impl::new_process_tbl(_new_asid++)),
      _heap(HeapManager::Kind::Process, &_tbl),
      _stack(PROCESS_STACK_PAGE_SIZE) {
  if (!_stack.is_status_okay()) {
    libk::panic("[ProcessMemory] Unable to allocate the process stack.");
  }

  map_chunk(_stack, PROCESS_STACK_PAGE_TOP, false, false);
}

ProcessMemory::~ProcessMemory() {
  free();
}

uint8_t ProcessMemory::get_asid() const {
  return _tbl.asid;
}

VirtualAddress ProcessMemory::get_stack_top() const {
  return PROCESS_STACK_PAGE_TOP;
}

VirtualAddress ProcessMemory::get_stack_bottom() const {
  return PROCESS_STACK_PAGE_BOTTOM;
}

VirtualPA ProcessMemory::change_heap_end(long byte_offset) {
  return _heap.change_heap_end(byte_offset);
}

VirtualPA ProcessMemory::get_heap_end() const {
  return _heap.get_heap_end();
}

size_t ProcessMemory::get_heap_byte_size() const {
  return _heap.get_heap_byte_size();
}

void ProcessMemory::activate() const {
  // First set TTBR0 to a null pointer, to invalidate all TBL with our ASID
  asm volatile("msr ttbr0_el1, xzr");
  reload_tlb(&_tbl);

  // Next we set the correct value for TTBR0
  asm volatile("msr ttbr0_el1, %x0" ::"r"(memory_impl::resolve_table_pgd(_tbl)));
}

void ProcessMemory::free() {
  // Free the heap
  _heap.free();

  // Free all mappings
  for (const auto chunk : _chunks) {
    unmap_chunk(chunk.start);  // <- chunk will be removed from the list by unmap
  }

  KASSERT(_chunks.is_empty());

  // Free MMU Table
  memory_impl::delete_process_tbl(_tbl);
}

bool ProcessMemory::map_chunk(MemoryChunk& chunk, const VirtualPA page_va, bool read_only, bool executable) {
  if (chunk._pas == nullptr) {
    return false;
  }

  const PagesAttributes attr = get_properties(read_only, executable);

  for (size_t page_id = 0; page_id < chunk._nb_pages; ++page_id) {
    const PhysicalPA page_pa = chunk._pas[page_id];

    if (!map_range(&_tbl, page_va + page_id * PAGE_SIZE, page_va + page_id * PAGE_SIZE, page_pa, attr)) {
      return false;
    }
  }

  _chunks.push_back({page_va, &chunk});
  chunk.register_mapping(this, page_va);

  return true;
}

void ProcessMemory::unmap_chunk(VirtualPA chunk_start_address) {
  auto it = std::find_if(_chunks.begin(), _chunks.end(), [&chunk_start_address](const MappedChunk& chunk) {
    return chunk.start == chunk_start_address;
  });

  if (it == std::end(_chunks)) {
    LOG_ERROR("There is no chunk mapped at {:#x} !", chunk_start_address);
    return;
  }

  if (!unmap_range(&_tbl, it->start, it->mem->end_address(it->start))) {
    LOG_ERROR("Failed to unmap from {:#x} tp {:#x} in process memory with asid: {}.", it->start,
              it->mem->end_address(it->start), get_asid());
  }

  _chunks.erase(it);
  it->mem->unregister_mapping(this);
}

bool ProcessMemory::change_chunk_attr(VirtualPA chunk_start_address, bool read_only, bool executable) {
  auto it = std::find_if(_chunks.begin(), _chunks.end(), [&chunk_start_address](const MappedChunk& chunk) {
    return chunk.start == chunk_start_address;
  });

  if (it == std::end(_chunks)) {
    LOG_ERROR("There is no chunk mapped at {:#x} !", chunk_start_address);
    return false;
  }

  return change_attr_range(&_tbl, it->start, it->mem->end_address(it->start), get_properties(read_only, executable));
}
