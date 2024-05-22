#include "task.hpp"
#include <algorithm>
#include "task_manager.hpp"

#include "fs/filesystem.hpp"
#include "wm/window.hpp"
#include "wm/window_manager.hpp"

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

Task::~Task() {
  free_resources();
}

TaskPtr Task::current() {
  return TaskManager::get().get_current_task();
}

bool Task::own_window(Window* window) const {
  if (window == nullptr)
    return false;

  auto it = std::find(m_windows.begin(), m_windows.end(), window);
  return it != m_windows.end();
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

bool Task::own_file(File* file) const {
  if (file == nullptr)
    return false;

  auto it = std::find(m_open_files.begin(), m_open_files.end(), file);
  return it != m_open_files.end();
}

void Task::register_file(File* file) {
  KASSERT(file != nullptr);

  m_open_files.push_back(file);
}

void Task::unregister_file(File* file) {
  KASSERT(file != nullptr);

  auto it = std::find(m_open_files.begin(), m_open_files.end(), file);
  KASSERT(it != m_open_files.end());
  m_open_files.erase(it);
}

bool Task::own_dir(Dir* dir) const {
  if (dir == nullptr)
    return false;

  auto it = std::find(m_open_dirs.begin(), m_open_dirs.end(), dir);
  return it != m_open_dirs.end();
}

void Task::register_dir(Dir* dir) {
  KASSERT(dir != nullptr);

  m_open_dirs.push_back(dir);
}

void Task::unregister_dir(Dir* dir) {
  KASSERT(dir != nullptr);

  auto it = std::find(m_open_dirs.begin(), m_open_dirs.end(), dir);
  KASSERT(it != m_open_dirs.end());
  m_open_dirs.erase(it);
}

void Task::free_resources() {
  // Destroy the windows.
  auto& window_manager = WindowManager::get();
  for (auto* window : m_windows) {
    window_manager.destroy_window(window);
  }

  m_windows.clear();

  // Free all open files.
  auto& fs = FileSystem::get();
  for (auto* file : m_open_files) {
    fs.close(file);
  }

  m_open_files.clear();

  // Free all open dirs.
  for (auto* dir : m_open_dirs) {
    fs.close_dir(dir);
  }

  m_open_dirs.clear();
}
