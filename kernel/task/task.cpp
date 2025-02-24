#include "task.hpp"
#include <algorithm>
#include "task_manager.hpp"
#include "pipe.hpp"
#include <sys/syscall.h>

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

MemoryChunk* Task::map_chunk(size_t nb_pages, VirtualAddress addr, bool executable, bool read_only) {
  MemoryChunk& chunk = m_mapped_chunks.emplace_back(nb_pages);
  const bool is_mapped = m_saved_state.memory->map_chunk(chunk, addr, read_only, executable);
  if (!is_mapped)
    return nullptr;
  return &chunk;
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

/*
 * Pipe resources
 */

bool Task::own_pipe(const PipeResource* pipe) const {
  if (pipe == nullptr)
    return false;

  auto it = std::find(m_open_pipes.begin(), m_open_pipes.end(), pipe);
  return it != m_open_pipes.end();
}

PipeResource* Task::get_pipe(const char* name) const {
  // TODO: Maybe optimize this by using a hash table.
  for (auto& pipe : m_open_pipes) {
    if (pipe->m_parent != this || pipe->m_name == nullptr)
      continue;

    if (libk::strcmp(pipe->m_name, name) == 0)
      return pipe;
  }

  return nullptr;
}

PipeResource* Task::create_pipe(const char* name, size_t capacity) {
  KASSERT(name != nullptr && libk::strlen(name) < SYS_PIPE_NAME_MAX);
  KASSERT(capacity > 0);

  // Check if the pipe already exists.
  if (get_pipe(name) != nullptr)
    return nullptr;

  // Create the pipe.
  auto pipe = new PipeResource(capacity);
  pipe->m_parent = this;
  pipe->m_name = name;
  register_pipe(pipe);
  return pipe;
}

void Task::register_pipe(PipeResource* pipe) {
  KASSERT(pipe != nullptr);

  pipe->ref();
  m_open_pipes.push_back(pipe);
}

void Task::unregister_pipe(PipeResource* pipe) {
  KASSERT(pipe != nullptr);

  if (pipe->m_parent == this) {
    // The pipe is owned by this task. We close the pipe.
    pipe->close();
  }

  auto it = std::find(m_open_pipes.begin(), m_open_pipes.end(), pipe);
  KASSERT(it != m_open_pipes.end());
  m_open_pipes.erase(it);
  pipe->unref();
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

  // Free all open pipes.
  for (auto* pipe : m_open_pipes) {
    unregister_pipe(pipe);
  }
  m_open_pipes.clear();
}
