#pragma once

#include "page_alloc_list.hpp"

namespace memory_impl {

[[nodiscard]] bool init();

PageAllocList* get_kernel_alloc();
MMUTable* get_kernel_tbl();

MMUTable new_process_tbl(uint8_t asid);
void delete_process_tbl(MMUTable& tbl);
PhysicalPA resolve_table_pgd(const MMUTable& tbl);

VirtualPA allocate_pages_section(size_t nb_pages, PhysicalPA* pages_ptr);
void free_section(size_t nb_pages, VirtualPA kernel_va, PhysicalPA* pages_ptr);

bool allocate_buffer_pa(size_t nb_pages, PhysicalPA* buffer_start, PhysicalPA* buffer_end);
void free_buffer_pa(PhysicalPA buffer_start, PhysicalPA buffer_end);
VirtualPA map_buffer(PhysicalPA buffer_start, PhysicalPA buffer_end);
void unmap_buffer(VirtualPA buffer_start, VirtualPA buffer_end);

PhysicalPA resolve_kernel_va(VirtualAddress va, bool read_only);

};  // namespace memory_impl
