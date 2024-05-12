#include "task.hpp"

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
