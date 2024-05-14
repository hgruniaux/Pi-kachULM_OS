#pragma once

#include <libk/linked_list.hpp>
#include <libk/memory.hpp>
#include <libk/spinlock.hpp>

class Task;

class WaitList {
 public:
  void add(const libk::SharedPointer<Task>& task);
  void wake_one();
  void wake_all();

 private:
  libk::SpinLock m_lock;
  libk::LinkedList<libk::SharedPointer<Task>> m_wait_list;
};  // class WaitList
