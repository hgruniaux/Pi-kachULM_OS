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

ProcessMemory::ProcessMemory(size_t minimum_stack_byte_size)
    : _tbl(memory_impl::new_process_tbl(_new_asid++)),
      _heap(HeapManager::Kind::Process, &_tbl),
      _stack(libk::div_round_up(minimum_stack_byte_size, PAGE_SIZE)) {
  if (!_stack.is_status_okay()) {
    libk::panic("[ProcessMemory] Unable to allocate the process stack.");
  }

  map_chunk(_stack, PROCESS_STACK_BASE, false, false);
}

ProcessMemory::~ProcessMemory() {
  free();
}

uint8_t ProcessMemory::get_asid() const {
  return _tbl.asid;
}

VirtualAddress ProcessMemory::get_stack_end() const {
  return PROCESS_STACK_BASE;
}

VirtualAddress ProcessMemory::get_stack_start() const {
  return PROCESS_STACK_BASE + _stack.get_byte_size();
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

void ProcessMemory::deactivate() {
  asm volatile("msr ttbr0_el1, xzr");
  asm volatile("tlbi vmalle1is");
  asm volatile("dsb sy; isb" ::: "memory");
}

void ProcessMemory::free() {
  // Free the heap
  _heap.free();

  // Free all mappings
  for (const auto chunk : _sec) {
    unmap_memory(chunk.start);  // <- chunk will be removed from the list by unmap
  }

  KASSERT(_sec.is_empty());

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

  _sec.emplace_back(page_va, false, &chunk);
  chunk.register_mapping(this, page_va);

  return true;
}

bool ProcessMemory::map_buffer(Buffer& chunk, VirtualPA page_va, bool read_only, bool executable) {
  if (chunk.buffer_pa_start == 0) {
    return false;
  }

  const PagesAttributes attr = get_properties(read_only, executable);

  const PhysicalPA buffer_va_start = page_va;
  const PhysicalPA buffer_va_end = buffer_va_start + chunk.buffer_pa_end - chunk.buffer_pa_start;

  if (!map_range(&_tbl, buffer_va_start, buffer_va_end, chunk.buffer_pa_start, attr)) {
    return false;
  }

  _sec.emplace_back(buffer_va_start, true, &chunk);
  chunk.register_mapping(this, page_va);

  return true;
}

void ProcessMemory::unmap_memory(VirtualPA start_address) {
  auto it = _sec.begin();
  for (; it != std::end(_sec); ++it) {
    if (it->start == start_address) {
      break;
    }
  }

  if (it == std::end(_sec)) {
    LOG_ERROR("There is no memory mapped at {:#x} !", start_address);
    return;
  }

  VirtualAddress end_address;
  if (it->is_buffer) {
    end_address = ((Buffer*)it->mem)->end_address(it->start);
  } else {
    end_address = ((MemoryChunk*)it->mem)->end_address(it->start);
  }

  if (!unmap_range(&_tbl, it->start, end_address)) {
    LOG_ERROR("Failed to unmap from {:#x} tp {:#x} in process memory with asid: {}.", it->start, end_address,
              get_asid());
  }

  if (it->is_buffer) {
    ((Buffer*)it->mem)->unregister_mapping(this);
  } else {
    ((MemoryChunk*)it->mem)->unregister_mapping(this);
  }

  _sec.erase(it);
}

bool ProcessMemory::change_memory_attr(VirtualPA start_address, bool read_only, bool executable) {
  auto it = _sec.begin();
  for (; it != std::end(_sec); ++it) {
    if (it->start == start_address) {
      break;
    }
  }

  if (it == std::end(_sec)) {
    LOG_ERROR("There is no memory mapped at {:#x} !", start_address);
    return false;
  }

  VirtualAddress end_address;
  if (it->is_buffer) {
    end_address = ((Buffer*)it->mem)->end_address(it->start);
  } else {
    end_address = ((MemoryChunk*)it->mem)->end_address(it->start);
  }

  return change_attr_range(&_tbl, it->start, end_address, get_properties(read_only, executable));
}

bool ProcessMemory::is_read_only(VirtualPA va) const {
  PagesAttributes attr;

  if (!get_attr(&_tbl, va, &attr)) {
    LOG_ERROR("There is no memory mapped at {:#x} !", va);
    return true;
  }

  return attr.rw == ReadWritePermission::ReadOnly;
}

bool ProcessMemory::is_executable(VirtualPA va) const {
  PagesAttributes attr;

  if (!get_attr(&_tbl, va, &attr)) {
    LOG_ERROR("There is no memory mapped at {:#x} !", va);
    return true;
  }

  return attr.exec == ExecutionPermission::ProcessExecute;
}
