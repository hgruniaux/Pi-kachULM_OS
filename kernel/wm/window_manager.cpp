#include "wm/window_manager.hpp"
#include "graphics/graphics.hpp"
#include "hardware/framebuffer.hpp"
#include "hardware/timer.hpp"
#include "input/mouse_input.hpp"
#include "libk/log.hpp"
#include "task/task_manager.hpp"
#include "wm/window.hpp"

#include <algorithm>

#ifdef CONFIG_USE_DMA
#include "hardware/dma/request.hpp"
#endif  // CONFIG_USE_DMA

#include "graphics/stb_image.h"

#ifdef CONFIG_HAS_CURSOR
static constexpr uint32_t CURSOR_DATA[] = {
    0xff000000, 0x00000100, 0x00000000, 0x00000000, 0x00000100, 0x00010000, 0x00000000, 0x00000000, 0x00000000,
    0xff000000, 0xff000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000001, 0x00000100, 0x00010000,
    0xff000001, 0xfffffeff, 0xff000000, 0x00000000, 0x00000100, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xff000000, 0xffffffff, 0xfffeffff, 0xff000000, 0x00000100, 0x00010000, 0x00000001, 0x00000100, 0x00010000,
    0xff010000, 0xfffffeff, 0xffffffff, 0xfffffffe, 0xff010001, 0x00000000, 0x00000001, 0x00000100, 0x00000000,
    0xff010001, 0xfffffeff, 0xfffeffff, 0xffffffff, 0xfffffeff, 0xff000000, 0x00000001, 0x00000000, 0x00010000,
    0xff010001, 0xfffffeff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xff010000, 0x00000000, 0x00010000,
    0xff000101, 0xfffffeff, 0xfffeffff, 0xffffffff, 0xfffffeff, 0xffffffff, 0xffffffff, 0xff000100, 0x00010000,
    0xff000001, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xfffeffff, 0xfffffffe, 0xfffffeff, 0xff000000,
    0xff000000, 0xfffffeff, 0xfffeffff, 0xfffffffe, 0xfffffeff, 0xfffeffff, 0xff000000, 0xff000100, 0xff010000,
    0xff000100, 0xffffffff, 0xfffeffff, 0xff000001, 0xffffffff, 0xfffeffff, 0xff000000, 0x00000000, 0x00000000,
    0xff000000, 0xffffffff, 0xff010000, 0x00000001, 0xff000000, 0xfffeffff, 0xfffffffe, 0xff000100, 0x00010000,
    0xff000000, 0xff000000, 0x00000000, 0x00000001, 0xff000100, 0xfffeffff, 0xfffffffe, 0xff000100, 0x00010000,
    0x00000000, 0x00000100, 0x00000000, 0x00000000, 0x00000000, 0xff000000, 0xffffffff, 0xfffffeff, 0xff010000,
    0x00000001, 0x00000000, 0x00000000, 0x00000001, 0x00000000, 0xff000000, 0xfffffffe, 0xffffffff, 0xff010000,
    0x00000001, 0x00000100, 0x00000000, 0x00000000, 0x00000000, 0x00010000, 0xff000000, 0xff000100, 0x00000000};

static constexpr uint32_t CURSOR_HEIGHT = 16;
static constexpr uint32_t CURSOR_WIDTH = 9;
#endif // CONFIG_HAS_CURSOR

WindowManager* WindowManager::g_instance = nullptr;

WindowManager::WindowManager() {
  KASSERT(g_instance == nullptr && "multiple window manager created");
  g_instance = this;

  auto& fb = FrameBuffer::get();
  m_is_supported = fb.is_initialized();
  if (m_is_supported) {
    m_screen_buffer = fb.get_buffer();
    m_screen_width = fb.get_width();
    m_screen_height = fb.get_height();
    m_screen_pitch = fb.get_pitch();
    m_screen_buffer = fb.get_buffer();

#ifdef CONFIG_USE_DMA
    m_screen_buffer_dma_addr = DMA::get_dma_bus_address((VirtualAddress)m_screen_buffer, false);
#endif  // CONFIG_USE_DMA
  }

  read_wallpaper();
  update();
}

