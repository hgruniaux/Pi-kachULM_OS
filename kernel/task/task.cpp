#include "task.hpp"
#include "task_manager.hpp"

void TaskSavedState::save(const Registers& current_regs) {
  regs = current_regs;
  // Save the stack pointer.
  asm volatile("mrs %0, SP_EL0" : "=r"(sp));
}

void TaskSavedState::restore(Registers& current_regs) {
  current_regs = regs;
  // Restore the stack pointer.
  asm volatile("msr SP_EL0, %0" : : "r"(sp));
  if (memory)
    memory->activate();
}

Task* Task::current() {
  return TaskManager::get().get_current_task();
}

void Task::sleep(uint64_t time_in_us) {
  return TaskManager::get().sleep_task(this, time_in_us);
}

void Task::pause() {
  return TaskManager::get().pause_task(this);
}

void Task::wake() {
  return TaskManager::get().wake_task(this);
}

void Task::kill(int exit_code) {
  return TaskManager::get().kill_task(this, exit_code);
}
