#pragma once

#include "resource.hpp"
#include "wait_list.hpp"

#include <libk/vector.hpp>
#include <cstdint>

/**
 * @brief Represents a pipe resource owned by a task or multiple tasks.
 *
 * A pipe is a I/O device that has a read end and a write end. One task
 * can write to it whereas another task can read from it. The kernel will
 * buffer data written to the pipe until it is read from a task.
 *
 * Of course, both ends of a pipe can be used by a single task. However,
 * there is pratical use for it (this just add kernel overhead where you
 * just use a buffer in the task's memory).
 *
 * Pipes are used to implement standard inputs, outputs and errors devices
 * (what the C standard call stdin, stdout and stderr).
 *
 * The pipe is implemented as a circular buffer. The buffer has a fixed size
 * specified at creation time.
 *
 * Example usage:
 * @code{.cpp}
 * // For reading:
 * if (pipe->wait_read(task)) {
 *    const size_t read_bytes = pipe->read(buffer, count);
 * }
 *
 * // For writing:
 * if (pipe->wait_write(task)) {
 *   const size_t written_bytes = pipe->write(buffer, count);
 * }
 * @endcode
 */
class PipeResource : public Resource {
public:
  explicit PipeResource(size_t capacity);
  ~PipeResource() override;

  /** Checks if this pipe is closed. */
  [[nodiscard]] bool is_closed() const { return m_parent == nullptr; }

  /** Gets the size of the buffer (count of bytes that can be read). */
  [[nodiscard]] size_t get_size() const { return m_size; }
  /** Returns the capacity allowed for the internal buffer, in bytes. */
  [[nodiscard]] size_t get_capacity() const { return m_capacity; }

  /** Checks if there is some bytes to read from this pipe (at least one byte). */
  [[nodiscard]] bool can_read() const { return m_size > 0; }
  /** Checks if there is some space to write into this pipe (at least one byte). */
  [[nodiscard]] bool can_write() const { return m_size < m_capacity; }

  /** If some data can be read, returns @c true and does nothing. Otherwise,
   * adds the task to the read wait list and returns @c false. */
  bool wait_read(const libk::SharedPointer<Task>& task);
  /** If some data can be written, returns @c true and does nothing. Otherwise,
   * adds the task to the write wait list and returns @c false. */
  bool wait_write(const libk::SharedPointer<Task>& task);

  /** Reads up to @c count bytes from the pipe into the buffer.
   * Returns the number of bytes read. */
  size_t read(uint8_t* buffer, size_t count);
  /** Writes up to @c count bytes from the buffer into the pipe.
   * Returns the number of bytes written. */
  size_t write(const uint8_t* buffer, size_t count);

private:
  /** Closes this pipe and free the internal buffer. The pipe resource object
   * is not deleted nor its reference count decremented. For that, use deref(). */
  void close();

private:
  friend class Task;
  // The parent task that created this pipe. Multiple tasks may reference the pipe
  // but one task "owns" it. If parent is nullptr, the pipe is closed and
  // all read and write operations will fail.
  Task* m_parent = nullptr;
  // The name of the pipe (used to identify it). The name is unique inside the
  // parent task.
  const char* m_name = nullptr;

  // Wait list to store tasks waiting for write operations
  // (e.g. when there is no more capacity in the pipe).
  WaitList m_write_wait_list;
  // Wait list to store tasks waiting for read operations
  // (e.g. when there is no data to read from the pipe).
  WaitList m_read_wait_list;

  // The following variables are used to implement the circular buffer that stores
  // the buffered data of a pipe:

  // General information about the buffer and its content:
  // Internal buffer of the pipe of size m_capacity.
  uint8_t* m_buffer = nullptr;
  // Maximum capacity of the internal buffer, size of m_data array.
  size_t m_capacity = 0;
  // Current size of the internal buffer, in bytes.
  size_t m_size = 0;

  // The head and tail of the circular buffer:
  // Index of the next byte to read from the buffer.
  size_t m_read_index = 0;
  // Index of the next byte to write to the buffer.
  size_t m_write_index = 0;
}; // class PipeResource

