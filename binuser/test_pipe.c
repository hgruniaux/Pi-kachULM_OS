#include <sys/syscall.h>
#include <string.h>

int main() {
  sys_pipe_t* pipe = sys_pipe_open("test_pipe");
  if (pipe == NULL) {
    sys_print("Failed to open the pipe");
    return 1;
  }

  const char* msg = "Hello, world!";
  size_t written = 0;
  sys_error_t err = sys_pipe_write(pipe, msg, strlen(msg), &written);
  if (SYS_IS_OK(err) && written == strlen(msg)) {
    sys_print("Message sent successfully");
    return 0;
  }

  sys_print("Failed to send the message");
  return 0;
}
