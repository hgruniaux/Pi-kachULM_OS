#pragma once

#include <cstdint>
#include <libk/assert.hpp>

class Task;

/**
 * This is a base class for all resources that can be owned by a task.
 *
 * Some examples of resources are:
 * - File descriptors
 * - Windows
 * - Mutexes
 * - etc.
 */
class Resource {
 public:
  virtual ~Resource()
  {
    KASSERT(m_ref_count == 0);
  }

  /** Increments the ref count. */
  void ref() { ++m_ref_count; }
  /** Decrements the ref count by one. If the reference count reach 0, the object
   * is freed using @c delete. */
  void unref() {
    KASSERT(m_ref_count > 0);
    --m_ref_count;
    if (m_ref_count == 0)
      delete this;
  }

 private:
  uint32_t m_ref_count = 0;
};  // class Resource
