#include "pika_syscalls.hpp"

#include <sys/syscall.h>
#include <sys/window.h>

#include <libk/assert.hpp>
#include <libk/log.hpp>

#include "task/task.hpp"
#include "task/task_manager.hpp"
#include "wm/window.hpp"
#include "wm/window_manager.hpp"

static void set_error(Registers& regs, sys_error_t error) {
  regs.gp_regs.x0 = error;
}

static void step_back_one_inst(Registers& regs) {
  // See aarch64 documentation about ELR_EL1 register, IL bit.
  const bool is_32bits_inst = (regs.esr & (1 << 25)) != 0;
  if (is_32bits_inst)
    regs.elr -= 4;  // 32-bits instruction
  else
    regs.elr -= 2;  // 16-bits instruction
}

static bool check_ptr(Registers& regs, void* ptr, bool needs_write = false) {
  // FIXME: check if pointer is accessible by current task
  return true;
}

static void pika_sys_unknown(Registers& regs) {
  set_error(regs, SYS_ERR_UNKNOWN_SYSCALL);
}

static void pika_sys_exit(Registers& regs) {
  const auto exit_code = (int)regs.gp_regs.x0;
  TaskManager::get().kill_task(Task::current(), exit_code);
  // This sys does not return to the user process, no need to set the error code.
}

static void pika_sys_sleep(Registers& regs) {
  const auto time_in_us = regs.gp_regs.x0;

  if (time_in_us > 0) {
    set_error(regs, SYS_ERR_OK);
    TaskManager::get().sleep_task(Task::current(), time_in_us);
    return;
  }

  set_error(regs, SYS_ERR_OK);
}

static void pika_sys_yield(Registers& regs) {
  TaskManager::get().schedule();
  set_error(regs, SYS_ERR_OK);
}

static void pika_sys_print(Registers& regs) {
  const auto* msg = (const char*)regs.gp_regs.x0;
  if (!check_ptr(regs, (void*)msg))
    return;

  libk::print("sys_print() from pid={}: {}", Task::current()->get_id(), msg);
  set_error(regs, SYS_ERR_OK);
}

static void pika_sys_getpid(Registers& regs) {
  auto task = Task::current();
  const sys_pid_t pid = Task::current()->get_id();
  regs.gp_regs.x0 = pid;
}

static void pika_sys_sbrk(Registers& regs) {
  const ptrdiff_t increment = regs.gp_regs.x0;
  auto task = Task::current();

  auto memory = task->get_memory();
  const auto previous_brk = memory->get_heap_end();
  task->get_memory()->change_heap_end(increment);
  regs.gp_regs.x0 = previous_brk;
}

static void pika_sys_sched_set_priority(Registers& regs) {
  //  const sys_pid_t pid = regs.gp_regs.x0;  // Unused
  const uint32_t priority = regs.gp_regs.x1;

  // TODO: use pid to set priority of another process
  if (TaskManager::get().set_task_priority(Task::current(), priority)) {
    set_error(regs, SYS_ERR_OK);
    return;
  }

  set_error(regs, SYS_ERR_INVALID_PRIORITY);
}

static void pika_sys_sched_get_priority(Registers& regs) {
  //  const sys_pid_t pid = regs.gp_regs.x0;  // Unused
  uint32_t* priority = (uint32_t*)regs.gp_regs.x1;
  if (!check_ptr(regs, (void*)priority, /* needs_write= */ true))
    return;

  // TODO: use pid to set priority of another process
  *priority = Task::current()->get_priority();
  set_error(regs, SYS_ERR_OK);
}

static bool check_window(Registers& regs, Window* window) {
  if (!WindowManager::get().is_valid(window) || window->get_task() != Task::current()) {
    set_error(regs, SYS_ERR_INVALID_WINDOW);
    return false;
  }

  return true;
}

static void pika_sys_poll_msg(Registers& regs) {
  auto* window = (Window*)regs.gp_regs.x0;
  if (!check_window(regs, window))
    return;

  auto* msg = (sys_message_t*)regs.gp_regs.x1;
  if (!check_ptr(regs, msg, true))
    return;

  MessageQueue& queue = window->get_message_queue();

  if (queue.dequeue(*msg))
    set_error(regs, SYS_ERR_OK);
  else
    set_error(regs, SYS_ERR_MSG_QUEUE_EMPTY);
}

