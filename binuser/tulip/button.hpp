#ifndef TULIP_BUTTON_HPP
#define TULIP_BUTTON_HPP

#include "widget.hpp"

class TuButton : public TuWidget {
 public:
  explicit TuButton(const char* label, TuWidget* parent = nullptr);

  [[nodiscard]] const char* get_label() const { return m_label; }
  void set_label(const char* label);

 protected:
  void on_paint_event(TuPaintEvent* event) override;

 private:
  const char* m_label;
  bool m_is_pressed = false;
  bool m_is_hovered = false;
};  // class TuButton

#endif  // TULIP_BUTTON_HPP
