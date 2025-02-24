#include <sys/syscall.h>
#include <sys/window.h>
#include <tulip/window.hpp>
#include <tulip/application.hpp>
#include <tulip/button.hpp>
#include <tulip/layout.hpp>

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

#define TRACE (sys_print("TRACE at " __FILE__ ":" STRINGIFY(__LINE__)))

extern "C" int main() {
  TuApplication app;

  TuWindow window("UI Test", 400, 200);
  if (!window.is_valid())
    return 1;

  #if 0
  TRACE;
  TuWidget widget(&window);

  TRACE;
  TuButton* button1 = new TuButton("Click me 1", &widget);
  TRACE;
  TuButton* button2 = new TuButton("Click me 2", &widget);
  TRACE;
  TuHBoxLayout layout(&widget);
  TRACE;
  layout.add_widget(button1);
  TRACE;
  layout.add_widget(button2);
  TRACE;
  widget.set_layout(&layout);
  TRACE;
  #endif

  return app.run();
}
