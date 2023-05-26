////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// BrokerBuyWindow.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BrokerBuyWindow.h"
#include "BrokerBuyTypes.h"
#include "BrokerBuyWidgets.h"
#include "BrokerWindow.h"
#include "Log.h"

namespace Broker::Buy {

  constexpr Flag_t kWindowFlags;

  constexpr POINT BrokerDefaultTabOffset{ -55, 41 }; // Broker Buy
  constexpr POINT BrokerDefaultTableOffset{ -12, 102 };
  constexpr POINT MarketDefaultTabOffset{ -54, 41 }; // Market Browse
  constexpr POINT MarketDefaultTableOffset{ -11, 98 };
  constexpr RECT InnerTableRect{ 0,  2, 0, 0 }; // wat ees thees magicks?

#if 0
  /* static */
  const Ui::Widget::Data_t& Window_t::GetWidgetData(Ui::WidgetId_t widgetId) {
    for (size_t widget = 0; widget < s_widgetCount; ++widget) {
      if (s_widgets[widget].WidgetId == widgetId) {
        return s_widgets[widget];
      }
    }
    throw invalid_argument("BrokerBuyTab::GetWidgetData()");
  }
#endif

  Window_t::Window_t(const Ui::Window_t& parent) :
    TableWindow_t(
      kWindowId,
      parent,
      kWindowName,
      {},
      kWindowFlags,
      std::span{ Widgets },
      BrokerDefaultTableOffset,
      InnerTableRect,
      BrokerDefaultTabOffset)
  {
  }

  const Broker::Window_t& Window_t::GetBrokerWindow() const {
    return dynamic_cast<const Broker::Window_t&>(GetParent());
  }

  ////////////////////////////////////////////////////////////////////////////////
  //
  // This function is Wrong and shouldn't be here. The "BuyWindow" should be
  // modeled as "not having tabs". Tabs (and this implementation) should be
  // owned by BrokerFrameWindow.
  //
  bool Window_t::GetWidgetRect(Ui::WidgetId_t widgetId, Rect_t* pRect) const {
    using namespace Broker::Frame;
    if ((Frame::Widget::Id::SellTab == widgetId) ||
      (Frame::Widget::Id::SalesLogTab == widgetId))
    {
      switch (GetBrokerWindow().GetLayout()) {
      case Layout::Broker:
        return Ui::Window_t::GetWidgetRect(widgetId, GetTableRect(), pRect,
          std::span{ BrokerTabs });
      case Layout::Market:
        return Ui::Window_t::GetWidgetRect(widgetId, GetTableRect(), pRect,
          std::span{ MarketTabs });
      default:
        throw std::logic_error("BrokerBuyWindow::GetWidgetRect() Invalid layout");
      }
    }
    else {
      return TableWindow_t::GetWidgetRect(widgetId, pRect);
    }
  }

  void Window_t::SetLayout(Frame::Layout_t layout) const {
    switch (layout) {
    case Frame::Layout::Broker:
      SetOffsets(BrokerDefaultTabOffset, BrokerDefaultTableOffset);
      break;
    case Frame::Layout::Market:
      SetOffsets(MarketDefaultTabOffset, MarketDefaultTableOffset);
      break;
    default:
      throw std::logic_error("BrokerBuyWindow::SetLayout() Invalid layout");
    }
  }
} // namespace Broker::Buy
