#pragma once

#include <cstddef>
#include <utility>

#include "assert.hpp"

namespace libk {
template <class T>
class ScopedPointer {
 public:
  ScopedPointer() : m_data(nullptr) {}
  ScopedPointer(std::nullptr_t) : m_data(nullptr) {}
  template <class U>
  explicit ScopedPointer(U* data = nullptr) : m_data(data) {}
  ~ScopedPointer() { reset(); }

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

  void reset() {
    if (m_data != nullptr)
      delete m_data;
    m_data = nullptr;
  }

  template <class U>
  void reset(U* ptr) {
    reset();
    m_data = ptr;
  }

  [[nodiscard]] T* get() const { return m_data; }
  [[nodiscard]] operator bool() const { return m_data != nullptr; }
  [[nodiscard]] bool operator!() const { return m_data == nullptr; }
  [[nodiscard]] T& operator*() const { return *m_data; }
  [[nodiscard]] T* operator->() const { return m_data; }

 private:
  T* m_data;
};  // class ScopedPointer

template <class T>
[[nodiscard]] bool operator==(const ScopedPointer<T>& lhs, std::nullptr_t) {
  return !lhs;
}

template <class T>
[[nodiscard]] bool operator==(const ScopedPointer<T>& lhs, const ScopedPointer<T>& rhs) {
  return lhs.get() == rhs.get();
}

template <class T, class... Args>
[[nodiscard]] ScopedPointer<T> make_scoped(Args&&... args) {
  T* ptr = new T(std::forward<Args>(args)...);
  return ScopedPointer<T>(ptr);
}

template <class T>
class SharedPointer {
 public:
  SharedPointer() : m_block(nullptr) {}
  SharedPointer(std::nullptr_t) : m_block(nullptr) {}
  template <class U>
  explicit SharedPointer(U* ptr) : m_block(nullptr) {
    reset(ptr);
  }
  ~SharedPointer() { reset(); }

  // Copy
  SharedPointer(const SharedPointer& other) : m_block(other.m_block) {
    if (m_block != nullptr)
      m_block->ref_count++;
  }
  SharedPointer& operator=(const SharedPointer& other) {
    if (m_block == other.m_block)  // check self-assignment
      return *this;

    reset();
    new (this) SharedPointer(other);
    return *this;
  }

  // Move
  SharedPointer(SharedPointer&& other) : m_block(other.m_block) { other.m_block = nullptr; }
  SharedPointer& operator=(SharedPointer&& other) {
    reset();
    m_block = other.m_block;
    other.m_block = nullptr;
    return *this;
  }

  void reset() {
    if (m_block == nullptr)
      return;

    m_block->ref_count--;
    if (m_block->ref_count == 0) {
      delete m_block->data;
      delete m_block;
    }
  }

  template <class U>
  void reset(U* ptr) {
    reset();
    m_block = new Block;
    m_block->data = ptr;
    m_block->ref_count++;
  }

  [[nodiscard]] T* get() const { return m_block != nullptr ? m_block->data : nullptr; }
  [[nodiscard]] operator bool() const { return m_block != nullptr; }
  [[nodiscard]] bool operator!() const { return m_block == nullptr; }
  [[nodiscard]] T& operator*() const { return *m_block->data; }
  [[nodiscard]] T* operator->() const { return m_block->data; }

 private:
  struct Block {
    T* data = nullptr;
    unsigned int ref_count = 0;
  };  // struct Block

  Block* m_block;
};  // class SharedPointer

template <class T>
[[nodiscard]] bool operator==(const SharedPointer<T>& lhs, std::nullptr_t) {
  return !lhs;
}

template <class T>
[[nodiscard]] bool operator==(const SharedPointer<T>& lhs, const SharedPointer<T>& rhs) {
  return lhs.get() == rhs.get();
}

template <class T, class... Args>
[[nodiscard]] SharedPointer<T> make_shared(Args&&... args) {
  T* ptr = new T(std::forward<Args>(args)...);
  return SharedPointer<T>(ptr);
}
}  // namespace libk
