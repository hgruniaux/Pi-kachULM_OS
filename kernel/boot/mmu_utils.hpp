#pragma once

#define PAGE_SIZE (4096)            // 4096 bytes
#define STACK_SIZE (2 * PAGE_SIZE)  // 2 * 4096 bytes

#define KERNEL_BASE (0xffff000000000000)
#define PROCESS_BASE (0x0000000000000000)
#define TTBR_MASK (0xffff000000000000)

#define PHYSICAL_KERNEL_LOAD_ADDRESS (0x80000)
#define PHYSICAL_STACK_TOP (0)

#define NORMAL_MEMORY KERNEL_BASE
#define VC_MEMORY (KERNEL_BASE + 0x0000100000000000)
#define DEVICE_MEMORY (KERNEL_BASE + 0x0000200000000000)
#define SPECIAL_MEMORY (KERNEL_BASE + 0x0000300000000000)
#define HEAP_MEMORY (KERNEL_BASE + 0x0000400000000000)
#define STACK_MEMORY (KERNEL_BASE + 0x0000f00000000000)

#define KERNEL_STACK_PAGE_TOP(core) ((STACK_MEMORY + PHYSICAL_STACK_TOP) | (core << 36))
#define KERNEL_STACK_PAGE_BOTTOM(core) (KERNEL_STACK_PAGE_TOP(core) + STACK_SIZE - PAGE_SIZE)

#define DEFAULT_CORE 0

#ifndef __ASSEMBLER__
#include <cstddef>
#include <cstdint>

using PhysicalPA = uintptr_t;
using VirtualPA = uintptr_t;

struct LinearPageAllocator {
  PhysicalPA first_page;
  PhysicalPA upper_bound;
  size_t nb_allocated;
};

struct DeviceMemoryProperties {
  bool is_arm_mem_address_u64;
  bool is_arm_mem_size_u64;

  bool is_soc_mem_address_u64;
  bool is_soc_mem_size_u64;
};

struct MMUInitData {
  VirtualPA pgd;

  LinearPageAllocator lin_alloc;
  DeviceMemoryProperties mem_prop;

  PhysicalPA dtb_page_start;
  PhysicalPA dtb_page_end;

  PhysicalPA kernel_start;
  PhysicalPA kernel_stop;
};

extern MMUInitData _init_data asm("_init_data");

inline constexpr auto* _lin_alloc = &_init_data.lin_alloc;
inline constexpr auto* _mem_prop = &_init_data.mem_prop;

bool allocate_pages(LinearPageAllocator* alloc, size_t nb_pages, PhysicalPA * page);
void zero_pages(VirtualPA pages, size_t nb_pages);
#endif
