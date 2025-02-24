#include "pika_syscalls.hpp"

#include <sys/file.h>
#include <sys/syscall.h>
#include <sys/window.h>

#include <libk/assert.hpp>
#include <libk/log.hpp>

#include "fs/filesystem.hpp"
#include "task/task.hpp"
#include "task/task_manager.hpp"
#include "wm/window.hpp"
#include "wm/window_manager.hpp"
#include "hardware/framebuffer.hpp"
#include "task/pipe.hpp"

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
  // TODO: Missing API in ProcessMemory to check if address is mapped (of the given size)
  // We need to check if the pointer points to a large enough region, we are missing a SIZE parameter in this function.
  (void)needs_write;
  (void)regs;

  return ptr != nullptr;
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

static void pika_sys_spawn(Registers& regs) {
  const auto* path = (const char*)regs.gp_regs.x0;
  const size_t argc = (size_t)regs.gp_regs.x1;
  const auto** argv = (const char**)regs.gp_regs.x2;

  // Check arguments
  if (!check_ptr(regs, (void*)path))
    return;
  if (!check_ptr(regs, (void*)argv))
    return;

  for (size_t i = 0; i < argc; ++i) {
    if (!check_ptr(regs, (void*)argv[i]))
      return;
  }

  auto task = TaskManager::get().create_task(path, Task::current().get());
  if (task == nullptr) {
    set_error(regs, SYS_ERR_GENERIC);
    return;
  }

  auto args_chunk = task->map_chunk(1, SYS_ARGS_ADDRESS, false, true);
  if (args_chunk == nullptr) {
    set_error(regs, SYS_ERR_OUT_OF_MEM);
    return;
  }

  // Write the arguments in the new task memory
  // Structure of arguments in args_chunk:
  //   [     ARGC     ]   -> UINT64
  //   [    ARGV[0]   ]   -> UINT64, points to the end of argv
  //          ...
  //   [ ARGV[ARGC-1] ]   -> UINT64,
  //   [ Data of ARGV[0] ] -> NUL terminated string
  //          ...
  //   [ Data of ARGV[ARGC-1] ] -> NUL terminated string

  (void)args_chunk->write(0, &argc, sizeof(argc));

  size_t argv_size = sizeof(uintptr_t) * (argc + 1);
  size_t argv_data_offset = size_t(argc) + argv_size;
  for (size_t i = 0; i < argc; ++i) {
    const char* data = argv[i];
    size_t data_len = libk::strlen(data) + 1;
    (void)args_chunk->write(argv_data_offset, data, data_len);
    uintptr_t argv_addr = SYS_ARGS_ADDRESS + argv_data_offset;
    (void)args_chunk->write(sizeof(argc) + sizeof(uintptr_t) * i, &argv_addr, sizeof(argv_addr));
    argv_data_offset += data_len;
  }

  LOG_TRACE("spawned new task pid={} from path={}", task->get_id(), path);

  TaskManager::get().wake_task(task);
  set_error(regs, SYS_ERR_OK);
}

static void pika_sys_sched_set_priority(Registers& regs) {
  // FIXME: What is the first parameter (PID of some other application)?
  // const sys_pid_t pid = regs.gp_regs.x0;  // Unused
  const uint32_t priority = regs.gp_regs.x1;

  // TODO: use pid to set priority of another process
  if (TaskManager::get().set_task_priority(Task::current(), priority)) {
    set_error(regs, SYS_ERR_OK);
    return;
  }

  set_error(regs, SYS_ERR_INVALID_PRIORITY);
}

static void pika_sys_sched_get_priority(Registers& regs) {
  // FIXME: What is the first parameter (PID of some other application)?
  // const sys_pid_t pid = regs.gp_regs.x0;  // Unused
  uint32_t* priority = (uint32_t*)regs.gp_regs.x1;
  if (!check_ptr(regs, (void*)priority, /* needs_write= */ true))
    return;

  // TODO: use pid to set priority of another process
  *priority = Task::current()->get_priority();
  set_error(regs, SYS_ERR_OK);
}

static bool check_file(Registers& regs, File* file) {
  if (Task::current()->own_file(file))
    return true;

  set_error(regs, SYS_ERR_INVALID_FILE);
  return false;
}