[[nodiscard]] bool WindowManager::is_valid(Window* window) const {
  if (window == nullptr)
    return false;

  const auto it = std::find(m_windows.begin(), m_windows.end(), window);
  return it != m_windows.end();
}

Window* WindowManager::create_window(const libk::SharedPointer<Task>& task, uint32_t flags) {
  if (!m_is_supported)
    return nullptr;

  Window* window = new Window(task);
  if (window == nullptr)
    return nullptr;

  if ((flags & SYS_WF_NO_FRAME) != 0)
    window->m_has_frame = false;

  task->register_window(window);
  ++m_window_count;
  m_windows.push_back(window);

  // Focus the window.
  focus_window(window);

  return window;
}

void WindowManager::destroy_window(Window* window) {
  KASSERT(is_valid(window));

  window->get_task()->unregister_window(window);

  if (m_focus_window == window) {
    // Unfocus the window.
    unfocus_window(window);
  }

  const auto it = std::find(m_windows.begin(), m_windows.end(), window);
  m_windows.erase(it);

  --m_window_count;
  if (window->is_visible())
    update(window->get_geometry());
  delete window;

  // Focus another window if needed.
  if (!m_windows.is_empty())
    focus_window(m_windows.back());
}

void WindowManager::set_window_visibility(Window* window, bool visible) {
  KASSERT(is_valid(window));

  if (window->is_visible() == visible)
    return;  // already the correct visibility

  window->m_visible = visible;

  bool updated = false;
  // Update the focus window if needed.
  if (visible) {  // show the window
    if (m_focus_window == nullptr) {
      focus_window(window);
      updated = true;
    }
  } else {  // hide the window
    if (m_focus_window == window) {
      unfocus_window(window);
      updated = true;
    }
  }

  // Send message.
  sys_message_t message;
  libk::bzero(&message, sizeof(sys_message_t));
  message.id = visible ? SYS_MSG_SHOW : SYS_MSG_HIDE;
  post_message(window, message);

  if (!updated && visible)
    update(window->get_geometry());
}

void WindowManager::set_window_geometry(Window* window, Rect rect) {
  KASSERT(is_valid(window));

  // Enforce constraints on window size.
  rect.set_width(libk::clamp(rect.width(), Window::MIN_WIDTH, libk::min(Window::MAX_WIDTH, m_screen_width)));
  rect.set_height(libk::clamp(rect.height(), Window::MIN_HEIGHT, libk::min(Window::MAX_HEIGHT, m_screen_height)));
  const uint32_t width = rect.width();
  const uint32_t height = rect.height();

  // Enforce constraints on window position.
  rect.x1 = (libk::clamp(rect.x(), 0, m_screen_width - rect.width()));
  rect.y1 = (libk::clamp(rect.y(), 0, m_screen_height - rect.height()));
  rect.x2 = rect.x1 + width;
  rect.y2 = rect.y1 + height;

  const auto old_rect = window->get_geometry();

  window->set_geometry(rect);

  const bool moved = old_rect.x() != rect.x() || old_rect.y() != rect.y();
  const bool resized = old_rect.width() != rect.width() || old_rect.height() != rect.height();

  // Send move and resize messages.
  if (moved) {
    sys_message_t message;
    libk::bzero(&message, sizeof(sys_message_t));
    message.id = SYS_MSG_MOVE;
    message.param1 = rect.x();
    message.param2 = rect.y();
    post_message(window, message);
  }

  if (resized) {
    sys_message_t message;
    libk::bzero(&message, sizeof(sys_message_t));
    message.id = SYS_MSG_RESIZE;
    message.param1 = rect.width();
    message.param2 = rect.height();
    post_message(window, message);
  }

  if (moved || resized)
    update(old_rect.union_with(rect));
}

