#pragma once

#include <cstdint>
#include <libk/linked_list.hpp>
#include <libk/memory.hpp>

class TaskManager;
class Task;

class DeltaQueue {
 public:
  DeltaQueue(TaskManager* task_manager);

  void tick();

  void add_task(const libk::SharedPointer<Task>& task, uint64_t ticks);

 private:
  struct Item {
    libk::SharedPointer<Task> task;
    uint64_t remaining_time;
  };  // struct Item

  libk::LinkedList<Item> m_items;
  TaskManager* m_task_manager;
};  // class DeltaQueue
