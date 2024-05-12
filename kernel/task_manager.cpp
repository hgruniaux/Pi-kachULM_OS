#include "task_manager.hpp"
#include "memory/mem_alloc.hpp"

#include <libk/log.hpp>

TaskManager* TaskManager::g_instance = nullptr;

TaskManager::TaskManager() {
  KASSERT(g_instance == nullptr && "multiple task manager created");
  g_instance = this;

  m_scheduler = new Scheduler();
}

Task* TaskManager::create_task() {
  Task* task = new Task;
  if (task == nullptr)
    return nullptr;

  libk::bzero(&task->m_saved_state, sizeof(task->m_saved_state));

  // Allocate a new stack of the task.
  constexpr size_t STACK_SIZE = 4096 * 2;
  task->m_stack = kmalloc(STACK_SIZE, 16);
  task->m_saved_state.regs.x28 = ((uintptr_t)task->m_stack + STACK_SIZE);
  if (task->m_stack == nullptr) {
    // Failed to allocate a new stack, something bad is happening...
    delete task;
    return nullptr;
  }

  libk::bzero(task->m_stack, STACK_SIZE);

  // task->m_saved_state.regs.x29 = (uint64_t)(uintptr_t)task->m_saved_state.sp;

  task->m_id = m_next_available_pid++;
  // FIXME: register id mapping

  task->m_syscall_table = m_default_syscall_table;
  m_tasks.push_back(task);

  LOG_DEBUG("Create a new task with pid={}", task->get_id());
  return task;
}

void TaskManager::pause_task(Task* task) {
  KASSERT(task != nullptr);

  if (!task->is_running())
    return;

  LOG_DEBUG("Pause the task pid={}", task->get_id());

  task->m_state = Task::State::UNINTERRUPTIBLE;
  m_scheduler->remove_task(task);
}

void TaskManager::wake_task(Task* task) {
  KASSERT(task != nullptr);

  if (task->is_running())
    return;

  LOG_DEBUG("Wake the task pid={}", task->get_id());

  task->m_state = Task::State::RUNNING;
  m_scheduler->add_task(task);
}

void TaskManager::kill_task(Task* task, int exit_code) {
  KASSERT(task != nullptr);

  LOG_DEBUG("Kill the task pid={} with status {}", task->get_id(), exit_code);

  task->m_state = Task::State::UNINTERRUPTIBLE;
  m_scheduler->remove_task(task);

  kfree(task->m_stack);
  delete task;
}

void TaskManager::kill_current_task(int exit_code) {
  Task* current_task = get_current_task();
  if (current_task == nullptr)
    return;

  kill_task(current_task, exit_code);
}

Task* TaskManager::get_current_task() const {
  return m_scheduler->get_current_task();
}

void TaskManager::schedule() {
  m_scheduler->schedule();
}
