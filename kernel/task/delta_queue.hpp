#pragma once
#include "libk/linked_list.hpp"
#include "task_manager.hpp"

class DeltaQueue {
 public:
  void tick();
  void add_task(Task* task, uint64_t ticks);

 private:
  struct Item {
    Task* task;
    uint64_t remaining_time;
  };  // struct Item

  libk::LinkedList<Item> m_items;
};  // class DeltaQueue