#ifndef TULIP_EVENT_HPP
#define TULIP_EVENT_HPP

#include "utils.hpp"

class TuEvent {
 public:
  enum class Type : uint8_t {
    PAINT,
    MOVE,
    RESIZE,
    FOCUS_IN,
    FOCUS_OUT,
    MOUSE_MOVE,
    MOUSE_DOWN,
    MOUSE_UP,
    KEY_DOWN,
    KEY_UP,
  };  // enum class Type

  explicit TuEvent(Type type) : m_type(type) {}

  [[nodiscard]] Type get_type() const { return m_type; }

 private:
  Type m_type;
};  // class TuEvent

class TuPaintEvent : public TuEvent {
 public:
  explicit TuPaintEvent(TuRect rect) : TuEvent(Type::PAINT), m_rect(rect) {}

  [[nodiscard]] TuRect get_rect() const { return m_rect; }

 private:
  TuRect m_rect;
};  // class TuPaintEvent

class TuMoveEvent : public TuEvent {
 public:
  TuMoveEvent(TuPoint old_position, TuPoint new_position)
      : TuEvent(Type::MOVE), m_old_position(old_position), m_new_position(new_position) {}

  /** Returns the old position, in pixels. */
  [[nodiscard]] TuPoint get_old_position() const { return m_old_position; }
  /** Returns the new position, in pixels. */
  [[nodiscard]] TuPoint get_new_position() const { return m_new_position; }

 private:
  TuPoint m_old_position;
  TuPoint m_new_position;
};  // class TuMoveEvent

class TuResizeEvent : public TuEvent {
 public:
  TuResizeEvent(TuSize old_size, TuSize new_size) : TuEvent(Type::RESIZE), m_old_size(old_size), m_new_size(new_size) {}

  /** Returns the old size, in pixels. */
  [[nodiscard]] TuSize get_old_size() const { return m_old_size; }
  /** Returns the new size, in pixels. */
  [[nodiscard]] TuSize get_new_size() const { return m_new_size; }

 private:
  TuSize m_old_size;
  TuSize m_new_size;
};  // class TuResizeEvent

class TuMouseEvent : public TuEvent {
 public:
  TuPoint position;
};  // class TuMouseEvent

class TuKeyEvent : public TuEvent {};  // class TuKeyEvent

#endif  // TULIP_EVENT_HPP
