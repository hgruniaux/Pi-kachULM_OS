#include "wait_list.hpp"
#include "task.hpp"
#include "task_manager.hpp"

void WaitList::add(const libk::SharedPointer<Task>& task) {
  KASSERT(task);

  if (task->is_terminated())
    return;

  task->get_manager()->pause_task(task);
  m_wait_list.push_back(task);
}

void WaitList::wake_one() {
  // Retrieve the first task that is not terminated.
  libk::SharedPointer<Task> task;
  do {
    if (m_wait_list.is_empty())
      return;

    task = m_wait_list.pop_front();
  } while (!task->is_terminated());

  task->get_manager()->wake_task(task);
}

void WaitList::wake_all() {
  // Wake all tasks.
  for (auto& task : m_wait_list) {
    if (task->is_terminated())
      continue;

    task->get_manager()->wake_task(task);
  }

  m_wait_list.clear();
}
