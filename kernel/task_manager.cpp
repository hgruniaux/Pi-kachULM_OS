#include "task_manager.hpp"
#include "memory/mem_alloc.hpp"

#include <libk/log.hpp>

TaskManager* TaskManager::g_instance = nullptr;

TaskManager::TaskManager() {
  KASSERT(g_instance == nullptr && "multiple task manager created");
  g_instance = this;

  m_scheduler = libk::make_scoped<Scheduler>();
}

Task* TaskManager::create_task(const elf::Header* program_image) {
  Task* task = new Task;
  if (task == nullptr)
    return nullptr;

  // Create a process virtual memory view and allocate its stack.
  auto memory = libk::make_shared<ProcessMemory>();
  task->m_saved_state.memory = memory;
  task->m_saved_state.sp = (void*)task->m_saved_state.memory->get_stack_top();

  // Load the program segments in memory.
  for (uint64_t i = 0; i < program_image->program_header_entry_count; ++i) {
    const elf::ProgramHeader* segment = elf::get_program_header(program_image, i);
    if (segment == nullptr)
      return nullptr;

    if (segment->is_load()) {
      // FIXME: use page size constant
      const auto page_size = MemoryChunk::page_byte_size();

      // va_start is required to be on a page boundary (aligned to page size).
      const auto va_start = libk::align_to_previous(segment->virtual_addr, page_size);
      const auto nb_pages = libk::div_round_up((segment->virtual_addr - va_start) + segment->mem_size, page_size);

      // The mapped segment attributes.
      const bool is_executable = (segment->flags & elf::ProgramFlag::EXECUTABLE) != 0;
      const bool is_writable = (segment->flags & elf::ProgramFlag::WRITABLE) != 0;

      MemoryChunk& chunk = task->m_mapped_chunks.emplace_back(nb_pages);
      const bool is_mapped = memory->map_chunk(chunk, va_start, !is_writable, is_executable);
      if (!is_mapped)
        return nullptr;

      if (segment->file_size > 0) {
        const char* segment_data = (const char*)(program_image) + segment->offset;
        const size_t written_bytes = chunk.write(0, segment_data, segment->file_size);
        KASSERT(written_bytes == segment->file_size);
      }
    }
  }

  // Set the entry point of the process.
  const auto entry_addr = program_image->entry_addr;
  task->m_saved_state.regs.elr = entry_addr;

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
