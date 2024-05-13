#pragma once

#include <cstdint>
#include <libk/memory.hpp>
#include "memory/process_memory.hpp"
#include "syscall_table.hpp"

struct TaskSavedState {
  Registers regs;
  libk::SharedPointer<ProcessMemory> memory;
  void* sp;  // stack pointer

  void save(const Registers& current_regs);
  void restore(Registers& current_regs);
};  // struct TaskSavedState

/**
 * Represents a runnable task in the system. This can be a user process, a thread, etc.
 */
class Task {
 public:
  using id_t = uint32_t;

  enum class State : uint8_t {
    /** The process is either executing on a CPU or waiting to be executed. */
    RUNNING,
    /** The process is suspended (sleeping) until some condition becomes true. Raising a
     * hardware interrupt, releasing a system resource the process is waiting for, or
     * delivering a signal are examples of conditions that might wake up the process (put
     * its state back to TASK_RUNNING). */
    INTERRUPTIBLE,
    /** Like TASK_INTERRUPTIBLE, except that delivering a signal to the sleeping
     * process leaves its state unchanged. This process state is seldom used. It is
     * valuable, however, under certain specific conditions in which a process must wait
     * until a given event occurs without being interrupted. For instance, this state may
     * be used when a process opens a device file and the corresponding device driver
     * starts probing for a corresponding hardware device. The device driver must not be
     * interrupted until the probing is complete, or the hardware device could be left in
     * an unpredictable state. */
    UNINTERRUPTIBLE,
    /** Process execution has been stopped. */
    STOPPED
  };  // enum class State

  [[nodiscard]] static Task* current();

  /** Gets the task identifier (process id). */
  [[nodiscard]] id_t get_id() const { return m_id; }

  /** Gets the task name. Names are not necessarily unique nor mandatory (may be null). */
  [[nodiscard]] const char* get_name() const { return m_name; }
  void set_name(const char* name) { m_name = name; }

  [[nodiscard]] bool is_running() const { return m_state == State::RUNNING; }
  [[nodiscard]] bool is_interruptible() const { return m_state == State::INTERRUPTIBLE; }
  [[nodiscard]] bool is_uninterruptible() const { return m_state == State::UNINTERRUPTIBLE; }
  /** Gets the task current state. */
  [[nodiscard]] State get_state() const { return m_state; }

  /** Gets the task saved execution state. This is all the data needed to do context switch. */
  [[nodiscard]] TaskSavedState& get_saved_state() { return m_saved_state; }
  [[nodiscard]] const TaskSavedState& get_saved_state() const { return m_saved_state; }

  /** Gets the task virtual memory. */
  [[nodiscard]] libk::SharedPointer<ProcessMemory> get_memory() const { return m_saved_state.memory; }

  /** Forward to `get_syscall_table()->call_syscall(id, registers)`. */
  void call_syscall(uint32_t id, Registers& registers) { m_syscall_table->call_syscall(id, registers); }
  /** Gets the task syscall table. */
  [[nodiscard]] SyscallTable* get_syscall_table() { return m_syscall_table; }
  [[nodiscard]] const SyscallTable* get_syscall_table() const { return m_syscall_table; }
  void set_syscall_table(SyscallTable* table) {
    KASSERT(table != nullptr);
    m_syscall_table = table;
  }

  // Forward to TaskManager functions.
  void sleep(uint64_t time_in_us);
  void pause();
  void wake();
  void kill(int exit_code = 0);

 public:  // FIXME
  friend class TaskManager;
  id_t m_id;
  State m_state = State::INTERRUPTIBLE;
  TaskSavedState m_saved_state;
  const char* m_name = nullptr;
  SyscallTable* m_syscall_table = nullptr;
  libk::LinkedList<MemoryChunk> m_mapped_chunks;
  libk::LinkedList<libk::SharedPointer<Task>> m_children;
};  // class Task
