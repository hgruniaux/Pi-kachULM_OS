#include "task.hpp"
#include <algorithm>
#include "task_manager.hpp"
#include "wm/window.hpp"

void TaskSavedState::save(const Registers& current_regs) {
  gp_regs = current_regs.gp_regs;
  fpu_regs.save();
  pc = current_regs.elr;

  // Save the stack pointer.
  asm volatile("mrs %0, SP_EL0" : "=r"(sp));
}

void TaskSavedState::restore(Registers& current_regs) {
  current_regs.gp_regs = gp_regs;
  fpu_regs.restore();
  current_regs.elr = pc;

  if (is_kernel) {
    current_regs.spsr &= ~0b1111ull;
    current_regs.spsr |= 0b0100;
  } else {
    current_regs.spsr &= ~0b1111ull;
  }

  // Restore the stack pointer.
  asm volatile("msr SP_EL0, %0" : : "r"(sp));

  if (memory)
    memory->activate();
}

TaskPtr Task::current() {
  return TaskManager::get().get_current_task();
}

void Task::register_window(Window* window) {
  KASSERT(window != nullptr && window->get_task().get() == this);
  m_windows.push_back(window);
}

void Task::unregister_window(Window* window) {
  KASSERT(window != nullptr && window->get_task().get() == this);

  auto it = std::find(m_windows.begin(), m_windows.end(), window);
  KASSERT(it != m_windows.end());
  m_windows.erase(it);
}
