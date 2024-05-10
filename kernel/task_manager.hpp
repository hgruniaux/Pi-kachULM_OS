#pragma once

#include "scheduler.hpp"
#include "task.hpp"

class TaskManager {
 public:
 private:
  libk::LinkedList<Task*> m_tasks;
};  // class TaskManager
