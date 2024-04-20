#include "mmu_utilss.hpp"

bool alloc_page(LinearPhysicalAlloc* alloc, libk::VirtualPA* page) {
  if (alloc->first_page + PAGE_SIZE * (alloc->nb_allocated + 1) < alloc->upper_bound) {
    *page = libk::VirtualPA(alloc->first_page + PAGE_SIZE * alloc->nb_allocated++);
    return true;
  }

  return false;
}

bool map_range_in_table(libk::VirtualPA va_start,
                        libk::VirtualPA va_end,
                        libk::PhysicalPA pa_start,
                        PagesAttributes attr,
                        uint64_t* table,
                        size_t table_level,
                        libk::VirtualPA table_first_page_va,
                        LinearPhysicalAlloc* alloc) {
  if (va_start > va_end) {
    return true;
  }

  const auto table_last_page_va = table_last_page(table_first_page_va, table_level);

  if ((va_end < table_first_page_va) || (va_start > table_last_page_va)) {
    // Input address space does not intersect with this table.
    return true;
  }

  const auto inter_start_va = libk::max(va_start, table_first_page_va);
  const auto inter_end_va = libk::min(va_end, table_last_page_va);

  // We need to check all entries of the table within [start_table_index:end_table_index].
  const size_t inter_start_index = get_index_in_table(inter_start_va, table_level);
  const size_t inter_end_index = get_index_in_table(inter_end_va, table_level);

  for (size_t index = inter_start_index; index <= inter_end_index; ++index) {
    const uint64_t entry = table[index];
    const auto entry_va_start = get_entry_va_from_table_index(table_first_page_va, table_level, index);

    switch (get_entry_kind(entry, table_level)) {
      case EntryKind::Invalid: {
        if (table_level == 4) {
          // Map with a page
          const size_t offset = entry_va_start - va_start;
          const auto entry_pa = pa_start + offset;
          const uint64_t new_entry = encode_new_entry(entry_pa, attr, PageSize::Page_4Kio, false);
          table[index] = new_entry;
        } else {
          // table_level < 4
          const auto entry_va_end = table_last_page(entry_va_start, table_level + 1);

          if (va_start <= entry_va_start && entry_va_end <= va_end && table_level >= 2) {
            // Map with a block
            const size_t offset = entry_va_start - va_start;
            const auto entry_pa = pa_start + offset;
            const uint64_t new_entry = encode_new_entry(entry_pa, attr, (PageSize)table_level, false);
            table[index] = new_entry;
          } else {
            // Map with a table

            // We need a new page, let's allocate it !
            libk::VirtualPA new_table_va;
            if (!alloc_page(alloc, &new_table_va)) {
              // Unable to allocate a fresh table :/
              return false;
            }

            // HERE, and ONLY HERE, VirtualAddress = PhysicalAddress
            const auto new_table_pa = libk::PhysicalPA(new_table_va);

            // And fill the current table with the new entry
            table[index] = new_table_pa | TABLE_MARKER;

            auto* new_table = (uint64_t*)new_table_va;
            const size_t new_level = table_level + 1;

            // And recursively map what's needed.
            if (!map_range_in_table(va_start, va_end, pa_start, attr, new_table, new_level, entry_va_start, alloc)) {
              return false;
            }
          }
        }
        break;
      }

      case EntryKind::Block:
      case EntryKind::Page: {
        return false;
      }

      case EntryKind::Table: {
        libk::PhysicalPA sub_table_pa = get_table_pa_from_entry(entry);

        // HERE, and ONLY HERE, VirtualAddress = PhysicalAddress
        auto* sub_table = (uint64_t*)sub_table_pa;

        // And recursively map what's needed.
        if (!map_range_in_table(va_start, va_end, pa_start, attr, sub_table, table_level + 1, entry_va_start, alloc)) {
          return false;
        }
      }
    }
  }

  return true;
}

bool change_properties_for_range(libk::VirtualPA va_start,
                                 libk::VirtualPA va_end,
                                 PagesAttributes attr,
                                 uint64_t* table,
                                 size_t table_level,
                                 libk::VirtualPA table_first_page_va,
                                 LinearPhysicalAlloc* alloc) {
  if (va_start > va_end) {
    return true;
  }

  const auto table_last_page_va = table_last_page(table_first_page_va, table_level);

  if ((va_end < table_first_page_va) || (va_start > table_last_page_va)) {
    // Input address space does not intersect with this table.
    return true;
  }

  const auto inter_start_va = libk::max(va_start, table_first_page_va);
  const auto inter_end_va = libk::min(va_end, table_last_page_va);

  // We need to check all entries of the table within [start_table_index:end_table_index].
  const size_t inter_start_index = get_index_in_table(inter_start_va, table_level);
  const size_t inter_end_index = get_index_in_table(inter_end_va, table_level);

  for (size_t index = inter_start_index; index <= inter_end_index; ++index) {
    const uint64_t entry = table[index];
    const auto entry_va_start = get_entry_va_from_table_index(table_first_page_va, table_level, index);

    switch (get_entry_kind(entry, table_level)) {
      case EntryKind::Invalid: {
        // Nothing to change here !
        continue;
      }

      case EntryKind::Block: {
        // If split needed:
        // - Decode Attrs and PA
        // - We split in 3 parts : BeforeChange, Change, AfterChange
        // - We call map_range for each part
        break;
      }
      case EntryKind::Page: {
        // Trivial case in theory
        break;
      }

      case EntryKind::Table: {
        libk::PhysicalPA sub_table_pa = get_table_pa_from_entry(entry);

        // HERE, and ONLY HERE, VirtualAddress = PhysicalAddress
        auto* sub_table = (uint64_t*)sub_table_pa;

        // And recursively map what's needed.
        if (!change_properties_for_range(va_start, va_end, attr, sub_table, table_level + 1, entry_va_start, alloc)) {
          return false;
        }
      }
    }
  }

  return false;
}
