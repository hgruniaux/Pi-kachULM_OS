#include "task_manager.hpp"
#include "hardware/interrupts.hpp"
#include "hardware/system_timer.hpp"
#include "memory/mem_alloc.hpp"
#include "pika_syscalls.hpp"

#include <libk/log.hpp>

TaskManager* TaskManager::g_instance = nullptr;

TaskManager::TaskManager() : m_delta_queue(this) {
  KASSERT(g_instance == nullptr && "multiple task manager created");
  g_instance = this;

  m_default_syscall_table = create_pika_syscalls();
  m_scheduler = libk::make_scoped<Scheduler>();

  // Select a timer and start the tick clock for the scheduler.
  bool timer_found = false;
  for (size_t timer_id = 0; timer_id < SystemTimer::nb_timers; ++timer_id) {
    if (SystemTimer::is_used(timer_id))
      continue;

    if (SystemTimer::set_recurrent_ms(timer_id, TICK_TIME, []() { TaskManager::get().tick(); })) {
      LOG_INFO("System timer {} used for scheduler with tick time {} ms", timer_id, TICK_TIME);
      timer_found = true;
      break;
    }
  }

  if (!timer_found) {
    LOG_CRITICAL("Not found an available system timer for the scheduler");
    return;
  }
}

TaskPtr TaskManager::create_task(Task* parent) {
  DisableInterrupts disable_interrupts;

  auto task = libk::make_shared<Task>();
  if (!task)
    return nullptr;

  task->m_manager = this;

  // Set parent-child relationship.
  if (parent != nullptr) {
    task->m_parent = parent;
    parent->m_children.push_back(task);
  }

  // Create a process virtual memory view and allocate its stack.
  const auto stack_size = MemoryChunk::get_page_byte_size() * 2;
  auto memory = libk::make_shared<ProcessMemory>(stack_size);
  task->m_saved_state.memory = memory;
  task->m_saved_state.sp = (void*)task->m_saved_state.memory->get_stack_start();

  // Set process unique ID.
  task->m_id = m_next_available_pid++;
  // FIXME: register id mapping

  task->m_priority = Scheduler::DEFAULT_PRIORITY;
  task->m_syscall_table = m_default_syscall_table;

  m_tasks.push_back(task);
  LOG_DEBUG("Create a new task with pid={}", task->get_id());
  return task;
}

TaskPtr TaskManager::create_task(const elf::Header* program_image) {
  DisableInterrupts disable_interrupts;

  auto task = create_task();
  if (!task)
    return nullptr;

  // Load the program segments in memory.
  ProcessMemory* memory = task->get_memory().get();
  for (uint64_t i = 0; i < program_image->program_header_entry_count; ++i) {
    const elf::ProgramHeader* segment = elf::get_program_header(program_image, i);
    if (segment == nullptr)
      return nullptr;

    if (segment->is_load()) {
      const auto page_size = MemoryChunk::get_page_byte_size();

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
        const size_t written_bytes = chunk.write(segment->virtual_addr - va_start, segment_data, segment->file_size);
        KASSERT(written_bytes == segment->file_size);
      }
    }
  }

  // Set the entry point of the process.
  const auto entry_addr = program_image->entry_addr;
  task->m_saved_state.regs.elr = entry_addr;
  return task;
}

void TaskManager::sleep_task(const TaskPtr& task, uint64_t time_in_us) {
  KASSERT(task != nullptr);
  KASSERT(task->get_manager() == this);
  KASSERT(!task->is_terminated());

  DisableInterrupts disable_interrupts;

  if (!task->is_running())
    return;

  const uint64_t ticks_count = time_in_us / 1000 / TICK_TIME;
  LOG_DEBUG("Sleep the task pid={} for {} us (i.e. {} ticks)", task->get_id(), time_in_us, ticks_count);

  m_scheduler->remove_task(task);
  task->m_state = Task::State::INTERRUPTIBLE;
  m_delta_queue.add_task(task, ticks_count);
}

void TaskManager::pause_task(const TaskPtr& task) {
  KASSERT(task != nullptr);
  KASSERT(task->get_manager() == this);
  KASSERT(!task->is_terminated());

  DisableInterrupts disable_interrupts;

  if (!task->is_running())
    return;

  LOG_DEBUG("Pause the task pid={}", task->get_id());

  m_scheduler->remove_task(task);
  task->m_state = Task::State::UNINTERRUPTIBLE;
}

void TaskManager::wake_task(const TaskPtr& task) {
  KASSERT(task != nullptr);
  KASSERT(task->get_manager() == this);
  KASSERT(!task->is_terminated());

  DisableInterrupts disable_interrupts;

  if (task->is_running())
    return;

  LOG_DEBUG("Wake the task pid={}", task->get_id());

  m_scheduler->add_task(task);
  task->m_state = Task::State::RUNNING;
}

void TaskManager::kill_task(const TaskPtr& task, int exit_code) {
  KASSERT(task != nullptr);
  KASSERT(task->get_manager() == this);

  DisableInterrupts disable_interrupts;

  if (task->is_terminated())
    return;  // already killed

  LOG_DEBUG("Kill the task pid={} with status {}", task->get_id(), exit_code);

  // Propagate the kill to children. This is done recursively.
  auto it = task->children_begin();
  for (; it != task->children_end(); ++it) {
    auto child = *it;
    kill_task(child, exit_code);
  }

  m_scheduler->remove_task(task);
  task->m_state = Task::State::TERMINATED;
}

bool TaskManager::set_task_priority(const TaskPtr& task, uint32_t new_priority) {
  KASSERT(task != nullptr);
  KASSERT(!task->is_terminated());

  DisableInterrupts disable_interrupts;

  if (new_priority < Scheduler::MIN_PRIORITY || new_priority > Scheduler::MAX_PRIORITY)
    return false;

  const uint32_t old_priority = task->get_priority();
  task->m_priority = new_priority;
  m_scheduler->update_task_priority(task, old_priority);
  return true;
}

TaskPtr TaskManager::get_current_task() const {
  return m_scheduler->get_current_task();
}

void TaskManager::schedule() {
  DisableInterrupts disable_interrupts;
  m_scheduler->schedule();
}

void TaskManager::tick() {
  DisableInterrupts disable_interrupts;
  m_delta_queue.tick();
  m_scheduler->tick();
}
