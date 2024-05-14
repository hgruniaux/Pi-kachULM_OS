#include "task/delta_queue.hpp"
#include "task/task_manager.hpp"

DeltaQueue::DeltaQueue(TaskManager* task_manager) : m_items(), m_task_manager(task_manager) {}

void DeltaQueue::tick() {
  if (m_items.is_empty())
    return;

  --m_items.begin()->remaining_time;

  while (m_items.is_empty() && m_items.begin()->remaining_time == 0) {
    const auto waking = m_items.pop_front();
    m_task_manager->wake_task(waking.task);
  }
}

void DeltaQueue::add_task(Task* sleepy, uint64_t ticks) {
  uint64_t elapsed_time = 0;

  for (auto it = m_items.begin(); it != m_items.end(); ++it) {
    if (elapsed_time + it->remaining_time > ticks) {
      const auto remaining_time = ticks - elapsed_time;
      m_items.insert_before(it, {sleepy, remaining_time});
      it->remaining_time -= remaining_time;
      return;
    }

    elapsed_time += it->remaining_time;
  }

  m_items.push_back({sleepy, ticks - elapsed_time});
}