static void pika_sys_open_file(Registers& regs) {
  const char* path = (const char*)regs.gp_regs.x0;
  if (!check_ptr(regs, (void*)path))
    return;

  const sys_file_mode_t mode = (sys_file_mode_t)regs.gp_regs.x1;
  auto* file = FileSystem::get().open(path, mode);
  regs.gp_regs.x0 = (sys_word_t)file;
  if (file == nullptr)
    return;

  Task::current()->register_file(file);
}

static void pika_sys_close_file(Registers& regs) {
  File* file = (File*)regs.gp_regs.x0;
  if (!check_file(regs, file))
    return;

  FileSystem::get().close(file);
  Task::current()->unregister_file(file);
  set_error(regs, SYS_ERR_OK);
}

static void pika_sys_read_file(Registers& regs) {
  File* file = (File*)regs.gp_regs.x0;
  if (!check_file(regs, file))
    return;

  void* buffer = (void*)regs.gp_regs.x1;
  size_t* read_bytes = (size_t*)regs.gp_regs.x3;
  if (!check_ptr(regs, buffer) || !check_ptr(regs, read_bytes))
    return;

  size_t bytes_to_read = regs.gp_regs.x2;
  bool success = file->read(buffer, bytes_to_read, read_bytes);
  if (success)
    set_error(regs, SYS_ERR_OK);
  else
    set_error(regs, SYS_ERR_GENERIC);
}

static void pika_sys_get_file_size(Registers& regs) {
  File* file = (File*)regs.gp_regs.x0;
  if (!check_file(regs, file))
    return;

  regs.gp_regs.x0 = file->get_size();
}

static bool check_dir(Registers& regs, Dir* dir) {
  if (Task::current()->own_dir(dir))
    return true;

  set_error(regs, SYS_ERR_INVALID_DIR);
  return false;
}

static void pika_sys_open_dir(Registers& regs) {
  const char* path = (const char*)regs.gp_regs.x0;
  if (!check_ptr(regs, (void*)path))
    return;

  auto* dir = FileSystem::get().open_dir(path);
  regs.gp_regs.x0 = (sys_word_t)dir;
  if (dir == nullptr)
    return;

  Task::current()->register_dir(dir);
}

static void pika_sys_close_dir(Registers& regs) {
  Dir* dir = (Dir*)regs.gp_regs.x0;
  if (!check_dir(regs, dir))
    return;

  FileSystem::get().close_dir(dir);
  Task::current()->unregister_dir(dir);
  set_error(regs, SYS_ERR_OK);
}

static void pika_sys_read_dir(Registers& regs) {
  Dir* dir = (Dir*)regs.gp_regs.x0;
  if (!check_dir(regs, dir))
    return;

  sys_file_info_t* file_info = (sys_file_info_t*)regs.gp_regs.x1;
  if (!check_dir(regs, dir))
    return;

  if (dir->read(file_info))
    set_error(regs, SYS_ERR_OK);
  else
    set_error(regs, SYS_ERR_GENERIC);
}

static bool check_window(Registers& regs, Window* window) {
  if (Task::current()->own_window(window))
    return true;

  set_error(regs, SYS_ERR_INVALID_WINDOW);
  return false;
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

  WindowManager::get().set_window_geometry(window, x, y, width, height);
  set_error(regs, SYS_ERR_OK);
}