static void pika_sys_wait_msg(Registers& regs) {
  auto* window = (Window*)regs.gp_regs.x0;
  if (!check_window(regs, window))
    return;

  auto* msg = (sys_message_t*)regs.gp_regs.x1;
  if (!check_ptr(regs, msg, true))
    return;

  auto current_task = Task::current();
  MessageQueue& queue = window->get_message_queue();

  if (queue.block_task_until_not_empty(current_task)) {
    // Task was blocked, the message queue is currently empty.
    // Step back at the SVC instruction, so the next time this task
    // is preempted, the system call is resubmitted.
    step_back_one_inst(regs);
    return;
  }

  if (queue.dequeue(*msg))
    set_error(regs, SYS_ERR_OK);
  else
    set_error(regs, SYS_ERR_INTERNAL);
}

static void pika_sys_window_create(Registers& regs) {
  const auto flags = regs.gp_regs.x0;
  auto* window = WindowManager::get().create_window(Task::current(), flags);
  regs.gp_regs.x0 = (sys_word_t)window;
}

static void pika_sys_window_destroy(Registers& regs) {
  auto* window = (Window*)regs.gp_regs.x0;
  if (!check_window(regs, window))
    return;

  set_error(regs, SYS_ERR_OK);
}

static void pika_sys_window_set_title(Registers& regs) {
  auto* window = (Window*)regs.gp_regs.x0;
  if (!check_window(regs, window))
    return;

  const char* title = (const char*)regs.gp_regs.x1;
  if (!check_ptr(regs, (void*)title))
    return;

  window->set_title(title);
  set_error(regs, SYS_ERR_OK);
}

static void pika_sys_window_get_visibility(Registers& regs) {
  auto* window = (Window*)regs.gp_regs.x0;
  if (!check_window(regs, window))
    return;

  bool* is_visible = (bool*)regs.gp_regs.x1;
  if (is_visible != nullptr) {
    if (!check_ptr(regs, is_visible, true))
      return;

    *is_visible = window->is_visible();
  }

  set_error(regs, SYS_ERR_OK);
}

static void pika_sys_window_set_visibility(Registers& regs) {
  auto* window = (Window*)regs.gp_regs.x0;
  if (!check_window(regs, window))
    return;

  const bool is_visible = regs.gp_regs.x1;
  WindowManager::get().set_window_visibility(window, is_visible);
  set_error(regs, SYS_ERR_OK);
}

static void pika_sys_window_get_geometry(Registers& regs) {
  auto* window = (Window*)regs.gp_regs.x0;
  if (!check_window(regs, window))
    return;

  int32_t* x = (int32_t*)regs.gp_regs.x1;
  int32_t* y = (int32_t*)regs.gp_regs.x2;
  uint32_t* width = (uint32_t*)regs.gp_regs.x3;
  uint32_t* height = (uint32_t*)regs.gp_regs.x4;

  const auto geometry = window->get_geometry();

  if (x != nullptr) {
    if (!check_ptr(regs, x, true))
      return;

    *x = geometry.x();
  }

  if (y != nullptr) {
    if (!check_ptr(regs, y, true))
      return;

    *y = geometry.y();
  }

  if (width != nullptr) {
    if (!check_ptr(regs, width, true))
      return;

    *width = geometry.width();
  }

  if (height != nullptr) {
    if (!check_ptr(regs, height, true))
      return;

    *height = geometry.height();
  }

  set_error(regs, SYS_ERR_OK);
}

static void pika_sys_window_set_geometry(Registers& regs) {
  auto* window = (Window*)regs.gp_regs.x0;
  if (!check_window(regs, window))
    return;

  const int32_t x = (int32_t)regs.gp_regs.x1;
  const int32_t y = (int32_t)regs.gp_regs.x2;
  const int32_t width = regs.gp_regs.x3;
  const int32_t height = regs.gp_regs.x4;

  WindowManager::get().set_window_geometry(window, Rect::from_pos_and_size(x, y, width, height));
  set_error(regs, SYS_ERR_OK);
}

static void pika_sys_window_present(Registers& regs) {
  auto* window = (Window*)regs.gp_regs.x0;
  if (!check_window(regs, window))
    return;

  WindowManager::get().present_window(window);
  set_error(regs, SYS_ERR_OK);
}

static void pika_sys_gfx_clear(Registers& regs) {
  auto* window = (Window*)regs.gp_regs.x0;
  if (!check_window(regs, window))
    return;

  const uint32_t argb = regs.gp_regs.x1;
  window->clear(argb);
  set_error(regs, SYS_ERR_OK);
}

static void unpack_couple(uint64_t reg, uint32_t& fst, uint32_t& snd) {
  fst = reg & UINT32_MAX;
  snd = (reg >> 32) & UINT32_MAX;
}

