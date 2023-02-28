////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// MainWindow_t.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainWindow_t.h"
//#include "Eq2LoginWindow.h"
#include "BrokerWindow.h"
#include "BrokerBuyWindow.h"
//#include "BrokerSellWindow.h"
//#include "SetPriceWindow.h"
#include "BrokerUi.h"
#include "BrokerId.h"
#include "Log.h"
#include "OtherWindows.h"
#include "CommonTypes.h"

namespace Broker
{

Rect_t MainWindow_t::m_PopupRect;

MainWindow_t::MainWindow_t(bool bTest) :
  Ui::Window::Base_t(
    Ui::Window::Id::MainWindow,
    bTest ? "" : "EQ2ApplicationClass",
    "MainWindow")
{
}

////////////////////////////////////////////////////////////////////////////////

Broker::Window_t& MainWindow_t::GetBrokerWindow() const {
    using namespace Broker::Window;
    return static_cast<Broker::Window_t&>(GetWindow(Id::BrokerFrame));
}

////////////////////////////////////////////////////////////////////////////////

Broker::Buy::Window_t& MainWindow_t::GetBrokerBuyWindow() const {
    using namespace Broker::Window;
    return static_cast<Buy::Window_t&>(GetWindow(Id::BrokerBuy));
}

////////////////////////////////////////////////////////////////////////////////
#if 0
Broker::Sell::Window_t&
MainWindow_t::
GetBrokerSellWindow() const
{
    using namespace Broker::Window;
    return static_cast<Sell::Window_t&>(GetWindow(Id::BrokerSellTab));
}

////////////////////////////////////////////////////////////////////////////////

Broker::SetPrice::Window_t&
MainWindow_t::
GetSetPricePopup() const
{
    using namespace Broker::Window;
    return static_cast<SetPrice::Window_t&>(GetWindow(Id::BrokerSetPricePopup));
}

////////////////////////////////////////////////////////////////////////////////

Broker::Eq2Login::Window_t&
MainWindow_t::
GetEq2LoginWindow() const
{
    using namespace Broker::Window;
    return static_cast<Eq2Login::Window_t&>(GetWindow(Id::Eq2Login));
}
#endif

////////////////////////////////////////////////////////////////////////////////

Ui::Window::Base_t& MainWindow_t::GetWindow(Ui::WindowId_t windowId) const {
#if 0
  static Eq2Login::Window_t eq2LoginWindow(*this);
  static Eq2LoadingWindow_t eq2LoadingWindow;
  static TransitionWindow_t zoningWindow;
  static SetPrice::Window_t setPricePopup(*this);
#endif
  static MainChatWindow_t   mainChatWindow;
  static Broker::Window_t   brokerFrameWindow(*this);

  switch (windowId) {
#if 0
  case Window::Id::Eq2Login:            return eq2LoginWindow;
  case Window::Id::Eq2Loading:          return eq2LoadingWindow;
  case Window::Id::Zoning:              return zoningWindow;
  case Window::Id::BrokerSetPricePopup: return setPricePopup;
  case Window::Id::BrokerSellTab: [[fallthrough]];
  case Window::Id::BrokerSalesLogTab: [[fallthrough]];
#endif
  case Window::Id::BrokerBuy: //[[fallthrough]]
  case Window::Id::BrokerSell:          return brokerFrameWindow.GetWindow(windowId);
  case Window::Id::BrokerFrame:         return brokerFrameWindow;
  case Window::Id::MainChat:            return mainChatWindow;
  default:
    throw std::invalid_argument(std::format("MainWindow_t::GetWindow() unknown window({})", intValue(windowId)));
  }
}

////////////////////////////////////////////////////////////////////////////////

Ui::WindowId_t MainWindow_t::GetWindowId(
    const CSurface& /*Surface*/,
    const POINT*    /*pptHint*/) const
{
    throw logic_error("MainWindow_t::GetWindowId() called");
}

////////////////////////////////////////////////////////////////////////////////

Ui::WindowId_t MainWindow_t::GetMessageWindowId(
  const DP::MessageId_t& messageId) const
{
  switch (messageId) {
  case Message::Id::Eq2Login: return Window::Id::Eq2Login;
  case Message::Id::Buy:      return Window::Id::BrokerBuy;
  case Message::Id::Sell:     return Window::Id::BrokerSell;
  case Message::Id::SalesLog: return Window::Id::BrokerSalesLog;
  case Message::Id::SetPrice: return Window::Id::BrokerSetPrice;
  default:
    return Ui::Window::Id::Unknown;
  }
}

} // Broker
