#pragma once

#include <cstddef>
#include <cstdint>

struct Registers {
  uint64_t x0;
  uint64_t x1;
  uint64_t x2;
  uint64_t x3;
  uint64_t x4;
  uint64_t x5;
  uint64_t x6;
  uint64_t x7;
  uint64_t x8;
  uint64_t x9;
  uint64_t x10;
  uint64_t x11;
  uint64_t x12;
  uint64_t x13;
  uint64_t x14;
  uint64_t x15;
  uint64_t x16;
  uint64_t x17;
  uint64_t x18;
  uint64_t x19;
  uint64_t x20;
  uint64_t x21;
  uint64_t x22;
  uint64_t x23;
  uint64_t x24;
  uint64_t x25;
  uint64_t x26;
  uint64_t x27;
  uint64_t x28;
  uint64_t x29;
  uint64_t x30;  // the link pointer (store the return address)
  uint64_t xzr;  // always zero

  uint64_t elr;   // Exception Link Register
  uint64_t spsr;  // Saved Program Status Register

  // FIXME: These two registers should not be stored here but in a special interrupt registers struct.
  // The following two registers are only used in case of exceptions.
  // In other cases, they are set to zero.
  uint64_t esr;
  uint64_t far;
};  // struct Registers

using SyscallCallback = void (*)(Registers&);

/**
 * The SyscallTable class contains all the information the kernel needs to
 * decode a system call from a user-space application and dispatch it to one
 * of the kernel's internal functions.
 *
 * In other words, this class provides a mapping of system calls to kernel functions.
 *
 * What's important is that each process can have a different table.
 * And therefore have a different interface with the kernel. In theory, it's
 * possible for processes to use both Linux and Windows binary interfaces
 * within the same system.
 *
 * Example:
 * ```c++
 * SyscallTable* table = new SyscallTable;
 *
 * #define SYS_EXIT 152
 * table->register_syscall(SYS_EXIT, [](Registers& regs) {
 *    // The first argument of the syscall is in x0 by convention.
 *    const auto exit_code = regs.x0;
 *
 *    // kill the current process...
 *
 *    // The return value of the syscall is stored in x0 by convention.
 *    regs.x0 = 0; // no error
 * });
 *
 * // etc.
 * ```
 */
class SyscallTable {
 public:
  using id_t = uint32_t;

  SyscallTable();

  // No copy
  SyscallTable(const SyscallTable&) = delete;
  SyscallTable(SyscallTable&&) = delete;

  // No move
  SyscallTable& operator=(const SyscallTable&) = delete;
  SyscallTable& operator=(SyscallTable&&) = delete;

  /**
   * Maximum value (excluded) allowed for a system call identifier.
   */
  static constexpr size_t MAX_ID = 512;

  /**
   * Calls the kernel callback for the given syscall @a id.
   * This effectively dispatch the call using the previously registered mapping.
   */
  void call_syscall(id_t id, Registers& registers);

  /**
   * Provides a callback to be used when an unknown system call is called.
   * If @a callback is null, a default placeholder callback is used instead.
   */
  void set_unknown_callback(SyscallCallback callback);

  /**
   * Registers a syscall handler for the given syscall @a id.
   *
   * Each time a userspace application will do a system call with the
   * identifier @a id, @a callback will be called to handle the syscall.
   * It is the responsibility of the callback to correctly set the
   * registers values to be returned to the user.
   *
   * If the requested syscall has an already registered handler, the
   * function returns false and nothing more is done (@a callback is ignored).
   * Otherwise, the function returns true and @a callback is correctly registered.
   */
  bool register_syscall(id_t id, SyscallCallback callback);

  /**
   * Unregisters a previously registered syscall handler for @a id.
   *
   * If no handler was registered for @a id before, the function does nothing.
   */
  void unregister_syscall(id_t id);

 private:
  SyscallCallback m_unknown_callback = nullptr;
  SyscallCallback m_entries[MAX_ID];
};  // class SyscallTable
