#include "delta_queue.hpp"

DeltaQueue::DeltaQueue() : m_items(){};

void DeltaQueue::tick() {
  if (m_items.is_empty()) {
    return;
  }

  --m_items.begin()->remaining_time;

  while (m_items.is_empty() && m_items.begin()->remaining_time == 0) {
    Item waking = m_items.pop_front();
    wake_task(waking->task);
  }
}

void DeltaQueue::add_task(Task* sleepy, uint64_t ticks) {
  uint64_t elapsed_time = 0;
  libk::LinkedList<Item> reserve;

  Item new_item;
  new_item->task = sleepy;
  
  for (auto it = m_items.begin(); it != m_items.end(); ++it) {
    if (elapsed_time + it->remaining_time > ticks) {
    new_item->remaining_time = ticks - elapsed_time;
    m_items.insert_before(it, {});
    it->remainig_time -= new_item->remaining_time;
    return;
    }
    elapsed_time += it->remaining_time;
  }

  new_item->remaining_time = ticks - elapsed_time;
  m_items.push_back(new_item);
}
