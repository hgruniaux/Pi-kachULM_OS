#pragma once

#include <cstddef>

namespace libk {
template <class T>
class Vector {
public:

private:
  T* m_data = nullptr;
  size_t m_capacity = 0;
  size_t m_size = 0;
}; // class Vector
} // namespace libk
