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

namespace Broker::Sell {
  class Broker::Window_t;

  class Window_t : public TableWindow_t {
  public:
    Window_t() = delete;
    explicit Window_t(const Ui::Window_t& parent);

    // Ui::Window_t virtual:
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

    void SetLayout(Frame::Layout_t layout) const;

  private:
    const Broker::Window_t& GetBrokerWindow() const;

    bool FindSetPricePopup(
      const CSurface& surface,
      const POINT& ptHint,
      Rect_t& surfaceRect) const;
  };
} // namespace Broker::Sell
