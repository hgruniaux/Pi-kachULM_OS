#pragma once

#include <cstdint>

namespace libk {
namespace detail {
template <class T>
struct OptionStorage {
  bool has_value_flag;
  union {
    T value;
    char dummy;
  };

  OptionStorage() : has_value_flag(false), dummy() {}
  OptionStorage(const T& value) : has_value_flag(true), value(value) {}
  ~OptionStorage() {
    if (has_value_flag)
      value.~T();
  }

  [[nodiscard]] bool has_value() const { return has_value_flag; }
};

template <class T>
struct OptionStorage<T*> {
  T* value;

  OptionStorage() : value(nullptr) {}
  OptionStorage(T* value) : value(value) {}
  ~OptionStorage() = default;

  [[nodiscard]] bool has_value() const { return value != nullptr; }
};

template <>
struct OptionStorage<bool> {
  static constexpr uint8_t NONE_VALUE = 2;

  union {
    bool value;
    uint8_t int_value;
  };

  OptionStorage() : int_value(NONE_VALUE) {}
  OptionStorage(bool value) : value(value) {}
  ~OptionStorage() = default;

  [[nodiscard]] bool has_value() const { return int_value != NONE_VALUE; }
};
}  // namespace detail

/**
 * @brief An Option interface to store conditionally a value.
 */
template <class T>
class Option {
 public:
  Option() : m_storage() {}
  Option(const T& value) : m_storage(value) {}

  /** @brief Checks if this option stores a value. */
  [[nodiscard]] bool has_value() const { return m_storage.has_value(); }

  [[nodiscard]] const T& get_value() const& {
    KASSERT(has_value());
    return m_storage.value;
  }

  [[nodiscard]] T& get_value() & {
    KASSERT(has_value());
    return m_storage.value;
  }

  [[nodiscard]] const T&& get_value() const&& {
    KASSERT(has_value());
    return std::move(m_storage.value);
  }

  [[nodiscard]] T&& get_value() && {
    KASSERT(has_value());
    return std::move(m_storage.value);
  }

 private:
  detail::OptionStorage<T> m_storage;
};  // class Option<T>

// No extra space for pointers.
static_assert(sizeof(Option<void*>) == sizeof(void*));
static_assert(sizeof(Option<int*>) == sizeof(int*));
// No extra space for bool.
static_assert(sizeof(Option<bool>) == sizeof(bool));
}  // namespace libk
