#include "syscall.hpp"
#include <libk/assert.hpp>
#include <libk/log.hpp>

void SyscallTable::call_syscall(uint32_t id, Registers& registers) {
  if (id >= MAX_ENTRIES || (m_entries[id].id != id)) {
    // Invalid syscall number
    LOG_WARNING("invalid syscall #{}", id);
    registers.x0 = 0;
    return;
  }

  LOG_TRACE("syscall #{}", id);

  const Entry& entry = m_entries[id];
  entry.callback(registers);
}

bool SyscallTable::register_syscall(uint32_t id, SyscallCallback callback) {
  KASSERT(id < MAX_ENTRIES);

  Entry& entry = m_entries[id];
  if (entry.id == id) {
    LOG_WARNING("syscall #{} registered more than once", id);
    return false;
  }

  LOG_TRACE("syscall #{} registered", id);
  entry.callback = callback;
  entry.id = id;
  return true;
}

void SyscallTable::unregister_syscall(uint32_t id) {
  KASSERT(id < MAX_ENTRIES);

  Entry& entry = m_entries[id];
  if (entry.id == id) {
    entry.id = UINT32_MAX;
    entry.callback = nullptr;
  }
}
