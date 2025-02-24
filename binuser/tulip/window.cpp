#include "window.hpp"
#include "application.hpp"
#include "event.hpp"
#include "painter.hpp"
#include "title_bar.hpp"
#include "widget.hpp"

#include <sys/syscall.h>
#include <cstdint>

class TuRootWidget : public TuWidget {
 public:
  TuRootWidget(TuWindow* window) : TuWidget(window) { title_bar = new TuTitleBar(this); }

  // Title bar widget. Defined by Tulip but can be customized.
  TuTitleBar* title_bar = nullptr;
  // Content widget (the area inside the window, excluding the title bar).
  // Defined by the user.
  TuWidget* content_widget = nullptr;

 protected:
  friend class TuWindow;

  void on_event(TuEvent* event) override {
    switch (event->get_type()) {
      case TuEvent::Type::PAINT:
        TuWidget::on_event(event);
        get_window()->present(static_cast<TuPaintEvent*>(event)->get_rect());
        break;
      case TuEvent::Type::FOCUS_IN:
      case TuEvent::Type::FOCUS_OUT:
        title_bar->repaint();
        get_window()->present(title_bar->get_geometry());
        break;
      default:
        TuWidget::on_event(event);
        break;
    }
  }

  void on_resize_event(TuResizeEvent* event) override {
    (void)event;
    update_layout();
  }

 private:
  void update_layout() {
    const TuSize new_size = get_size();
    const TuSize title_bar_size = {new_size.width, title_bar->get_height()};

    title_bar->resize(title_bar_size);

    if (content_widget != nullptr) {
      const TuSize content_widget_size = {new_size.width, new_size.height - title_bar->get_height()};
      content_widget->move({0, (int32_t)title_bar->get_height()});
      content_widget->resize(content_widget_size);
    }

    repaint();
  }
};  // class TuRootWidget

TuWindow::TuWindow(const char* title, uint32_t width, uint32_t height) {
  m_has_focus = true;  // Generally, when a window is created it has the focus
  m_window = sys_window_create(title, SYS_POS_DEFAULT, SYS_POS_DEFAULT, width, height, SYS_WF_NO_FRAME);
  if (m_window == nullptr)
    return;

  m_title = title;
  sys_window_get_geometry(m_window, (uint32_t*)&m_position.x, (uint32_t*)&m_position.y, &m_size.width, &m_size.height);

  m_root_widget = new TuRootWidget(this);
  m_root_widget->resize(m_size);

  TuApplication::get()->register_window(this);
}

TuWindow::~TuWindow() {
  if (m_root_widget != nullptr)
    delete m_root_widget;

  if (m_window != nullptr) {
    TuApplication::get()->unregister_window(this);
    sys_window_destroy(m_window);
  }
}

void TuWindow::set_title(const char* title) {
  sys_window_set_title(m_window, title);
  m_title = title;
  if (m_root_widget->title_bar != nullptr)
    m_root_widget->title_bar->repaint();
}

void TuWindow::move(TuPoint position) {
  if (position == m_position)
    return;  // No change

  sys_window_set_geometry(m_window, m_position.x, m_position.y, m_size.width, m_size.height);
  sys_window_get_geometry(m_window, (uint32_t*)&m_position.x, (uint32_t*)&m_position.y, &m_size.width, &m_size.height);
}

void TuWindow::resize(TuSize size) {
  if (size == m_size)
    return;  // No change

  sys_window_set_geometry(m_window, m_position.x, m_position.y, m_size.width, m_size.height);
  sys_window_get_geometry(m_window, (uint32_t*)&m_position.x, (uint32_t*)&m_position.y, &m_size.width, &m_size.height);
}

void TuWindow::set_content(TuWidget* widget) {
  m_root_widget->content_widget = widget;
  widget->set_parent(m_root_widget);
  m_root_widget->update_layout();
}

void TuWindow::present(int32_t x, int32_t y, uint32_t width, uint32_t height) {
  x = TuClamp(x, 0, (int32_t)m_size.width);
  y = TuClamp(y, 0, (int32_t)m_size.height);
  width = TuClamp<uint32_t>(width, 0, m_size.width - x);
  height = TuClamp<uint32_t>(height, 0, m_size.height - y);
  sys_window_present2(m_window, x, y, width, height);
}

void TuWindow::poll_system_messages() {
  sys_message_t msg;
  while (sys_poll_message(m_window, &msg)) {
    switch (msg.id) {
      case SYS_MSG_MOVE: {
        const TuPoint old_position = m_position;
        m_position = {(int32_t)msg.param1, (int32_t)msg.param2};
        TuMoveEvent event(old_position, m_position);
        m_root_widget->on_event(&event);
      } break;
      case SYS_MSG_RESIZE: {
        const TuSize old_size = m_size;
        m_size = {(uint32_t)msg.param1, (uint32_t)msg.param2};
        TuResizeEvent event(old_size, m_size);
        m_root_widget->on_event(&event);
      } break;
      case SYS_MSG_FOCUS_IN: {
        m_has_focus = true;
        TuEvent event(TuEvent::Type::FOCUS_IN);
        m_root_widget->on_event(&event);
      } break;
      case SYS_MSG_FOCUS_OUT: {
        m_has_focus = false;
        TuEvent event(TuEvent::Type::FOCUS_OUT);
        m_root_widget->on_event(&event);
      } break;
      case SYS_MSG_CLOSE:
        m_should_close = true;
        break;
      default:
        break;
    }
  }
}
