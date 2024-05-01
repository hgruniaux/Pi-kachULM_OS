#pragma once

#define PAGE_SIZE (4096)            // 4096 bytes
#define STACK_SIZE (2 * PAGE_SIZE)  // 2 * 4096 bytes

#define KERNEL_AARCH64_LOAD_ADDRESS (0x80000)
#define KERNEL_BASE (0xffff000000000000)
#define PROCESS_BASE (0x0000000000000000)

#define NORMAL_MEMORY KERNEL_BASE
#define VC_MEMORY (KERNEL_BASE + 0x0000100000000000)
#define DEVICE_MEMORY (KERNEL_BASE + 0x0000200000000000)
#define STACK_MEMORY (KERNEL_BASE + 0x0000f00000000000)

#define KERNEL_STACK_PAGE_TOP(core) (STACK_MEMORY | (core << 36))
#define KERNEL_STACK_PAGE_BOTTOM(core) (KERNEL_STACK_PAGE_TOP(core) + STACK_SIZE - PAGE_SIZE)

#define DEFAULT_CORE 0

#ifndef __ASSEMBLER__
#include <cstdint>

enum class Shareability : uint8_t {
  NonShareable = 0b00,    //<! Memory not shared at all
  OuterShareable = 0b10,  //<! Memory shared across CPU cores
  InnerShareable = 0b11,  //<! Memory shared across multiples peripheral
};

enum class ExecutionPermission : uint8_t {
  // {UXN, PXN} = {0, 0}
  // {UXN, PXN} = {0, 1}
  // {UXN, PXN} = {1, 0}
  // {UXN, PXN} = {1, 1}

  AllExecute = 0b00,         //<! Kernel and Process can execute the memory
  ProcessExecute = 0b01,     //<! Only the Process can execute the memory
  PrivilegedExecute = 0b10,  //<! Only the Kernel can execute the memory
  NeverExecute = 0b11,       //<! No one can execute the memory
};

enum class ReadWritePermission : uint8_t {
  ReadWrite = 0b0,  //<! Read and write allowed on the memory
  ReadOnly = 0b1,   //<! Only read allowed on the memory
};

enum class Accessibility : uint8_t {
  Privileged = 0b0,  //<! Read and write (if allowed) only allowed by the Kernel
  AllProcess = 0b1,  //<! Read and write (if allowed) allowed by everyone (Kernel included)
};

enum class MemoryType : uint8_t {
  Normal = 0,          //<! Normal memory cached
  Device_nGnRnE = 1,   //<! Device memory without any Gathering, Reordering nor Elimination
  Device_nGRE = 2,     //<! Device memory without any Gathering but Reordering and Elimination allowed
  Normal_NoCache = 3,  //<! Normal memory not cached
};

struct PagesAttributes {
  Shareability sh;
  ExecutionPermission exec;
  ReadWritePermission rw;
  Accessibility access;
  MemoryType type;
};

using PhysicalPA = uintptr_t;
using VirtualPA = uintptr_t;

#endif
