#include "syscall_table.hpp"
#include <libk/assert.hpp>
#include <libk/log.hpp>

SyscallTable::SyscallTable() {
  // Use the default unknown callback for now.
  set_unknown_callback(nullptr);
}

void SyscallTable::call_syscall(id_t id, Registers& registers) {
  if (id >= MAX_ID || m_entries[id] == nullptr) {
    m_unknown_callback(registers);
    return;
  }

  m_entries[id](registers);
}

void SyscallTable::set_unknown_callback(SyscallCallback callback) {
  if (callback != nullptr) {
    m_unknown_callback = callback;
    return;
  }

  // Provide a default placeholder callback.
  m_unknown_callback = [](Registers& regs) {
    LOG_ERROR("Unknown syscall called");
    regs.x0 = 0;
  };
}

bool SyscallTable::register_syscall(id_t id, SyscallCallback callback) {
  KASSERT(id < MAX_ID);

  if (m_entries[id] != nullptr) {
    LOG_WARNING("syscall #{} registered more than once", id);
    return false;
  }

  m_entries[id] = callback;
  return true;
}

void SyscallTable::unregister_syscall(id_t id) {
  KASSERT(id < MAX_ID);
  m_entries[id] = nullptr;
}