void WindowManager::set_window_geometry(Window* window, int32_t x, int32_t y, int32_t w, int32_t h) {
  KASSERT(is_valid(window));

  // Handle special values.
  if (x == SYS_POS_DEFAULT) {
    x = m_last_window_x;
    m_last_window_x += 50;
  } else if (x == SYS_POS_CENTERED) {
    x = m_screen_width / 2 - w / 2;
  }

  if (y == SYS_POS_DEFAULT) {
    y = m_last_window_y;
    m_last_window_y += 50;
  } else if (y == SYS_POS_CENTERED) {
    y = m_screen_height / 2 - h / 2;
  }

  set_window_geometry(window, Rect::from_pos_and_size(x, y, w, h));
}

void WindowManager::focus_window(Window* window) {
  KASSERT(is_valid(window));

  if (m_focus_window == window)
    return;

  // Unfocus the previous window.
  if (m_focus_window != nullptr) {
    unfocus_window(m_focus_window);
  }

  // Send focus in message.
  sys_message_t message;
  libk::bzero(&message, sizeof(sys_message_t));
  message.id = SYS_MSG_FOCUS_IN;
  post_message(window, message);

  m_focus_window = window;
  m_focus_window->m_focus = true;

  auto it = std::find(m_windows.begin(), m_windows.end(), window);
  KASSERT(it != m_windows.end());
  m_windows.erase(it);
  m_windows.push_front(window);
  update(m_focus_window->get_geometry());
}

void WindowManager::unfocus_window(Window* window) {
  KASSERT(is_valid(window));

  if (m_focus_window != window)
    return;

  if (m_focus_window != nullptr) {
    m_focus_window->m_focus = false;

    // Send focus out messsage.
    sys_message_t message;
    libk::bzero(&message, sizeof(sys_message_t));
    message.id = SYS_MSG_FOCUS_OUT;
    post_message(m_focus_window, message);
  }

  Window* new_focus_window = nullptr;
  for (auto* w : m_windows) {
    // TODO: give focus to the nearest window (according to depth).
    if (w != window && w->is_visible())
      new_focus_window = w;
  }

  m_focus_window = nullptr;

  if (new_focus_window == nullptr)
    return;

  focus_window(new_focus_window);
}

void WindowManager::post_message(sys_message_t message) {
  if (!m_is_supported)
    return;

  // Handle window manager specific shortcuts.
  if (message.id == SYS_MSG_KEYDOWN) {
    if (handle_key_event(message.param1))
      return;  // the key event was handled by the window manager, do not propagate it.
  }

#ifdef CONFIG_HAS_CURSOR
  if (message.id == SYS_MSG_MOUSEMOVE) {
    if (handle_mouse_move_event(message.param1, message.param2))
      return;  // the mouse move event was handled by the window manager, do not propagate it.
  } else if (message.id == SYS_MSG_MOUSECLICK) {
    if (handle_mouse_click_event(message.param1, message.param2))
      return;  // the mouse click event was handled by the window manager, do not propagate it.
  }
#endif // CONFIG_HAS_CURSOR

  // Propagate the message to the focus window.
  if (m_focus_window != nullptr) {
    post_message(m_focus_window, message);
  }
}

bool WindowManager::post_message(Window* window, sys_message_t message) {
  KASSERT(is_valid(window));

  message.timestamp = GenericTimer::get_elapsed_time_in_ms();
  return window->get_message_queue().enqueue(message);
}

void WindowManager::present_window(Window* window) {
  KASSERT(is_valid(window));

  // Redraw the window.
  update(window->get_geometry());
}

