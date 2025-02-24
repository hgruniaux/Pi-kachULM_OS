#include "pipe.hpp"

#include <libk/utils.hpp>
#include <libk/string.hpp>

PipeResource::PipeResource(size_t capacity) : m_capacity(capacity) {
  m_buffer = new uint8_t[capacity];
}

PipeResource::~PipeResource() {
  close();
}

bool PipeResource::wait_read(const libk::SharedPointer<Task>& task) {
  if (can_read()) return true; // No need to wait.
  if (is_closed()) return false; // The pipe is closed, no more wait lists.
  m_read_wait_list.add(task);
  return false;
}

bool PipeResource::wait_write(const libk::SharedPointer<Task>& task) {
  if (can_write()) return true; // No need to wait.
  if (is_closed()) return false; // The pipe is closed, no more wait lists.
  m_write_wait_list.add(task);
  return false;
}

size_t PipeResource::read(uint8_t* buffer, size_t count) {
  if (is_closed()) return 0; // The pipe is closed.

  // Limit the read count to the size of the buffer.
  count = libk::min(count, m_size);
  if (count == 0) return 0; // nothing.

  // There is two scenarios here:
  // 1. The read operation is trivial because there is enough of contiguous data to read from.
  //      [ |                     XXXXXXXXXXXX  ]
  //        ^- write_index        ^- read_index
  // 2. The read operation is not trivial because the data is fragmented in the buffer.
  //      [XXX    |                  XXXXXXXXXXX]
  //       ^~~    ^- write_index     ^- read_index
  //       |- still read data

  if (m_read_index + count <= m_capacity) {
    // Scenario 1: The read operation is trivial because data is contiguous.
    libk::memcpy(buffer, m_buffer + m_read_index, count);
  } else {
    // Scenario 2: The read operation is not trivial because data is fragmented.
    const size_t first_part = m_capacity - m_read_index;
    libk::memcpy(buffer, m_buffer + m_read_index, first_part);
    libk::memcpy(buffer + first_part, m_buffer, count - first_part);
  }

  // Update the read index.
  m_read_index += count;
  if (m_read_index >= m_capacity)
    m_read_index -= m_capacity;

  m_size -= count;

  // Yeah! We have more free space in the buffer! Wake tasks that
  // are waiting for write operations.
  m_write_wait_list.wake_all();
  return count;
}

size_t PipeResource::write(const uint8_t* buffer, size_t count) {
  if (is_closed()) return 0; // The pipe is closed.

  // Limit the write count to the free space in the buffer.
  count = libk::min(count, m_capacity - m_size);
  if (count == 0) return 0; // nothing.

  // There is two scenarios here:
  // 1. The write operation is trivial because there is enough of contiguous space to write to.
  //      [ |                     XXXXXXXXXXXX  ]
  //        ^- read_index        ^- write_index
  // 2. The write operation is not trivial because the space is fragmented in the buffer.
  //      [XXX    |                  XXXXXXXXXXX]
  //       ^~~    ^- read_index     ^- write_index
  //       |- still write data

  if (m_write_index + count <= m_capacity) {
    // Scenario 1: The write operation is trivial because space is contiguous.
    libk::memcpy(m_buffer + m_write_index, buffer, count);
  } else {
    // Scenario 2: The write operation is not trivial because space is fragmented.
    const size_t first_part = m_capacity - m_write_index;
    libk::memcpy(m_buffer + m_write_index, buffer, first_part);
    libk::memcpy(m_buffer, buffer + first_part, count - first_part);
  }

  // Update the write index.
  m_write_index += count;
  if (m_write_index >= m_capacity)
    m_write_index -= m_capacity;

  m_size += count;

  // Yeah! We have more data in the buffer! Wake tasks that
  // are waiting for read operations.
  m_read_wait_list.wake_all();
  return count;
}

void PipeResource::close() {
  m_parent = nullptr;
  delete[] m_buffer;
  m_buffer = nullptr;
  m_capacity = 0;
  m_size = 0;
  m_read_index = 0;
  m_write_index = 0;

  // Wake all tasks that are waiting for read or write operations
  // to avoid deadlocks.
  m_read_wait_list.wake_all();
  m_write_wait_list.wake_all();
}