static void pika_sys_gfx_draw_line(Registers& regs) {
  auto* window = (Window*)regs.gp_regs.x0;
  if (!check_window(regs, window))
    return;

  uint32_t x0, y0, x1, y1;
  unpack_couple(regs.gp_regs.x1, x0, y0);
  unpack_couple(regs.gp_regs.x2, x1, y1);

  const uint32_t argb = regs.gp_regs.x3;

  window->draw_line(x0, y0, x1, y1, argb);
  set_error(regs, SYS_ERR_OK);
}

static void pika_sys_gfx_draw_rect(Registers& regs) {
  auto* window = (Window*)regs.gp_regs.x0;
  if (!check_window(regs, window))
    return;

  uint32_t x, y, width, height;
  unpack_couple(regs.gp_regs.x1, x, y);
  unpack_couple(regs.gp_regs.x2, width, height);

  const uint32_t argb = regs.gp_regs.x3;

  window->draw_rect(x, y, width, height, argb);
  set_error(regs, SYS_ERR_OK);
}

static void pika_sys_gfx_fill_rect(Registers& regs) {
  auto* window = (Window*)regs.gp_regs.x0;
  if (!check_window(regs, window))
    return;

  uint32_t x, y, width, height;
  unpack_couple(regs.gp_regs.x1, x, y);
  unpack_couple(regs.gp_regs.x2, width, height);

  const uint32_t argb = regs.gp_regs.x3;

  window->fill_rect(x, y, width, height, argb);
  set_error(regs, SYS_ERR_OK);
}

static void pika_sys_gfx_draw_text(Registers& regs) {
  auto* window = (Window*)regs.gp_regs.x0;
  if (!check_window(regs, window))
    return;

  uint32_t x, y;
  unpack_couple(regs.gp_regs.x1, x, y);

  const char* text = (const char*)regs.gp_regs.x2;
  if (!check_ptr(regs, (void*)text))
    return;

  const uint32_t argb = regs.gp_regs.x3;

  window->draw_text(x, y, text, argb);
  set_error(regs, SYS_ERR_OK);
}

SyscallTable* create_pika_syscalls() {
  SyscallTable* table = new SyscallTable;
  KASSERT(table != nullptr);

  table->set_unknown_callback(pika_sys_unknown);
  table->register_syscall(SYS_EXIT, pika_sys_exit);
  table->register_syscall(SYS_PRINT, pika_sys_print);
  table->register_syscall(SYS_GETPID, pika_sys_getpid);
  table->register_syscall(SYS_DEBUG, [](Registers& regs) {
    libk::print("Debug: {} from pid={}", regs.gp_regs.x0, Task::current()->get_id());
    set_error(regs, SYS_ERR_OK);
  });

  // Memory system calls.
  table->register_syscall(SYS_SBRK, pika_sys_sbrk);

  // Scheduler system calls.
  table->register_syscall(SYS_SLEEP, pika_sys_sleep);
  table->register_syscall(SYS_YIELD, pika_sys_yield);
  table->register_syscall(SYS_SCHED_SET_PRIORITY, pika_sys_sched_set_priority);
  table->register_syscall(SYS_SCHED_GET_PRIORITY, pika_sys_sched_get_priority);

  // Window manager system calls.
  table->register_syscall(SYS_POLL_MESSAGE, pika_sys_poll_msg);
  table->register_syscall(SYS_WAIT_MESSAGE, pika_sys_wait_msg);
  table->register_syscall(SYS_WINDOW_CREATE, pika_sys_window_create);
  table->register_syscall(SYS_WINDOW_DESTROY, pika_sys_window_destroy);
  table->register_syscall(SYS_WINDOW_SET_TITLE, pika_sys_window_set_title);
  table->register_syscall(SYS_WINDOW_GET_VISIBILITY, pika_sys_window_get_visibility);
  table->register_syscall(SYS_WINDOW_SET_VISIBILITY, pika_sys_window_set_visibility);
  table->register_syscall(SYS_WINDOW_GET_GEOMETRY, pika_sys_window_get_geometry);
  table->register_syscall(SYS_WINDOW_SET_GEOMETRY, pika_sys_window_set_geometry);

  // Window graphics calls.
  table->register_syscall(SYS_WINDOW_PRESENT, pika_sys_window_present);
  table->register_syscall(SYS_GFX_CLEAR, pika_sys_gfx_clear);
  table->register_syscall(SYS_GFX_DRAW_LINE, pika_sys_gfx_draw_line);
  table->register_syscall(SYS_GFX_DRAW_RECT, pika_sys_gfx_draw_rect);
  table->register_syscall(SYS_GFX_FILL_RECT, pika_sys_gfx_fill_rect);
  table->register_syscall(SYS_GFX_DRAW_TEXT, pika_sys_gfx_draw_text);

  return table;
}
