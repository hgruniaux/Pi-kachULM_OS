#include "scheduler.hpp"

#include <algorithm>
#include <libk/log.hpp>

Scheduler* Scheduler::g_instance = nullptr;

Scheduler::Scheduler() {
  g_instance = this;
}

void Scheduler::add_task(const TaskPtr& task) {
  KASSERT(task != nullptr);
  KASSERT(!task->is_running());

  const uint32_t priority = task->get_priority();
  KASSERT(priority >= MIN_PRIORITY && priority <= MAX_PRIORITY);
  m_run_queue[priority].push_back(task);
}

bool Scheduler::remove_task(const TaskPtr& task) {
  KASSERT(task != nullptr);
  KASSERT(task->is_running());

  const uint32_t priority = task->get_priority();
  KASSERT(priority >= MIN_PRIORITY && priority <= MAX_PRIORITY);

  // The task to remove is the current one.
  // This case is easy: reschedule.
  if (m_current_task == task) {
    m_current_task = nullptr;
    schedule();
    return true;
  }

  // Otherwise, the task is probably in the run queue.
  auto& run_queue = m_run_queue[priority];
  auto it = std::find(run_queue.begin(), run_queue.end(), task);
  if (it == run_queue.end())
    return false;  // but it can also not be registered in the scheduler (e.g. paused task)

  run_queue.erase(it);
  return true;
}

void Scheduler::update_task_priority(const TaskPtr& task, uint32_t old_priority) {
  KASSERT(task != nullptr);

  if (m_current_task == task) {
    // Its current task! Nothing to do as the task is not in a run queue.
    return;
  }

  const uint32_t new_priority = task->get_priority();
  KASSERT(new_priority >= MIN_PRIORITY && new_priority <= MAX_PRIORITY);

  if (new_priority == old_priority)
    return;  // nothing has changed...

  // Remove the task from its old run queue.
  auto& run_queue = m_run_queue[old_priority];
  auto it = std::find(run_queue.begin(), run_queue.end(), task);
  if (it == run_queue.end())
    return;  // The task was not registered in the scheduler, stop there is nothing to update.

  run_queue.erase(it);

  // Add back the task to its new run queue.
  m_run_queue[new_priority].push_back(task);
}

void Scheduler::schedule() {
  auto old_task = Task::current();
  if (old_task != nullptr && !old_task->can_preempt())
    return;  // we cannot preempt the current task.

  TaskPtr new_task = nullptr;

  // Find a new task starting with higher priority tasks.
  for (int i = MAX_PRIORITY; i >= (int)MIN_PRIORITY; --i) {
    auto& run_queue = m_run_queue[i];
    if (!run_queue.is_empty()) {
      new_task = run_queue.pop_front();
      break;
    }
  }

  switch_to(new_task);

#if LOG_MIN_LEVEL <= LOG_TRACE_LEVEL
  const auto old_task_id = old_task ? old_task->get_id() : UINT16_MAX;
  const auto new_task_id = new_task ? new_task->get_id() : UINT16_MAX;
  if (old_task_id != new_task_id)
    LOG_TRACE("Scheduler: old={}, new={}", old_task_id, new_task_id);
#endif
}

void Scheduler::tick() {
  auto old_task = Task::current();

  // Algorithm overview:
  //   - We have multiple run queues, one per thread priority (currently there are 32 priorities).
  //   - At each tick:
  //      - If there is a higher priority task, switch to it unconditionally.
  //      - Otherwise, do a round-robin scheduling of tasks inside the same run queue
  //        as the current task. We respect allocated time slices, that is we do nothing
  //        if the current task has not consumed all its CPU ticks.
  //        If there are no more tasks with the same priority, we fall back to use
  //        lower priority tasks.
  // This algorithm is called multilevel queue scheduling. It has some advantages:
  //   - It is quite simple and efficient.
  //   - Higher priority tasks can preempt.
  // However, sometimes lower priority tasks can starve.

  if (old_task != nullptr)
    old_task->m_elapsed_ticks++;

  if (old_task != nullptr && !old_task->can_preempt())
    return;  // we cannot preempt the current task.

  // Check if there is a waiting process with a higher priority.
  TaskPtr new_task = find_higher_priority_task_than_current();

  // Otherwise, check if the current task time slice has expired and if yes
  // then schedule using round-robin.
  if (new_task == nullptr &&
      (old_task != nullptr && old_task->m_elapsed_ticks >= get_time_slice_for_priority(old_task->get_priority()))) {
    const uint32_t current_priority = get_current_priority();
    for (int i = (int)current_priority; i >= (int)MIN_PRIORITY; --i) {
      if (!m_run_queue[current_priority].is_empty()) {
        new_task = m_run_queue[current_priority].pop_front();
        break;
      }
    }
  }

  switch_to(new_task);

#if LOG_MIN_LEVEL <= LOG_TRACE_LEVEL
  if (new_task == nullptr)
    return;

  const auto old_task_id = old_task ? old_task->get_id() : UINT16_MAX;
  const auto new_task_id = new_task ? new_task->get_id() : UINT16_MAX;
  if (old_task_id != new_task_id)
    LOG_TRACE("Scheduler (tick): old={}, new={}", old_task_id, new_task_id);
#endif
}

uint32_t Scheduler::get_current_priority() const {
  if (m_current_task == nullptr)
    return 0;
  return m_current_task->get_priority();
}

TaskPtr Scheduler::find_higher_priority_task_than_current() {
  const uint32_t current_priority = get_current_priority();
  for (uint32_t i = MAX_PRIORITY; i > current_priority; --i) {
    auto& run_queue = m_run_queue[i];
    if (!run_queue.is_empty()) {
      return run_queue.pop_front();
    }
  }

  return nullptr;
}

void Scheduler::switch_to(const TaskPtr& new_task) {
  if (new_task == nullptr)
    return;  // no new task, nothing to do

  KASSERT(new_task->is_running());

  // Enqueue again the old task into the run queue.
  if (m_current_task != nullptr && m_current_task != new_task) {
    const uint32_t current_priority = m_current_task->get_priority();
    m_run_queue[current_priority].push_back(m_current_task);
  }

  m_current_task = new_task;
  m_current_task->m_elapsed_ticks = 0;  // start a new time slice for the new task
}

uint64_t Scheduler::get_time_slice_for_priority(uint32_t priority) {
  switch (priority) {
    case 0:  // low-priority process, try to preempt more often.
      return 1;
    default:
      return 10;
  }
}
