#pragma once

#include <libk/linked_list.hpp>
#include "task.hpp"

class Scheduler {
 public:
  Scheduler();
  Scheduler(const Scheduler&) = delete;
  Scheduler(Scheduler&&) = delete;
  Scheduler& operator=(const Scheduler&) = delete;
  Scheduler& operator=(Scheduler&&) = delete;

  static Scheduler& get() { return *g_instance; }

  /** Returns the task being currently run. */
  [[nodiscard]] Task* get_current_task() { return m_current_task; }
  [[nodiscard]] const Task* get_current_task() const { return m_current_task; }

  void add_task(Task* task);
  bool remove_task(Task* task);

  void schedule();

 private:
  static Scheduler* g_instance;
  Task* m_current_task = nullptr;
  libk::LinkedList<Task*> m_run_queue;
};  // class Scheduler
