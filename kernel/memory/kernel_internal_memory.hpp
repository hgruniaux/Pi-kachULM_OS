#pragma once

#include "page_alloc_list.hpp"

namespace memory_impl {

void init();

PageAllocList* get_kernel_alloc();
MMUTable* get_kernel_tbl();

MMUTable new_process_tbl(uint8_t asid);
void delete_process_tbl(MMUTable& tbl);
PhysicalPA resolve_table_pgd(const MMUTable& tbl);

VirtualPA allocate_pages_section(const size_t nb_pages, PhysicalPA* pages_ptr);
void free_section(size_t nb_pages, VirtualPA kernel_va, PhysicalPA* pages_ptr);
};  // namespace memory_impl
