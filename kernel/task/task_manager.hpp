#pragma once

#include <elf/elf.hpp>
#include <libk/hash_table.hpp>
#include <libk/linked_list.hpp>
#include <libk/memory.hpp>
#include "delta_queue.hpp"
#include "scheduler.hpp"
#include "task.hpp"

class TaskManager {
 public:
  /** Time (in milliseconds) between each tick for the scheduler. */
  static constexpr uint32_t TICK_TIME = 100;

  TaskManager();

  [[nodiscard]] static TaskManager& get() { return *g_instance; }

  [[nodiscard]] SyscallTable* get_default_syscall_table() const { return m_default_syscall_table; }
  void set_default_syscall_table(SyscallTable* table) { m_default_syscall_table = table; }

  TaskPtr create_task(Task* parent = nullptr);
  TaskPtr create_task(const elf::Header* program_image);

  /**
   * Put the given task to sleep for a minimum duration given by @a time_in_us (in microseconds).
   *
   * @warning @a task is expected to be non null and previously
   * created by this task manager.
   */
  void sleep_task(const TaskPtr& task, uint64_t time_in_us);

  /**
   * Pauses the given task until it is awakened again by a call to wake_task().
   *
   * If the task was already paused before, this function does nothing.
   *
   * @warning @a task is expected to be non null and previously
   * created by this task manager.
   */
  void pause_task(const TaskPtr& task);
  /**
   * Wake up the given task that has been previously put to sleep by calling pause_task().
   *
   * If the task was not paused before, this function does nothing.
   *
   * @warning @a task is expected to be non null and previously
   * created by this task manager.
   */
  void wake_task(const TaskPtr& task);

  /**
   * Terminates the given task and sets the given exit code.
   *
   * The task object is deleted. Do not use it after this function.
   *
   * @warning @a task is expected to be non null and previously
   * created by this task manager.
   */
  void kill_task(const TaskPtr& task, int exit_code = 0);

  bool set_task_priority(const TaskPtr& task, uint32_t new_priority);

  [[nodiscard]] TaskPtr get_current_task() const;

  void schedule();
  void tick();

 private:
  static TaskManager* g_instance;
  libk::ScopedPointer<Scheduler> m_scheduler;
  libk::LinkedList<libk::SharedPointer<Task>> m_tasks;
  //  libk::HashTable<Task::id_t, Task*> m_id_mapping;  // Unused
  Task::id_t m_next_available_pid = 0;
  SyscallTable* m_default_syscall_table = nullptr;
  DeltaQueue m_delta_queue;
};  // class TaskManager
