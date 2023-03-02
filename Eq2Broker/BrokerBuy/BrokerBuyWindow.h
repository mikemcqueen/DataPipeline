////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// BrokerBuyWindow.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef INCLUDE_BROKERBUYWINDOW_H
#define INCLUDE_BROKERBUYWINDOW_H

#include "TabWindow.h"
#include "BrokerUi.h"

namespace Broker::Buy {
  class Broker::Window_t;

  class Window_t : public TableWindow_t {
  public:
    explicit Window_t(const Ui::Window_t& parent);

    bool GetWidgetRect(
      Ui::WidgetId_t widgetId,
      Rect_t* pRect) const override;

    void SetLayout(Frame::Layout_t layout) const;

  private:
    const Broker::Window_t& GetBrokerWindow() const;

    Window_t();
    Window_t(const Window_t&);
    Window_t& operator=(const Window_t&);
  };
} // Broker::Buy

#endif // INCLUDE_BROKERBUYWINDOW_H
