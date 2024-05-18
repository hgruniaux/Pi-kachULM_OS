// Author: Hubert Gruniaux
// Date: 5/16/24
// The following code is released in the public domain (where applicable).

#include "window.hpp"

Window::Window(const libk::SharedPointer<Task>& task) : m_task(task) {
  KASSERT(task != nullptr);
}

void Window::set_title(libk::StringView title) {
  delete[] m_title.get_data();

  // Clamp the title length, so the user cannot abuse from the kernel.
  const size_t length = libk::min(title.get_length(), MAX_TITLE_LENGTH);

  char* buffer = new char[length + 1];
  libk::memcpy(buffer, title.get_data(), length);
  buffer[length] = '\0';
  m_title = {buffer, length};
}
