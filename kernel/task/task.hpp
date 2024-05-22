#pragma once

#include <cstdint>
#include <libk/memory.hpp>
#include "hardware/regs.hpp"
#include "memory/process_memory.hpp"
#include "task/syscall_table.hpp"

struct TaskSavedState {
  GPRegisters gp_regs;
  FPURegisters fpu_regs;
  libk::SharedPointer<ProcessMemory> memory;
  uint64_t pc;  // program counter
  uint64_t sp;  // stack pointer
  bool is_kernel;

  void save(const Registers& current_regs);
  void restore(Registers& current_regs);
};  // struct TaskSavedState

class TaskManager;
class Window;
class File;

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
    /** Process has been terminated/killed. */
    TERMINATED
  };  // enum class State

  ~Task();

  /** Gets the current active (running) task. This forward to TaskManager::get_current_task(). */
  [[nodiscard]] static libk::SharedPointer<Task> current();

  /** Gets the task identifier (process id). */
  [[nodiscard]] id_t get_id() const { return m_id; }

  /** Gets the task name. Names are not necessarily unique nor mandatory (may be null). */
  [[nodiscard]] const char* get_name() const { return m_name; }
  void set_name(const char* name) { m_name = name; }

  [[nodiscard]] bool is_running() const { return m_state == State::RUNNING; }
  [[nodiscard]] bool is_interruptible() const { return m_state == State::INTERRUPTIBLE; }
  [[nodiscard]] bool is_uninterruptible() const { return m_state == State::UNINTERRUPTIBLE; }
  [[nodiscard]] bool is_terminated() const { return m_state == State::TERMINATED; }
  /** Gets the task current state. */
  [[nodiscard]] State get_state() const { return m_state; }

  /** Gets the task priority for scheduling. The larger it is, the higher the process priority. */
  [[nodiscard]] uint32_t get_priority() const { return m_priority; }

  /** Gets the task manager that that ownership over this task. */
  [[nodiscard]] TaskManager* get_manager() { return m_manager; }
  [[nodiscard]] const TaskManager* get_manager() const { return m_manager; }

  [[nodiscard]] bool has_parent() const { return m_parent != nullptr; }
  /** Gets the task parent if any. */
  [[nodiscard]] Task* get_parent() { return m_parent; }
  [[nodiscard]] const Task* get_parent() const { return m_parent; }

  [[nodiscard]] auto children_begin() const { return m_children.begin(); }
  [[nodiscard]] auto children_end() const { return m_children.begin(); }

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

  [[nodiscard]] bool is_marked_to_be_killed() const { return m_marked_kill; }
  void mark_to_be_killed() { m_marked_kill = true; }

  [[nodiscard]] bool own_window(Window* window) const;
  void register_window(Window* window);
  void unregister_window(Window* window);

  [[nodiscard]] bool own_file(File* file) const;
  void register_file(File* file);
  void unregister_file(File* file);

  [[nodiscard]] bool can_preempt() const { return m_preempt_count == 0; }
  void disable_preempt() { m_preempt_count++; }
  void enable_preempt() {
    m_preempt_count--;
    KASSERT(m_preempt_count >= 0);
  }

 private:
  void free_resources();

 private:
  friend class TaskManager;
  friend class Scheduler;

  id_t m_id;
  State m_state = State::INTERRUPTIBLE;
  uint32_t m_priority = 0;
  TaskSavedState m_saved_state;
  const char* m_name = nullptr;
  SyscallTable* m_syscall_table = nullptr;
  TaskManager* m_manager = nullptr;
  uint64_t m_elapsed_ticks = 0;
  bool m_is_kernel = false;    // it is a kernel stack (in EL1)?
  bool m_marked_kill = false;  // is the task marked to be called at the next context switch?
  int m_preempt_count = 0;

  // Parent-children relationship.
  Task* m_parent = nullptr;  // not a SharedPointer to avoid cyclic dependencies
  libk::LinkedList<libk::SharedPointer<Task>> m_children;

  // Task resources
  libk::LinkedList<Window*> m_windows;
  libk::LinkedList<File*> m_open_files;
  libk::LinkedList<MemoryChunk> m_mapped_chunks;
};  // class Task

using TaskPtr = libk::SharedPointer<Task>;