static void pika_sys_window_present(Registers& regs) {
  auto* window = (Window*)regs.gp_regs.x0;
  if (!check_window(regs, window))
    return;

  const uint32_t x = (uint32_t)regs.gp_regs.x1;
  const uint32_t y = (uint32_t)regs.gp_regs.x2;
  const uint32_t width = (uint32_t)regs.gp_regs.x3;
  const uint32_t height = (uint32_t)regs.gp_regs.x4;

  WindowManager::get().present_window(window, x, y, width, height);
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

static void pika_sys_get_framebuffer(Registers& reg) {
  void** pixels = (void**)reg.gp_regs.x0;
  uint32_t* width = (uint32_t*)reg.gp_regs.x1;
  uint32_t* height = (uint32_t*)reg.gp_regs.x2;
  uint32_t* stride = (uint32_t*)reg.gp_regs.x3;

  if (pixels != nullptr && !check_ptr(reg, pixels, true))
    return;
  if (width != nullptr && !check_ptr(reg, width, true))
    return;
  if (height != nullptr && !check_ptr(reg, height, true))
    return;
  if (stride != nullptr && !check_ptr(reg, stride, true))
    return;

  auto& fb = FrameBuffer::get();

  // Trivial properties to get from the framebuffer.
  if (width != nullptr)
    *width = fb.get_width();
  if (height != nullptr)
    *height = fb.get_width();
  if (stride != nullptr)
    *stride = fb.get_pitch();

  // If the user wan the pixels buffer, then we need to map it in its memory space.
  if (pixels != nullptr) {
    // FIXME: Map the framebuffer physical address to an address in userspace
    auto* pixels_chunk = Task::current()->map_chunk(1, 0, false, true);
    if (pixels_chunk == nullptr) {
      set_error(reg, SYS_ERR_OUT_OF_MEM);
      return;
    }

    // Write the framebuffer buffer in the user memory.
    (void)pixels_chunk->write(0, fb.get_buffer(), fb.get_width() * fb.get_height() * sizeof(uint32_t));
    *pixels = (void*)0;
  }

  set_error(reg, SYS_ERR_OK);
}

static void pika_sys_gfx_blit(Registers& regs) {
  auto* window = (Window*)regs.gp_regs.x0;
  if (!check_window(regs, window))
    return;

  uint32_t x, y;
  uint32_t width, height;
  unpack_couple(regs.gp_regs.x1, x, y);
  unpack_couple(regs.gp_regs.x2, width, height);

  const uint32_t* argb_buffer = (const uint32_t*)regs.gp_regs.x3;
  if (!check_ptr(regs, (void*)argb_buffer))
    return;

  window->blit(x, y, width, height, argb_buffer);
  set_error(regs, SYS_ERR_OK);
}

/*
 * Pipe system calls.
 */

// Signature: sys_pipe_t* sys_pipe_open(const char* name);
static void pika_sys_pipe_open(Registers& regs) {
  const char* name = (const char*)regs.gp_regs.x0;
  if (!check_ptr(regs, (void*)name))
    return; // FIXME: Check if name is not too long (see SYS_PIPE_NAME_MAX).

  auto current_task = Task::current();
  KASSERT(current_task != nullptr);
  auto* pipe = current_task->create_pipe(name);
  regs.gp_regs.x0 = (sys_word_t)pipe;
}

// Signature: sys_error_t sys_pipe_close(sys_pipe_t* pipe);
static void pika_sys_pipe_close(Registers& regs) {
  auto current_task = Task::current();
  KASSERT(current_task != nullptr);

  auto* pipe = (PipeResource*)regs.gp_regs.x0;
  if (!current_task->own_pipe(pipe)) {
    set_error(regs, SYS_ERR_INVALID_PIPE);
    return;
  }

  current_task->unregister_pipe(pipe);
  set_error(regs, SYS_ERR_OK);
}

// Signature: sys_pipe_t* sys_pipe_get(sys_pid_t pid, const char* name);
static void pika_sys_pipe_get(Registers& regs) {
  const sys_pid_t pid = regs.gp_regs.x0;
  const char* name = (const char*)regs.gp_regs.x1;
  if (!check_ptr(regs, (void*)name))
    return; // FIXME: Check if name is not too long (see SYS_PIPE_NAME_MAX).

  TaskPtr task = TaskManager::get().find_by_id(pid);
  if (task == nullptr) {
    // Task not found, PID invalid.
    regs.gp_regs.x0 = 0; // NULL pointer as return value
    return;
  }

  auto pipe = task->get_pipe(name);
  regs.gp_regs.x0 = (sys_word_t)pipe;
}

// Signature: sys_error_t sys_pipe_read(sys_pipe_t* pipe, void* buffer, size_t size, size_t* read_bytes);
static void pika_sys_pipe_read(Registers& regs) {
  auto current_task = Task::current();
  KASSERT(current_task != nullptr);

  auto* pipe = (PipeResource*)regs.gp_regs.x0;
  if (!current_task->own_pipe(pipe)) {
    set_error(regs, SYS_ERR_INVALID_PIPE);
    return;
  }

  const size_t size = regs.gp_regs.x2;

  // FIXME: Check if buffer is at least 'size' bytes long.
  uint8_t* buffer = (uint8_t*)regs.gp_regs.x1;
  if (!check_ptr(regs, buffer, true))
    return;

  size_t* read_bytes = (size_t*)regs.gp_regs.x3;
  if (read_bytes != nullptr && !check_ptr(regs, read_bytes, true))
    return;

  // TODO: Allow non-blocking pipes
  if (!pipe->wait_read(current_task)) {
    // The pipe is empty, we need to block the task.
    // Step back at the SVC instruction, so the next time this task
    // is scheduled, the system call is resubmitted.
    step_back_one_inst(regs);
    return;
  }

  const size_t read_bytes_value = pipe->read(buffer, size);
  if (read_bytes != nullptr)
    *read_bytes = read_bytes_value;
  set_error(regs, SYS_ERR_OK);
}

// Signature: sys_error_t sys_pipe_write(sys_pipe_t* pipe, const void* buffer, size_t size, size_t* written_bytes);
static void pika_sys_pipe_write(Registers& regs) {
  auto current_task = Task::current();
  KASSERT(current_task != nullptr);

  auto* pipe = (PipeResource*)regs.gp_regs.x0;
  if (!current_task->own_pipe(pipe)) {
    set_error(regs, SYS_ERR_INVALID_PIPE);
    return;
  }

  const size_t size = regs.gp_regs.x2;

  // FIXME: Check if buffer is at least 'size' bytes long.
  const uint8_t* buffer = (const uint8_t*)regs.gp_regs.x1;
  if (!check_ptr(regs, (void*)buffer))
    return;

  size_t* written_bytes = (size_t*)regs.gp_regs.x3;
  if (written_bytes != nullptr && !check_ptr(regs, written_bytes, true))
    return;

  // TODO: Allow non-blocking pipes
  if (!pipe->wait_write(current_task)) {
    // The pipe is full, we need to block the task.
    // Step back at the SVC instruction, so the next time this task
    // is scheduled, the system call is resubmitted.
    step_back_one_inst(regs);
    return;
  }

  const size_t written_bytes_value = pipe->write(buffer, size);
  if (written_bytes != nullptr)
    *written_bytes = written_bytes_value;
  set_error(regs, SYS_ERR_OK);
}

SyscallTable* create_pika_syscalls() {
  SyscallTable* table = new SyscallTable;
  KASSERT(table != nullptr);

  table->set_unknown_callback(pika_sys_unknown);
  table->register_syscall(SYS_EXIT, pika_sys_exit);
  table->register_syscall(SYS_PRINT, pika_sys_print);
  table->register_syscall(SYS_GETPID, pika_sys_getpid);
  table->register_syscall(SYS_SPAWN, pika_sys_spawn);
  table->register_syscall(SYS_DEBUG, [](Registers& regs) {
    libk::print("Debug: {} from pid={}", regs.gp_regs.x0, Task::current()->get_id());
    set_error(regs, SYS_ERR_OK);
  });

  // Memory system calls.
  table->register_syscall(SYS_SBRK, pika_sys_sbrk);

  // Framebuffer system calls.
  table->register_syscall(SYS_GET_FRAMEBUFFER, pika_sys_get_framebuffer);

  // Scheduler system calls.
  table->register_syscall(SYS_SLEEP, pika_sys_sleep);
  table->register_syscall(SYS_YIELD, pika_sys_yield);
  table->register_syscall(SYS_SCHED_SET_PRIORITY, pika_sys_sched_set_priority);
  table->register_syscall(SYS_SCHED_GET_PRIORITY, pika_sys_sched_get_priority);

  // Pipe system calls.
  table->register_syscall(SYS_PIPE_OPEN, pika_sys_pipe_open);
  table->register_syscall(SYS_PIPE_CLOSE, pika_sys_pipe_close);
  table->register_syscall(SYS_PIPE_GET, pika_sys_pipe_get);
  table->register_syscall(SYS_PIPE_READ, pika_sys_pipe_read);
  table->register_syscall(SYS_PIPE_WRITE, pika_sys_pipe_write);

  // File system calls.
  table->register_syscall(SYS_OPEN_FILE, pika_sys_open_file);
  table->register_syscall(SYS_CLOSE_FILE, pika_sys_close_file);
  table->register_syscall(SYS_READ_FILE, pika_sys_read_file);
  table->register_syscall(SYS_GET_FILE_SIZE, pika_sys_get_file_size);
  table->register_syscall(SYS_OPEN_DIR, pika_sys_open_dir);
  table->register_syscall(SYS_CLOSE_DIR, pika_sys_close_dir);
  table->register_syscall(SYS_READ_DIR, pika_sys_read_dir);

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
  table->register_syscall(SYS_GFX_BLIT, pika_sys_gfx_blit);

  return table;
}
