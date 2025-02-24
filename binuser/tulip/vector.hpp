#ifndef TULIP_VECTOR_HPP
#define TULIP_VECTOR_HPP

#include <cstddef>

template <class T>
class TuVector {
 public:
  using iterator = T*;
  using const_iterator = const T*;

  TuVector() = default;
  ~TuVector() { delete[] m_data; }

  [[nodiscard]] bool is_empty() const { return m_size == 0; }
  [[nodiscard]] size_t get_size() const { return m_size; }
  [[nodiscard]] size_t get_capacity() const { return m_capacity; }

  void push_back(const T& value) {
    if (m_size == m_capacity) {
      reserve(m_capacity == 0 ? 1 : m_capacity * 2);
    }

    m_data[m_size++] = value;
  }

  void pop_back() {
    if (m_size == 0) {
      return;
    }

    --m_size;
  }

  [[nodiscard]] T& operator[](size_t index) { return m_data[index]; }
  [[nodiscard]] const T& operator[](size_t index) const { return m_data[index]; }

  void reserve(size_t new_capacity) {
    if (new_capacity <= m_capacity) {
      return;
    }

    T* new_data = new T[new_capacity];
    for (size_t i = 0; i < m_size; ++i) {
      new_data[i] = m_data[i];
    }

    delete[] m_data;
    m_data = new_data;
    m_capacity = new_capacity;
  }

  [[nodiscard]] iterator begin() { return m_data; }
  [[nodiscard]] const_iterator begin() const { return m_data; }
  [[nodiscard]] const_iterator cbegin() const { return m_data; }

  [[nodiscard]] iterator end() { return m_data + m_size; }
  [[nodiscard]] const_iterator end() const { return m_data + m_size; }
  [[nodiscard]] const_iterator cend() const { return m_data + m_size; }

 private:
  T* m_data = nullptr;
  size_t m_size = 0;
  size_t m_capacity = 0;
};  // class TuVector

#endif  // TULIP_VECTOR_HPP
