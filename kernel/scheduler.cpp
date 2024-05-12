#include "scheduler.hpp"

#include <libk/log.hpp>
#include <algorithm>

Scheduler* Scheduler::g_instance = nullptr;

Scheduler::Scheduler() {
  g_instance = this;
}

void Scheduler::add_task(Task* task) {
  KASSERT(task != nullptr);
  m_run_queue.push_back(task);
}

bool Scheduler::remove_task(Task* task) {
  KASSERT(task != nullptr);

  if (m_current_task == task) {
    if (m_run_queue.is_empty()) {
      m_current_task = nullptr;
    } else {
      m_current_task = m_run_queue.pop_front();
    }

    if (m_current_task != nullptr)
      LOG_DEBUG("New scheduled task is pid={}", m_current_task->get_id());
    return true;
  }

  auto it = std::find(m_run_queue.begin(), m_run_queue.end(), task);
  if (it == m_run_queue.end())
    return false;

  m_run_queue.erase(it);
  return true;
}

void Scheduler::schedule() {
  // For scheduling, we use a quite simple algorithm: round-robin scheduling.
  // The idea is that each task is assigned some time slice. Once a time slice is
  // expired for a process, we switch to the next task in the run queue, and add
  // the previous task at the end of the queue.

  if (m_run_queue.is_empty())
    return;

  Task* new_task = m_run_queue.pop_front();

  if (m_current_task != nullptr) {
    m_run_queue.push_back(m_current_task);
  }

  if (new_task != nullptr)
    LOG_DEBUG("New scheduled task is pid={}", new_task->get_id());
  m_current_task = new_task;
}
