////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// BrokerSellWindow.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "TabWindow.h"
#include "BrokerUi.h"

class MainWindow_t;

namespace Broker::Sell {
  class Window_t : public TableWindow_t {
  public:
    explicit Window_t(const Ui::Window_t& parent);

    //
    // Ui::Window_t virtual:
    //

    Ui::WindowId_t GetWindowId(
      const CSurface& surface,
      const POINT* pptHint) const override;

    bool GetWidgetRect(
      Ui::WidgetId_t widgetId,
      Rect_t* rect) const override;

    void GetScrollOffsets(
      const CSurface& surface,
      const Rect_t& tableRect,
      SIZE& scrollOffsets) const; // override;

    void SetLayout(Frame::Layout_t layout);

  private:
    MainWindow_t& GetMainWindow() const;

    bool FindSetPricePopup(
      const CSurface& surface,
      const POINT& ptHint,
      Rect_t& surfaceRect) const;

  private:
    Window_t();
    Window_t(const Window_t&);
    Window_t& operator=(const Window_t&);
  };
} // namespace Broker::Sell
