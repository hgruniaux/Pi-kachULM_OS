#pragma once

#define KERNEL_BASE (0xffff000000000000)
#define PROCESS_BASE (0x0000000000000000)
#define PAGE_SIZE 4096

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
  Device_nGnRnE = 0,   //<! Device memory without any Gathering, Reordering nor Elimination
  Device_nGRE = 1,     //<! Device memory without any Gathering but Reordering and Elimination allowed
  Normal_NoCache = 2,  //<! Normal memory not cached
  Normal = 3,          //<! Normal memory cached
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
