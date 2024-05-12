#pragma once

#include <iterator>
#include "assert.hpp"

namespace libk {
template <class T>
class LinkedList {
 private:
  struct Node {
    T data;
    Node* next = nullptr;
    Node* previous = nullptr;
  };  // struct Node

  void insert_after(Node* node, Node* new_node) {
    new_node->previous = node;

    new_node->next = node->next;
    if (node->next == nullptr) {
      m_tail = new_node;
    } else {
      node->next->previous = new_node;
    }

    node->next = new_node;
  }

  void insert_back(Node* node) {
    if (m_tail == nullptr) {
      m_head = node;
      m_tail = node;
    } else {
      insert_after(m_tail, node);
    }
  }

  void insert_before(Node* node, Node* new_node) {
    new_node->next = node;

    new_node->previous = node->previous;
    if (node->previous == nullptr) {
      m_head = new_node;
    } else {
      node->previous->next = new_node;
    }

    node->previous = new_node;
  }

  void insert_front(Node* node) {
    if (m_tail == nullptr) {
      m_head = node;
      m_tail = node;
    } else {
      insert_before(m_head, node);
    }
  }

  void remove_node(Node* node) {
    if (node->previous == nullptr) {
      m_head = node->next;
    } else {
      node->previous->next = node->next;
    }

    if (node->next == nullptr) {
      m_tail = node->previous;
    } else {
      node->next->previous = node->previous;
    }

    delete node;
  }

  Node* m_head = nullptr;
  Node* m_tail = nullptr;

 public:
  [[nodiscard]] bool is_empty() const { return m_head == nullptr; }

  T pop_back() {
    KASSERT(!is_empty());
    T data = m_tail->data;
    remove_node(m_tail);
    return data;
  }

  T pop_front() {
    KASSERT(!is_empty());
    T data = m_head->data;
    remove_node(m_head);
    return data;
  }

  void push_front(const T& data) {
    Node* node = new Node{data};
    KASSERT(node != nullptr);
    insert_front(node);
  }

  void push_back(const T& data) {
    Node* node = new Node{data};
    KASSERT(node != nullptr);
    insert_back(node);
  }

  template <class... Args>
  T& emplace_back(Args&&... args) {
    Node* node = new Node{T(std::forward<Args>(args)...)};
    KASSERT(node != nullptr);
    insert_back(node);
    return node->data;
  }

  class Iterator {
   public:
    using iterator_category = std::bidirectional_iterator_tag;
    using element_type = T;
    using difference_type = std::ptrdiff_t;

    Iterator() = default;

    element_type& operator*() const { return m_node->data; }
    element_type* operator->() const { return &(m_node->data); }

    Iterator& operator++() {
      if (m_node != nullptr) {
        m_node = m_node->next;
      }

      return *this;
    }
    Iterator operator++(int) {
      Node* old_node = m_node;
      ++(*this);
      return Iterator(old_node);
    }

    Iterator& operator--() {
      if (m_node != nullptr) {
        m_node = m_node->previous;
      }

      return *this;
    }
    Iterator operator--(int) {
      Node* old_node = m_node;
      --(*this);
      return Iterator(old_node);
    }

    bool operator==(const Iterator& b) const { return m_node == b.m_node; }

   private:
    friend LinkedList;
    explicit Iterator(Node* node) : m_node(node){};

    Node* m_node = nullptr;
  };

  [[nodiscard]] Iterator begin() const { return Iterator(m_head); }
  [[nodiscard]] Iterator end() const { return Iterator(nullptr); }

  void erase(Iterator it) { remove_node(it.m_node); }
};  // class LinkedList
}  // namespace libk

static_assert(std::bidirectional_iterator<libk::LinkedList<int>::Iterator>);
