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
  // Registers x19-x28 are callee-saved so no need to save them.
  uint64_t x30;  // the link pointer (store the return address)

  // The following two registers are only used in case of exceptions.
  // In other cases, they are set to zero.
  uint64_t esr;
  uint64_t far;
};  // struct Registers

using SyscallCallback = void (*)(Registers&);

class SyscallManager {
 public:
  SyscallManager(const SyscallManager&) = delete;
  SyscallManager(SyscallManager&&) = delete;
  SyscallManager& operator=(const SyscallManager&) = delete;
  SyscallManager& operator=(SyscallManager&&) = delete;

  /**
   * Max value (excluded) allowed for the syscall identifier.
   */
  static constexpr size_t MAX_ENTRIES = 512;

  /**
   * Returns the global syscall manager (singleton).
   */
  [[nodiscard]] static SyscallManager& get();

  void call_syscall(uint32_t id, Registers& registers);
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
  bool register_syscall(uint32_t id, SyscallCallback callback);
  /**
   * Unregisters a previously registered syscall handler for @a id.
   *
   * If no handler was registered for @a id before, the function does nothing.
   */
  void unregister_syscall(uint32_t id);

 private:
  SyscallManager() = default;

  struct Entry {
    SyscallCallback callback = nullptr;
    uint32_t id = UINT32_MAX;
  };  // struct Entry

  Entry m_entries[MAX_ENTRIES];
};  // class SyscallManager
