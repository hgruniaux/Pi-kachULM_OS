#ifndef TULIP_TITLE_BAR_HPP
#define TULIP_TITLE_BAR_HPP

#include <tulip/widget.hpp>

class TuTitleBar : public TuWidget {
public:
  explicit TuTitleBar(TuWidget* parent);

protected:
    void on_paint_event(TuPaintEvent* event) override;
}; // class TuTitleBar

#endif // TULIP_TITLE_BAR_HPP
