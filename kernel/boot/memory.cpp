#include "memory.hpp"
#include "mmu_defs.hpp"

// uintptr_t Memory::get_pgd() {
//   return physical_pgd;
// }
// uintptr_t Memory::get_first_stage1_p() {
//   return first_stage1_page;
// }
// uintptr_t Memory::get_last_stage1_p() {
//   return last_stage1_page;
// }

VirtualPA alloc_page(void* handle);
void free_page(void* handle, VirtualPA page_address);

VirtualPA resolve_pa(void*, PhysicalPA page_address) {
  return page_address + KERNEL_BASE;
}

PhysicalPA resolve_va(void*, VirtualPA page_address) {
  KASSERT((page_address & TTBR_MASK) == KERNEL_BASE);
  KASSERT(((page_address >> 44) & 0xf) == 0);
  return page_address - KERNEL_BASE;
}

//KernelMemory::KernelMemory(const DeviceTree& dt, PageAlloc alloc, MMUTable tbl)
//    : _dt(dt), _alloc(alloc), _kernel_tbl(tbl) {}
//
//KernelMemory::Kind KernelMemory::get_address_kind(uintptr_t address) const {
//  if ((address & TTBR_MASK) == PROCESS_BASE) {
//    return Kind::Process;
//  }
//
//  size_t last_byte = (address >> 44) & 0xf;
//
//  switch (last_byte) {
//    case 0x0:
//      return Kind::Reserved;
//    case 0x1:
//      return Kind::VC;
//    case 0x2:
//      return Kind::Device;
//    case 0x3:
//      return Kind::Special;
//    case 0xf:
//      return Kind::Reserved;
//    default:
//      return Kind::Heap;
//  }
//}
//
//KernelMemory KernelMemory::from_device_tree(const DeviceTree& dt) {
//  // Build a MMUTable
//  // Required :
//  // - A page Allocator
//  // - TBL Pointer
//
//  MMUTable tbl = {
//      .kind = MMUTable::Kind::Kernel,
//      .pgd = pgd,
//      .asid = 0,
//      .handle = (void*)&handle,
//      .alloc = &alloc_page,
//      .free = &free_page,  // We don't free anything here !
//      .resolve_pa = &resolve_pa,
//      .resolve_va = &resolve_va,
//  };
//
//  return KernelMemory(DeviceTree(), PageAlloc(), tbl);
//}