void WindowManager::present_window(Window* window, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
  KASSERT(is_valid(window));

  const auto window_rect = window->get_geometry();
  x = libk::min<uint32_t>(x, window_rect.width());
  y = libk::min<uint32_t>(y, window_rect.height());

  width = libk::min<uint32_t>(width, window_rect.width() - x);
  height = libk::min<uint32_t>(height, window_rect.height() - y);

  // Redraw the window.
  update(Rect::from_pos_and_size(window_rect.x() + x, window_rect.y() + y, width, height));
}

void WindowManager::mosaic_layout() {
  if (m_window_count == 0)
    return;

  // Let nb_columns be ceil(sqrt(m_window_count)).
  int nb_columns = libk::isqrt(m_window_count);  // isqrt returns the floor value
  if ((size_t)(nb_columns) * (size_t)(nb_columns) != m_window_count)
    nb_columns += 1;  // convert it into ceil(sqrt(...))

  const int nb_rows = (int)libk::div_round_up(m_window_count, nb_columns);

  const int window_width = m_screen_width / nb_columns;
  const int window_height = m_screen_height / nb_rows;

  int current_idx = 0;
  int i = 0, j = 0;
  for (auto* window : m_windows) {
    const auto x = i * window_width;
    const auto y = j * window_height;

    int32_t width = window_width;
    // If we are the last window, take all the remaining place in the last row.
    const bool is_last = (size_t)(current_idx + 1) == m_window_count;
    if (is_last) {
      width = (nb_columns - i) * window_width;
    }

    set_window_geometry(window, Rect::from_pos_and_size(x, y, width, window_height));

    ++i;
    if (i >= nb_columns) {
      i = 0;
      ++j;
    }

    ++current_idx;
  }
}

void WindowManager::update() {
  update({0, 0, m_screen_width, m_screen_height});
}

void WindowManager::update(const Rect& rect) {
  if (!m_is_supported)
    return;

  DMARequestQueue request_queue{};

  // Draw the windows and the background.
  draw_windows(rect, request_queue);

#ifdef CONFIG_USE_DMA
  request_queue.execute_and_wait(m_dma_channel);
#endif  // CONFIG_USE_DMA

#ifdef CONFIG_HAS_CURSOR
  // Draw the cursor.
  auto cursor_rect = Rect::from_pos_and_size(m_cursor_x, m_cursor_y, CURSOR_WIDTH, CURSOR_HEIGHT);
  clip_rect_to_screen(cursor_rect);

  if (cursor_rect.intersects(rect))
    draw_cursor();
#endif // CONFIG_HAS_CURSOR
}

void WindowManager::clip_rect_to_screen(Rect& rect) {
  rect.x1 = libk::max(0, rect.x1);
  rect.y1 = libk::max(0, rect.y1);
  rect.x2 = libk::min(m_screen_width, rect.x2);
  rect.y2 = libk::min(m_screen_height, rect.y2);
}

void WindowManager::draw_background(const Rect& rect, DMARequestQueue& request_queue) {
  if (rect.is_null())
    return;

  if (m_wallpaper == nullptr) {
    // No wallpaper found. Fill the background.
    // TODO
    return;
  }

#if defined(CONFIG_USE_DMA) && defined(CONFIG_USE_DMA_FOR_WALLPAPER)
  const auto framebuffer_dma_addr = m_wallpaper->get_dma_address();
  const auto src_stride = 0;
  const auto dst_stride = sizeof(uint32_t) * (m_screen_pitch - m_screen_width);
  auto* request =
      DMA::Request::memcpy_2d(framebuffer_dma_addr, m_screen_buffer_dma_addr, sizeof(uint32_t) * (m_wallpaper_width),
                              m_wallpaper_height, src_stride, dst_stride);
  request_queue.add(request);
#else
  (void)request_queue;

  for (int32_t x = rect.left(); x < rect.right(); ++x) {
    for (int32_t y = rect.top(); y < rect.bottom(); ++y) {
      m_screen_buffer[x + m_screen_pitch * y] = m_wallpaper[x + m_wallpaper_width * y];
    }
  }
#endif  // CONFIG_USE_DMA && CONFIG_USE_DMA_FOR_WALLPAPER
}

