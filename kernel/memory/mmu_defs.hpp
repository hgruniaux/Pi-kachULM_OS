#pragma once

#define KERNEL_BASE (0xffff000000000000)
#define PROCESS_BASE (0x0000000000000000)
#define PAGE_SIZE 4096

#ifndef __ASSEMBLER__
#include <cstdint>
// TODO : Add Doc

enum class Shareability : uint8_t {
  NonShareable = 0b00,
  OuterShareable = 0b10,
  InnerShareable = 0b11,
};

enum class ExecutionPermission : uint8_t {
  AllExecute = 0b00,         // {UXN, PXN} = {0, 0}
  ProcessExecute = 0b01,     // {UXN, PXN} = {0, 1}
  PrivilegedExecute = 0b10,  // {UXN, PXN} = {1, 0}
  NeverExecute = 0b11,       // {UXN, PXN} = {1, 1}
};

enum class ReadWritePermission : uint8_t {
  ReadWrite = 0b0,
  ReadOnly = 0b1,
};

enum class Accessibility : uint8_t {
  Privileged = 0b0,
  AllProcess = 0b1,
};

enum class MemoryType : uint8_t {
  Device_nGnRnE = 0,
  Device_nGRE = 1,
  Normal_NoCache = 2,
  Normal = 3,
};

struct PagesAttributes {
  const Shareability sh;
  const ExecutionPermission exec;
  const ReadWritePermission rw;
  const Accessibility access;
  const MemoryType type;
};

/** Used to specify the amount of memory mapped after a virtual address. */
enum class PageSize : uint8_t {
  Page_1Gio = 2,  //!< Maps 1Gio of memory at specified address
  Page_2Mio = 3,  //!< Maps 2Mio of memory at specified address
  Page_4Kio = 4,  //!< Maps 4Kio of memory at specified address
};

#endif
