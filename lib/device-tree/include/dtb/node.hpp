#pragma once

#include <iterator>
#include <libk/option.hpp>
#include <libk/string_view.hpp>

#include "parser.hpp"

class DeviceTree;

class Node;

struct Property {
  libk::StringView name;
  size_t length;
  const char* data;

  /** @brief Tests if this property is likely a string */
  [[nodiscard]] bool is_string() const;

  /** Tries to parse an `<u32>` at *byte index* @a index into @a value.
   * @returns - `true` if @a value contains the parsed integer, and @a index the next index to use for parsing
   *          - `false` if the value can't be parsed, @a index and @a value are not modified.  */
  [[nodiscard]] bool get_u32_at(size_t* index, uint64_t* value) const;

  /** Tries to parse an `<u32>` at *byte index* @a index into @a value.
   * @returns - `true` if @a value contains the parsed integer, and @a index the next index to use for parsing
   *          - `false` if the value can't be parsed, @a index and @a value are not modified.  */
  [[nodiscard]] bool get_u64_at(size_t* index, uint64_t* value) const;

  /** Tries to parse an `<u32>` or an `<u64>` depending @a is_u64_integer at *byte index* @a index into @a value.
   * @returns - `true` if @a value contains the parsed integer, and @a index the next index to use for parsing
   *          - `false` if the value can't be parsed, @a index and @a value are not modified.  */
  [[nodiscard]] bool get_variable_int(size_t* index, uint64_t* value, bool is_u64_integer) const;

  /** @brief Parses the property value as a `<u32>`. */
  [[nodiscard]] libk::Option<uint32_t> get_u32() const;
  /** @brief Parses the property value as a `<u64>`. */
  [[nodiscard]] libk::Option<uint64_t> get_u64() const;
  /** @brief Parses the property value as a `<u32>` or `<u64>` depending on property length. */
  [[nodiscard]] libk::Option<uint64_t> get_u32_or_u64() const;
  /** @brief Parses the property value as a `<string>`. */
  [[nodiscard]] libk::StringView get_string() const { return {data, length}; }
};

class PropertyIterator {
 public:
  using iterator_category = std::forward_iterator_tag;
  using element_type = Property;
  using difference_type = std::ptrdiff_t;

  PropertyIterator() = default;
  explicit PropertyIterator(const DeviceTreeParser* parser, size_t offset);
  explicit PropertyIterator(const DeviceTreeParser* parser);

  element_type operator*() const;

  PropertyIterator& operator++();
  PropertyIterator operator++(int) {
    PropertyIterator old = *this;
    ++(*this);
    return old;
  }

  bool operator==(const PropertyIterator& b) const { return m_p == b.m_p && m_off == b.m_off; }

 private:
  const DeviceTreeParser* m_p;
  size_t m_off;
};

class NodeIterator {
 public:
  using iterator_category = std::forward_iterator_tag;
  using element_type = Node;
  using difference_type = std::ptrdiff_t;

  NodeIterator() = default;
  explicit NodeIterator(const DeviceTreeParser* parser, size_t offset);
  explicit NodeIterator(const DeviceTreeParser* parser);

  element_type operator*() const;

  NodeIterator& operator++();
  NodeIterator operator++(int) {
    NodeIterator old = *this;
    ++(*this);
    return old;
  }

  bool operator==(const NodeIterator& b) const { return m_p == b.m_p && m_off == b.m_off; }

 private:
  const DeviceTreeParser* m_p;
  size_t m_off;
};

class Node {
 public:
  class Properties {
   public:
    [[nodiscard]] PropertyIterator begin() const { return PropertyIterator(m_p, m_begin_offset); }
    [[nodiscard]] PropertyIterator end() const { return PropertyIterator(m_p); }

   protected:
    friend Node;
    explicit Properties(const DeviceTreeParser* parser, const size_t begin_offset)
        : m_p(parser), m_begin_offset(begin_offset) {}

    const DeviceTreeParser* m_p;
    const size_t m_begin_offset;
  };

  class Children {
   public:
    [[nodiscard]] NodeIterator begin() const { return NodeIterator(m_p, m_begin_offset); }
    [[nodiscard]] NodeIterator end() const { return NodeIterator(m_p); }

   protected:
    friend Node;
    explicit Children(const DeviceTreeParser* parser, const size_t begin_offset)
        : m_p(parser), m_begin_offset(begin_offset) {}

    const DeviceTreeParser* m_p;
    const size_t m_begin_offset;
  };

  Node() = default;

  [[nodiscard]] bool find_child(libk::StringView child_name, Node* child) const {
    for (const Node n : get_children()) {
      if (n.get_name() == child_name) {
        *child = n;
        return true;
      }
    }

    return false;
  }


  [[nodiscard]] bool find_property(libk::StringView property_name, Property* property) const {
    for (const Property p : get_properties()) {
      if (p.name == property_name) {
        *property = p;
        return true;
      }
    }

    return false;
  }

  [[nodiscard]] Properties get_properties() const { return Properties(m_p, m_off); }
  [[nodiscard]] Children get_children() const { return Children(m_p, m_off); }

  [[nodiscard]] libk::StringView get_name() const { return m_name; }

 protected:
  friend DeviceTree;
  friend NodeIterator;
  explicit Node(const DeviceTreeParser* parser, size_t offset);

  const DeviceTreeParser* m_p = nullptr;
  const char* m_name = "";
  size_t m_off = 0;
};

static_assert(std::forward_iterator<PropertyIterator>);
static_assert(std::forward_iterator<NodeIterator>);
