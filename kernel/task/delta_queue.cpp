#include "task/delta_queue.hpp"
#include "task/task_manager.hpp"

DeltaQueue::DeltaQueue(TaskManager* task_manager) : m_items(), m_task_manager(task_manager) {}

void DeltaQueue::tick() {
  if (m_items.is_empty())
    return;

  --m_items.begin()->remaining_ticks;

  while (!m_items.is_empty() && m_items.begin()->remaining_ticks == 0) {
    const auto waking = m_items.pop_front();
    m_task_manager->wake_task(waking.task);
  }
}

void DeltaQueue::add_task(const TaskPtr& sleepy, uint64_t ticks) {
  KASSERT(!sleepy->is_running());

  uint64_t elapsed_time = 0;

  for (auto it = m_items.begin(); it != m_items.end(); ++it) {
    if (elapsed_time + it->remaining_ticks > ticks) {
      const auto remaining_time = ticks - elapsed_time;
      m_items.insert_before(it, {sleepy, remaining_time});
      it->remaining_ticks -= remaining_time;
      return;
    }

    elapsed_time += it->remaining_ticks;
  }

  m_items.push_back({sleepy, ticks - elapsed_time});
}