void WindowManager::draw_window(Window* window, const Rect& dst_rect, DMARequestQueue& request_queue) {
#ifndef CONFIG_USE_DMA
  (void)request_queue;
#endif  // !CONFIG_USE_DMA

  if (!window->is_visible())
    return;

  const Rect& src_rect = window->m_geometry;
  if (!src_rect.intersects(dst_rect))
    return;

  window->draw_frame();

  uint32_t x1 = 0, x2 = src_rect.width();
  uint32_t y1 = 0, y2 = src_rect.height();

  // Do clipping to avoid drawing outside the destination rect.
  if (src_rect.left() < dst_rect.left())
    x1 = dst_rect.left() - src_rect.left();
  if (src_rect.top() < dst_rect.top())
    y1 = dst_rect.top() - src_rect.top();
  if (src_rect.right() > dst_rect.right())
    x2 = x1 + dst_rect.right() - libk::max(src_rect.left(), dst_rect.left());
  if (src_rect.bottom() > dst_rect.bottom())
    y2 = y1 + dst_rect.bottom() - libk::max(src_rect.top(), dst_rect.top());

  if (x1 == x2 || y1 == y2)
    return;

  const uint32_t* framebuffer = window->get_framebuffer();
  const uint32_t framebuffer_pitch = window->get_geometry().width();
  KASSERT(framebuffer != nullptr);

  // Blit the framebuffer into the screen.
#ifdef CONFIG_USE_DMA
  const auto framebuffer_dma_addr =
      window->get_framebuffer_dma_addr() + sizeof(uint32_t) * (x1 + framebuffer_pitch * y1);
  const auto screen_dma_addr =
      m_screen_buffer_dma_addr + sizeof(uint32_t) * (src_rect.x() + x1 + m_screen_pitch * (src_rect.y() + y1));
  const auto src_stride = sizeof(uint32_t) * (framebuffer_pitch - (x2 - x1));
  const auto dst_stride = sizeof(uint32_t) * (m_screen_pitch - (x2 - x1));
  auto* request = DMA::Request::memcpy_2d(framebuffer_dma_addr, screen_dma_addr, sizeof(uint32_t) * (x2 - x1), y2 - y1,
                                          src_stride, dst_stride);
  request_queue.add(request);
#else
  for (uint32_t src_x = x1, dst_x = src_rect.x() + x1; src_x < x2; ++src_x, dst_x++) {
    for (uint32_t src_y = y1, dst_y = src_rect.y() + y1; src_y < y2; ++src_y, dst_y++) {
      const auto color = framebuffer[src_x + framebuffer_pitch * src_y];
      m_screen_buffer[dst_x + m_screen_pitch * dst_y] = color;
    }
  }
#endif  // CONFIG_USE_DMA

  // Draw the focus border to inform the user what window has the focus.
  if (window->has_focus()) {
    graphics::Painter painter(m_screen_buffer, m_screen_width, m_screen_height, m_screen_pitch);
    painter.set_clipping(dst_rect.x1, dst_rect.y1, dst_rect.x2, dst_rect.y2);
    const auto window_rect = window->get_geometry();
    painter.draw_rect(window_rect.x(), window_rect.y(), window_rect.width(), window_rect.height(), 3,
                      0xAA6BA4B8);
  }
}

