#include "message_queue.hpp"

#include <libk/string.hpp>

bool MessageQueue::enqueue(const sys_message_t& msg) {
  if (is_full())
    return false;

  m_queue[m_pending_count] = msg;
  m_pending_count++;
  m_wait_list.wake_all();
  return true;
}

bool MessageQueue::dequeue(sys_message_t& msg) {
  if (is_empty())
    return false;

  m_pending_count--;
  msg = m_queue[0];
  libk::memmove(&m_queue[0], &m_queue[1], sizeof(m_queue[0]) * m_pending_count);
  return true;
}

bool MessageQueue::peek(sys_message_t& msg) {
  if (is_empty())
    return false;

  msg = m_queue[0];
  return true;
}

bool MessageQueue::block_task_until_not_empty(const libk::SharedPointer<Task>& task) {
  if (!is_empty())
    return false;

  m_wait_list.add(task);
  return true;
}
