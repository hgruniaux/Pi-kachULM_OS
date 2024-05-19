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

  void clear() {
    Node* node = m_head;
    while (node != nullptr) {
      Node* next_node = node->next;
      delete node;
      node = next_node;
    }

    m_head = nullptr;
    m_tail = nullptr;
  }

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

  template <bool CONST>
  class BaseIterator {
    using NodeType = std::conditional_t<CONST, const Node, Node>;

   public:
    using iterator_category = std::bidirectional_iterator_tag;
    using element_type = std::conditional_t<CONST, const T, T>;
    using difference_type = std::ptrdiff_t;

    BaseIterator() = default;

    element_type& operator*() const { return m_node->data; }
    element_type* operator->() const { return &(m_node->data); }

    BaseIterator& operator++() {
      if (m_node != nullptr) {
        m_node = m_node->next;
      }

      return *this;
    }
    BaseIterator operator++(int) {
      auto* old_node = m_node;
      ++(*this);
      return Iterator(old_node);
    }

    BaseIterator& operator--() {
      if (m_node != nullptr) {
        m_node = m_node->previous;
      }

      return *this;
    }
    BaseIterator operator--(int) {
      auto* old_node = m_node;
      --(*this);
      return Iterator(old_node);
    }

    bool operator==(const BaseIterator& b) const { return m_node == b.m_node; }

   private:
    friend LinkedList;
    explicit BaseIterator(Node* node) : m_node(node){};

    NodeType* m_node = nullptr;
  };  // class BaseIterator

  using Iterator = BaseIterator<false>;
  using ConstIterator = BaseIterator<false>;
  using ReverseIterator = std::reverse_iterator<Iterator>;
  using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

  [[nodiscard]] Iterator begin() { return Iterator(m_head); }
  [[nodiscard]] ConstIterator begin() const { return Iterator(m_head); }
  [[nodiscard]] Iterator end() { return Iterator(nullptr); }
  [[nodiscard]] ConstIterator end() const { return Iterator(nullptr); }

  [[nodiscard]] ReverseIterator rbegin() { return std::make_reverse_iterator(Iterator(m_tail)); }
  [[nodiscard]] ConstReverseIterator rbegin() const { return std::make_reverse_iterator(Iterator(m_tail)); }
  [[nodiscard]] ReverseIterator rend() { return std::make_reverse_iterator(Iterator(nullptr)); }
  [[nodiscard]] ConstReverseIterator rend() const { return std::make_reverse_iterator(Iterator(nullptr)); }

  void insert_before(Iterator it, const T& data) {
    Node* node = new Node{data};
    KASSERT(node != nullptr);
    insert_before(it.m_node, node);
  }

  void insert_after(Iterator it, const T& data) {
    Node* node = new Node{data};
    KASSERT(node != nullptr);
    insert_after(it.m_node, node);
  }

  void erase(Iterator it) { remove_node(it.m_node); }
};  // class LinkedList
}  // namespace libk

static_assert(std::bidirectional_iterator<libk::LinkedList<int>::Iterator>);
