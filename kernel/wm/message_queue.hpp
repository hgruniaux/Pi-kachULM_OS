#pragma once

#include <sys/window.h>
#include "task/wait_list.hpp"

class MessageQueue {
 public:
  static constexpr size_t MAX_PENDING_MESSAGES = 64;

  [[nodiscard]] bool is_empty() const { return m_pending_count == 0; }
  [[nodiscard]] bool is_full() const { return m_pending_count == MAX_PENDING_MESSAGES; }
  [[nodiscard]] size_t get_pending_count() const { return m_pending_count; }

  /** Returns true if succeeded (queue not full). */
  bool enqueue(const sys_message_t& msg);
  /** Returns true if succeeded (queue not empty). */
  bool dequeue(sys_message_t& msg);
  /** Returns true if succeeded (queue not empty). */
  bool peek(sys_message_t& msg);

  /**
   * Blocks the given task until this message is not anymore empty.
   * The given task will be paused and awaken at the next enqueue call.
   * If the message queue is not yet empty, nothing is done.
   *
   * Returns true if the task was blocked (the queue is currently empty).
   * Otherwise, returns false.
   */
  bool block_task_until_not_empty(const libk::SharedPointer<Task>& task);

 private:
  WaitList m_wait_list;
  size_t m_pending_count = 0;
  sys_message_t m_queue[MAX_PENDING_MESSAGES];
};  // class MessageQueue
