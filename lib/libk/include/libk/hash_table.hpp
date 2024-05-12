#pragma once

#include "hash.hpp"
#include "linked_list.hpp"

namespace libk {
template <class K, class T>
class HashTable {
 public:
 private:
  size_t m_bucket_count;
  libk::LinkedList<T>* m_buckets;
};  // class HashTable
}  // namespace libk
