#pragma once

#include "assert.hpp"

namespace libk {
template <class T>
class ScopedPointer {
 public:
  ScopedPointer(T* data = nullptr) : m_data(data) {}
  ~ScopedPointer() {
    if (m_data != nullptr)
      delete m_data;
  }

  // No copy
  ScopedPointer(const ScopedPointer&) = delete;
  ScopedPointer& operator=(const ScopedPointer&) = delete;

  // Move
  ScopedPointer(ScopedPointer&& other) : m_data(other.m_data) { other.m_data = nullptr; }
  ScopedPointer& operator=(ScopedPointer&& other) {
    m_data = other.m_data;
    other.m_data = nullptr;
    return *this;
  }

  [[nodiscard]] T* get() const { return m_data; }
  [[nodiscard]] operator bool() const { return m_data != nullptr; }
  [[nodiscard]] bool operator!() const { return m_data == nullptr; }
  [[nodiscard]] T& operator*() const { return *m_data; }
  [[nodiscard]] T* operator->() const { return m_data; }

 private:
  T* m_data;
};  // class ScopedPointer
}  // namespace libk
