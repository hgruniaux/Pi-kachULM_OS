#include "application.hpp"
#include "window.hpp"

TuApplication::TuApplication() {
  s_instance = this;
}

TuApplication* TuApplication::get() {
  return s_instance;
}

int TuApplication::run() {
  while (m_windows != nullptr) {
    // Poll all system messages for all windows.
    TuWindow* window = m_windows;
    while (window != nullptr) {
      window->poll_system_messages();
      window = window->m_next;
    }
  }

  return 0;
}

void TuApplication::register_window(TuWindow* window) {
  window->m_next = m_windows;
  m_windows = window;
}

void TuApplication::unregister_window(TuWindow* window) {
  if (m_windows == window) {
    m_windows = window->m_next;
    return;
  }

  TuWindow* prev = m_windows;
  while (prev->m_next != window) {
    prev = prev->m_next;
  }

  prev->m_next = window->m_next;
}