void WindowManager::draw_windows_helper(libk::LinkedList<Window*>::Iterator it,
                                        const Rect& rect,
                                        DMARequestQueue& request_queue) {
  if (rect.is_null())
    return;

  auto* window = *it;
  const auto window_rect = window->get_geometry();
  const auto intersection = rect.intersection_with(window_rect);
  if (!window->is_visible() || intersection.is_null()) {
    if (it.has_next())
      return draw_windows_helper(++it, rect, request_queue);

    // No more windows. Draw the background.
    draw_background(rect, request_queue);
    return;
  }

  // We decompose the update rectangle into 5 rectangles:
  // A, B, C, D, and E with E the part of the update rectangle that
  // intersects with the current window (e.g. E = intersection).
  // --------------------
  // |     |  C   |     |
  // |     --------     |
  // |  A  |  E   |  B  |
  // |     --------     |
  // |     |  D   |     |
  // --------------------
  // We then draw the current window with the rect E, and recursively
  // update A, B, C and D with the next window.

  const Rect A = {rect.x(), rect.y(), intersection.left(), rect.bottom()};
  const Rect B = {intersection.right(), rect.y(), rect.right(), rect.bottom()};
  const Rect C = {intersection.left(), rect.y(), intersection.right(), intersection.top()};
  const Rect D = {intersection.left(), intersection.bottom(), intersection.right(), rect.bottom()};

  // Draw the window.
  draw_window(window, intersection, request_queue);

  if (!it.has_next()) {
    // No more windows. Draw the background.
    draw_background(A, request_queue);
    draw_background(B, request_queue);
    draw_background(C, request_queue);
    draw_background(D, request_queue);
    return;
  }

  ++it;

  // Update the other parts.
  draw_windows_helper(it, A, request_queue);
  draw_windows_helper(it, B, request_queue);
  draw_windows_helper(it, C, request_queue);
  draw_windows_helper(it, D, request_queue);
}

void WindowManager::draw_windows(const Rect& rect, DMARequestQueue& request_queue) {
  if (m_windows.is_empty()) {
    draw_background(rect, request_queue);
    return;
  }

  // Draw the windows from the top to the bottom.
  draw_windows_helper(m_windows.begin(), rect, request_queue);
}

#ifdef CONFIG_HAS_CURSOR
void WindowManager::draw_cursor() {
  const uint32_t x = m_cursor_x;
  const uint32_t y = m_cursor_y;

  // Clip the cursor to the screen (to avoid drawing outside the framebuffer...).
  const uint32_t cursor_width = libk::min(CURSOR_WIDTH, m_screen_width - x);
  const uint32_t cursor_height = libk::min(CURSOR_HEIGHT, m_screen_height - y);

  // Draw the cursor.
  for (uint32_t i = 0; i < cursor_width; ++i) {
    for (uint32_t j = 0; j < cursor_height; ++j) {
      const uint32_t color = CURSOR_DATA[i + j * CURSOR_WIDTH];
      if ((color & 0xff000000) == 0)
        continue;
      m_screen_buffer[x + i + (y + j) * m_screen_pitch] = color;
    }
  }
}

bool WindowManager::handle_mouse_move_event(int32_t dx, int32_t dy) {
  const int32_t previous_x = m_cursor_x;
  const int32_t previous_y = m_cursor_y;

  // Update cursor position.
  m_cursor_x += dx;
  m_cursor_y += dy;

  // Clamp the cursor position.
  m_cursor_x = libk::clamp(m_cursor_x, 0, m_screen_width);
  m_cursor_y = libk::clamp(m_cursor_y, 0, m_screen_height);

  // Check if the cursor has moved.
  if (m_cursor_x == previous_x && m_cursor_y == previous_y)
    return false;

  // Update the previous cursor position.
  auto previous_rect = Rect::from_pos_and_size(previous_x, previous_y, CURSOR_WIDTH, CURSOR_HEIGHT);
  clip_rect_to_screen(previous_rect);

  // Redraw the windows and background at the previous cursor position.
  // The update function will also draw the cursor at the new position.
  update(previous_rect);
  return false;
}

