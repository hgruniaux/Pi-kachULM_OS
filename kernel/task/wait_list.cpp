#include "wait_list.hpp"

void WaitList::add(const libk::SharedPointer<Task>& task) {
  KASSERT(task);

  const libk::SpinLockGuard lock(m_lock);

  if (task->is_terminated())
    return;

  task->pause();
  m_wait_list.push_back(task);
}

void WaitList::wake_one() {
  const libk::SpinLockGuard lock(m_lock);

  // Retrieve the first task that is not terminated.
  libk::SharedPointer<Task> task;
  do {
    if (m_wait_list.is_empty())
      return;

    task = m_wait_list.pop_front();
  } while (!task->is_terminated());

  task->wake();
}

void WaitList::wake_all() {
  const libk::SpinLockGuard lock(m_lock);

  // Wake all tasks.
  for (auto& task : m_wait_list) {
    if (task->is_terminated())
      continue;

    task->wake();
  }

  m_wait_list.clear();
}
