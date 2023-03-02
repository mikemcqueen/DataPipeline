////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// MainWindow_t.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainWindow_t.h"
#include "BrokerUi.h"
#include "BrokerId.h"
#include "Log.h"
#include "OtherWindows.h"
#include "CommonTypes.h"

//Rect_t MainWindow_t::m_PopupRect;

MainWindow_t::MainWindow_t() :
  Ui::Window::WithHandle_t(
    Ui::Window::Id::MainWindow,
    "EQ2ApplicationClass",
    "MainWindow",
    { broker_frame_ }),
  broker_frame_(*this)
{}

/*
const Broker::Window_t& MainWindow_t::GetBrokerWindow() const {
  using namespace Broker::Window;
  return static_cast<Broker::Window_t&>(GetWindow(Id::BrokerFrame));
}
*/

#if 0
i::Window::Base_t& MainWindow_t::GetWindow(Ui::WindowId_t windowId) const {
  using namespace Broker;
 
  switch (windowId) {

  case Window::Id::BrokerBuy: //[[fallthrough]]
  case Window::Id::BrokerSell:          return brokerFrameWindow.GetWindow(windowId);
  case Window::Id::BrokerFrame:         return brokerFrameWindow;
//  case Window::Id::MainChat:            return mainChatWindow;
  default:
    throw std::invalid_argument(std::format("MainWindow_t::GetWindow() unknown window({})", intValue(windowId)));
  }
}
#endif

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
  using namespace Broker;
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
