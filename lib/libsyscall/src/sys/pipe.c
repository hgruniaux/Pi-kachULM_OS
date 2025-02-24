#include <sys/syscall.h>
#include <assert.h>

sys_pipe_t* sys_pipe_open(const char* name) {
  assert(name != NULL);
  assert(strlen(name) < SYS_PIPE_NAME_MAX);
  return (sys_pipe_t*)__syscall1(SYS_PIPE_OPEN, (sys_word_t)name);
}

sys_error_t sys_pipe_close(sys_pipe_t* pipe) {
  assert(pipe != NULL);
  return __syscall1(SYS_PIPE_CLOSE, (sys_word_t)pipe);
}

sys_pipe_t* sys_pipe_get(sys_pid_t pid, const char* name) {
  assert(name != NULL);
  assert(strlen(name) < SYS_PIPE_NAME_MAX);
  return (sys_pipe_t*)__syscall2(SYS_PIPE_GET, (sys_word_t)pid, (sys_word_t)name);
}

sys_error_t sys_pipe_read(sys_pipe_t* pipe, void* buffer, size_t size, size_t* read) {
  assert(pipe != NULL);
  assert(size == 0 || buffer != NULL);
  return __syscall4(SYS_PIPE_READ, (sys_word_t)pipe, (sys_word_t)buffer, size, (sys_word_t)read);
}

sys_error_t sys_pipe_write(sys_pipe_t* pipe, const void* buffer, size_t size, size_t* written) {
  assert(pipe != NULL);
  assert(size == 0 || buffer != NULL);
  return __syscall4(SYS_PIPE_WRITE, (sys_word_t)pipe, (sys_word_t)buffer, size, (sys_word_t)written);
}