bool WindowManager::handle_mouse_click_event(int button_type, bool is_pressed) {
  if (button_type == SYS_MOUSE_BUTTON_LEFT && is_pressed) {
    // Check if the cursor is on a window.
    for (auto* window : m_windows) {
      if (window->get_geometry().contains(m_cursor_x, m_cursor_y)) {
        focus_window(window);
        return true;
      }
    }
  }

  return false;
}
#endif // #ifdef CONFIG_HAS_CURSOR

constexpr uint32_t WINDOW_MOVE_STEP = 10;
constexpr uint32_t WINDOW_RESIZE_STEP = 10;

bool WindowManager::handle_key_event(sys_key_event_t event) {
  if (!sys_is_press_event(event))
    return false;

  if (!sys_is_alt_pressed(event))
    return false;

  switch (sys_get_key_code(event)) {
    case SYS_KEY_T:  // Alt+T -> switch focus
      switch_focus();
      return true;
    case SYS_KEY_LEFT_ARROW:  // Alt+Left -> move window to the left
      if (sys_is_shift_pressed(event))
        resize_focus_window(-WINDOW_RESIZE_STEP, 0);
      else
        move_focus_window(-WINDOW_MOVE_STEP, 0);
      return true;
    case SYS_KEY_RIGHT_ARROW:  // Alt+Right -> move window to the right
      if (sys_is_shift_pressed(event))
        resize_focus_window(WINDOW_RESIZE_STEP, 0);
      else
        move_focus_window(WINDOW_MOVE_STEP, 0);
      return true;
    case SYS_KEY_UP_ARROW:  // Alt+Arrow -> move window up
      if (sys_is_shift_pressed(event))
        resize_focus_window(0, -WINDOW_RESIZE_STEP);
      else
        move_focus_window(0, -WINDOW_MOVE_STEP);
      return true;
    case SYS_KEY_DOWN_ARROW:  // Alt+Down -> move window down
      if (sys_is_shift_pressed(event))
        resize_focus_window(0, WINDOW_RESIZE_STEP);
      else
        move_focus_window(0, WINDOW_MOVE_STEP);
      return true;
    case SYS_KEY_F:  // Alt+F -> toggle fullscreen
                     // TODO: better fullscreen support (for example, when focus out, the window should not be
                     // fullscreen anymore)
      if (m_focus_window != nullptr)
        set_window_geometry(m_focus_window, Rect::from_pos_and_size(0, 0, m_screen_width, m_screen_height));
      return true;
    case SYS_KEY_Q:  // Alt+Q -> close window
      if (m_focus_window != nullptr) {
        sys_message_t message = {};
        message.id = SYS_MSG_CLOSE;
        post_message(m_focus_window, message);
      }

      return true;
    case SYS_KEY_M:  // Alt+M -> mosaic layout
      mosaic_layout();
      return true;
    case SYS_KEY_E: {  // Alt+E -> spawn the file explorer
      auto explorer = TaskManager::get().create_task("/bin/explorer");
      if (explorer == nullptr) {
        LOG_ERROR("Failed to spawn the file explorer");
        return true;
      }

      TaskManager::get().wake_task(explorer);
      return true;
    }
    default:
      return false;
  }
}

void WindowManager::switch_focus() {
  // This is a rolling window switch
  // The one with the focus goes to the back and
  // the one just behin him gets the focus.
  // (It's ugly, but it works for now.)

  if (m_windows.is_empty())
    return;

  if (m_focus_window == nullptr) {
    focus_window(*m_windows.begin());
    return;
  } else {
    auto old_window_it = std::find(m_windows.begin(), m_windows.end(), m_focus_window);
    auto new_window_it = old_window_it;

    new_window_it++;

    if (new_window_it == m_windows.end())
      new_window_it = m_windows.begin();

    if (*old_window_it == *new_window_it)
      return;

    sys_message_t message;

    (*old_window_it)->m_focus = false;

    // Send focus out messsage.
    libk::bzero(&message, sizeof(sys_message_t));
    message.id = SYS_MSG_FOCUS_OUT;
    post_message(*old_window_it, message);

    // Send focus in message.
    libk::bzero(&message, sizeof(sys_message_t));
    message.id = SYS_MSG_FOCUS_IN;
    post_message(*new_window_it, message);

    m_focus_window = *new_window_it;
    m_focus_window->m_focus = true;

    // Move new window to front
    auto new_window = *new_window_it;
    m_windows.erase(new_window_it);
    m_windows.push_front(new_window);

    // Move old window to back
    auto old_window = *old_window_it;
    m_windows.erase(old_window_it);
    m_windows.push_back(old_window);

    update(new_window->get_geometry());
  }
}

