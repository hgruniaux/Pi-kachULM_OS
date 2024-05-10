#pragma once

#include "assert.hpp"

namespace libk {
template <class T>
class LinkedList {
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
    if (m_tail == nullptr) {
      m_head = node;
      m_tail = node;
    } else {
      insert_before(m_head, node);
    }
  }

  void push_back(const T& data) {
    Node* node = new Node{data};
    KASSERT(node != nullptr);
    if (m_tail == nullptr) {
      m_head = node;
      m_tail = node;
    } else {
      insert_after(m_tail, node);
    }
  }

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
};  // class LinkedList
}  // namespace libk
