#pragma once

#include <libk/linked_list.hpp>
#include "task.hpp"

class Scheduler {
 public:
  static constexpr uint32_t MIN_PRIORITY = 0;
  static constexpr uint32_t MAX_PRIORITY = 31;
  static constexpr uint32_t DEFAULT_PRIORITY = (MAX_PRIORITY - MIN_PRIORITY) / 2;

  Scheduler();

  // No copy and move
  Scheduler(const Scheduler&) = delete;
  Scheduler(Scheduler&&) = delete;
  Scheduler& operator=(const Scheduler&) = delete;
  Scheduler& operator=(Scheduler&&) = delete;

  static Scheduler& get() { return *g_instance; }

  /** Returns the task being currently run. */
  [[nodiscard]] TaskPtr get_current_task() const { return m_current_task; }

  void add_task(const TaskPtr& task);
  bool remove_task(const TaskPtr& task);
  void update_task_priority(const TaskPtr& task, uint32_t old_priority);

  void schedule();
  void tick();

 private:
  [[nodiscard]] uint32_t get_current_priority() const;
  [[nodiscard]] TaskPtr find_higher_priority_task_than_current();
  void switch_to(const libk::SharedPointer<Task>& new_task);

  static Scheduler* g_instance;
  TaskPtr m_current_task = nullptr;

  static constexpr uint32_t TIME_SLICE = 10;
  uint32_t m_elapsed_ticks = 0;

  static constexpr uint32_t PRIORITY_COUNT = MAX_PRIORITY - MIN_PRIORITY + 1;
  libk::LinkedList<TaskPtr> m_run_queue[PRIORITY_COUNT];
};  // class Scheduler