void WindowManager::move_focus_window(int32_t dx, int32_t dy) {
  if (m_focus_window == nullptr)
    return;

  auto rect = m_focus_window->get_geometry();
  rect.x1 += dx;
  rect.x2 += dx;
  rect.y1 += dy;
  rect.y2 += dy;
  set_window_geometry(m_focus_window, rect);
}

void WindowManager::resize_focus_window(int32_t dx, int32_t dy) {
  if (m_focus_window == nullptr)
    return;

  auto rect = m_focus_window->get_geometry();

  if (rect.width() + dx < Window::MIN_WIDTH)
    dx = Window::MIN_WIDTH - rect.width();
  rect.x2 += dx;

  if (rect.height() + dy < Window::MIN_HEIGHT)
    dy = Window::MIN_HEIGHT - rect.height();
  rect.y2 += dy;

  set_window_geometry(m_focus_window, rect);
}

#include "fs/fat/ff.h"
#include "window_manager.hpp"

void WindowManager::read_wallpaper() {
  constexpr const char* WALLPAPER_PATH = "/wallpaper.jpg";

  FIL file = {};
  if (f_open(&file, WALLPAPER_PATH, FA_READ) != FR_OK) {
    LOG_WARNING("Failed to open '{}'", WALLPAPER_PATH);
    return;
  }

  const UINT file_size = f_size(&file);
  uint8_t* buffer = (uint8_t*)kmalloc(file_size, alignof(max_align_t));
  KASSERT(buffer != nullptr);

  UINT read_bytes;
  const auto result = f_read(&file, buffer, file_size, &read_bytes);
  KASSERT(result == FR_OK);
  KASSERT(read_bytes == file_size);
  f_close(&file);

  int wallpaper_width, wallpaper_height;
  uint32_t* wallpaper =
      (uint32_t*)stbi_load_from_memory(buffer, file_size, &wallpaper_width, &wallpaper_height, nullptr, 4);
  kfree(buffer);

  if (wallpaper == nullptr) {
    LOG_WARNING("Failed to load the wallpaper, the file is probably badly formatted or not a JPEG");
    return;
  }

  // The wallpaper is in RGBA, we expect ABGR. Do the conversion once.
  for (int32_t x = 0; x < wallpaper_width; ++x) {
    for (int32_t y = 0; y < wallpaper_height; ++y) {
      // The wallpaper is in RGBA, we expect ABGR.
      const auto color = libk::bswap(wallpaper[x + wallpaper_width * y]);
      wallpaper[x + wallpaper_width * y] = color >> 8;
    }
  }

#if defined(CONFIG_USE_DMA) && defined(CONFIG_USE_DMA_FOR_WALLPAPER)
  m_wallpaper = libk::make_scoped<Buffer>(sizeof(uint32_t) * wallpaper_width * wallpaper_height);
  libk::memcpy(m_wallpaper->get(), wallpaper, m_wallpaper->get_byte_size());
#else
  m_wallpaper = wallpaper;
#endif  // CONFIG_USE_DMA && CONFIG_USE_DMA_FOR_WALLPAPER
  m_wallpaper_width = wallpaper_width;
  m_wallpaper_height = wallpaper_height;

  LOG_INFO("Wallpaper loaded (size {}x{})", m_wallpaper_width, m_wallpaper_height);
}
