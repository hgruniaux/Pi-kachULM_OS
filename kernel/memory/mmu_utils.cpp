#include "mmu_utils.hpp"

uint64_t encode_new_entry(libk::PhysicalPA pa, PagesAttributes attr, PageSize amount, bool set_nG) {
  const uint64_t upper_attr = (uint64_t)attr.exec << 53;

  const uint64_t lower_attr = ((uint64_t)attr.type << 2) | ((uint64_t)attr.access << 6) | ((uint64_t)attr.rw << 7) |
                              ((uint64_t)attr.sh << 8) |
                              ((uint64_t)1 << 10)  // Set the Access flag to disable access interrupts
                              | ((uint64_t)(set_nG ? 1 : 0) << 11);

  const uint64_t marker = amount == PageSize::Page_4Kio ? PAGE_MARKER : BLOCK_MARKER;

  const uint64_t new_entry = upper_attr | (pa & libk::mask_bits(12, 47)) | lower_attr | marker;

  return new_entry;
}
