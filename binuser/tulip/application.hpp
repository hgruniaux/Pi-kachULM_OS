#ifndef TU_APPLICATION_HPP
#define TU_APPLICATION_HPP

#include "style.hpp"

class TuWindow;

class TuApplication {
public:
  TuApplication();

  static TuApplication* get();

  [[nodiscard]] TuStyle* get_style() { return &m_style; }

  int run();

  // Automatically called by TuWindow constructor and destructor.
  void register_window(TuWindow* window);
  void unregister_window(TuWindow* window);

private:
  static inline TuApplication* s_instance = nullptr;

  // Linked list of all active windows (follow the TuWindow::m_next field).
  TuWindow* m_windows = nullptr;
  TuStyle m_style = TuStyle::default_dark_style();
}; // class TuApplication

#endif // TU_APPLICATION_HPP

