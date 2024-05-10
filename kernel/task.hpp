#pragma once

#include <cstdint>
#include "syscall.hpp"

struct TaskSavedState {
  Registers regs;
  uint64_t pc;
};  // struct TaskSavedState

/**
 * Represents a runnable task in the system. This can be a user process, a thread, etc.
 */
class Task {
 public:
  using id_t = uint32_t;

  enum class State : uint8_t {
    /** The task is running or in a run-queue about to be running. */
    RUNNING,
    /** The task is sleeping. */
    INTERRUPTIBLE,
    /** The task is sleeping, and cannot be waken by the scheduler automatically. */
    UNINTERRUPTIBLE,
  };  // enum class State

  /** Gets the task identifier (process id). */
  [[nodiscard]] id_t get_id() const { return m_id; }

  [[nodiscard]] bool is_running() const { return m_state == State::RUNNING; }
  [[nodiscard]] bool is_interruptible() const { return m_state == State::INTERRUPTIBLE; }
  [[nodiscard]] bool is_uninterruptible() const { return m_state == State::UNINTERRUPTIBLE; }
  /** Gets the task current state. */
  [[nodiscard]] State get_state() const { return m_state; }

  [[nodiscard]] TaskSavedState& get_saved_state() { return m_saved_state; }
  [[nodiscard]] const TaskSavedState& get_saved_state() const { return m_saved_state; }

  /** Forward to `get_syscall_table()->call_syscall(id, registers)`. */
  void call_syscall(uint32_t id, Registers& registers) { m_syscall_table->call_syscall(id, registers); }
  /** Gets the task syscall table. */
  [[nodiscard]] SyscallTable* get_syscall_table() { return m_syscall_table; }
  [[nodiscard]] const SyscallTable* get_syscall_table() const { return m_syscall_table; }

 public:  // FIXME
  id_t m_id;
  State m_state;
  TaskSavedState m_saved_state;
  SyscallTable* m_syscall_table;
};  // class Task
