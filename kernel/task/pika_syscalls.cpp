#include "pika_syscalls.hpp"

#include <syscall/syscall.h>
#include <libk/assert.hpp>
#include <libk/log.hpp>

#include "graphics/graphics.hpp"
#include "task.hpp"
#include "task_manager.hpp"

static void set_error(Registers& regs, sys_error_t error) {
  regs.x0 = error;
}

static void step_back_one_inst(Registers& regs) {
  // See aarch64 documentation about ELR_EL1 register, IL bit.
  const bool is_32bits_inst = (regs.esr & (1 << 25)) != 0;
  if (is_32bits_inst)
    regs.elr -= 4;  // 32-bits instruction
  else
    regs.elr -= 2;  // 16-bits instruction
}

static void pika_sys_unknown(Registers& regs) {
  set_error(regs, SYS_ERR_UNKNOWN_SYSCALL);
}

static void pika_sys_exit(Registers& regs) {
  const auto exit_code = (int)regs.x0;
  TaskManager::get().kill_task(Task::current(), exit_code);
  // This syscall does not return to the user process, no need to set the error code.
}

static void pika_sys_sleep(Registers& regs) {
  const auto time_in_us = regs.x0;

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
  const auto* msg = (const char*)regs.x0;
  // FIXME: check if pointer is accessible by current task
  libk::print("sys_print() from pid={}: {}", Task::current()->get_id(), msg);
  set_error(regs, SYS_ERR_OK);
}

static void pika_sys_spawn(Registers& regs) {
  set_error(regs, SYS_ERR_OK);
}

static void pika_sys_getpid(Registers& regs) {
  auto task = Task::current();
  const sys_pid_t pid = Task::current()->get_id();
  regs.x0 = pid;
}

static void pika_sys_sched_set_priority(Registers& regs) {
  //  const sys_pid_t pid = regs.x0;  // Unused
  const uint32_t priority = regs.x1;

  // TODO: use pid to set priority of another process
  if (TaskManager::get().set_task_priority(Task::current(), priority)) {
    set_error(regs, SYS_ERR_OK);
    return;
  }

  set_error(regs, SYS_ERR_INVALID_PRIORITY);
}

static void pika_sys_sched_get_priority(Registers& regs) {
  //  const sys_pid_t pid = regs.x0;  // Unused
  uint32_t* priority = (uint32_t*)regs.x1;
  // FIXME: check if pointer is accessible by current task

  // TODO: use pid to set priority of another process
  *priority = Task::current()->get_priority();
  set_error(regs, SYS_ERR_OK);
}

static void pika_sys_poll_msg(Registers& regs) {
  sys_msg_t* msg = (sys_msg_t*)regs.x0;
  // FIXME: check if pointer is accessible by current task

  MessageQueue& queue = Task::current()->get_message_queue();

  if (queue.dequeue(*msg))
    set_error(regs, SYS_ERR_OK);
  else
    set_error(regs, SYS_ERR_MSG_QUEUE_EMPTY);
}

static void pika_sys_wait_msg(Registers& regs) {
  sys_msg_t* msg = (sys_msg_t*)regs.x0;
  // FIXME: check if pointer is accessible by current task

  auto current_task = Task::current();
  MessageQueue& queue = current_task->get_message_queue();

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

static void pika_sys_draw_line(Registers& regs) {
  graphics::Painter painter;
  painter.draw_line(regs.x0, regs.x1, regs.x2, regs.x3, graphics::Color(regs.x4));
  set_error(regs, SYS_ERR_OK);
}

static void pika_sys_draw_rect(Registers& regs) {
  graphics::Painter painter;
  painter.draw_rect(regs.x0, regs.x1, regs.x2, regs.x3, graphics::Color(regs.x4));
  set_error(regs, SYS_ERR_OK);
}

static void pika_sys_fill_rect(Registers& regs) {
  graphics::Painter painter;
  painter.fill_rect(regs.x0, regs.x1, regs.x2, regs.x3, graphics::Color(regs.x4));
  set_error(regs, SYS_ERR_OK);
}

static void pika_sys_draw_text(Registers& regs) {
  graphics::Painter painter;
  const auto* msg = (const char*)regs.x2;
  painter.draw_text(regs.x0, regs.x1, msg, graphics::Color(regs.x3));
  set_error(regs, SYS_ERR_OK);
}

SyscallTable* create_pika_syscalls() {
  SyscallTable* table = new SyscallTable;
  KASSERT(table != nullptr);

  table->set_unknown_callback(pika_sys_unknown);
  table->register_syscall(SYS_EXIT, pika_sys_exit);
  table->register_syscall(SYS_SLEEP, pika_sys_sleep);
  table->register_syscall(SYS_YIELD, pika_sys_yield);
  table->register_syscall(SYS_PRINT, pika_sys_print);
  table->register_syscall(SYS_SPAWN, pika_sys_spawn);
  table->register_syscall(SYS_GETPID, pika_sys_getpid);
  table->register_syscall(SYS_SCHED_SET_PRIORITY, pika_sys_sched_set_priority);
  table->register_syscall(SYS_SCHED_GET_PRIORITY, pika_sys_sched_get_priority);

  table->register_syscall(SYS_POLL_MSG, pika_sys_poll_msg);
  table->register_syscall(SYS_WAIT_MSG, pika_sys_wait_msg);

  table->register_syscall(SYS_DEBUG, [](Registers& regs) {
    libk::print("Debug: {} from pid={}", regs.x0, Task::current()->get_id());
    set_error(regs, SYS_ERR_OK);
  });

  // GFX syscalls
  table->register_syscall(SYS_GFX_DRAW_LINE, pika_sys_draw_line);
  table->register_syscall(SYS_GFX_DRAW_RECT, pika_sys_draw_rect);
  table->register_syscall(SYS_GFX_FILL_RECT, pika_sys_fill_rect);
  table->register_syscall(SYS_GFX_DRAW_TEXT, pika_sys_draw_text);

  return table;
}
