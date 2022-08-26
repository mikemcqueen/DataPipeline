////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// MainWindow_t.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainWindow_t.h"
#include "Eq2LoginWindow.h"
#include "BrokerWindow.h"
#include "BrokerBuyWindow.h"
#include "BrokerSellWindow.h"
#include "SetPriceWindow.h"
#include "BrokerUi.h"
#include "BrokerId.h"
#include "Log.h"
#include "OtherWindows.h"

namespace Broker
{

////////////////////////////////////////////////////////////////////////////////

static const wchar_t s_szClassName[] = L"EQ2ApplicationClass";

Rect_t MainWindow_t::m_PopupRect;

////////////////////////////////////////////////////////////////////////////////

MainWindow_t::
MainWindow_t(
    bool bTest)
:
    Ui::Window::Base_t(
        Ui::Window::Id::MainWindow,
        bTest ? NULL : s_szClassName,
        NULL)
{
}

////////////////////////////////////////////////////////////////////////////////

Broker::Window_t&
MainWindow_t::
GetBrokerWindow() const
{
    using namespace Broker::Window;
    return static_cast<Broker::Window_t&>(GetWindow(Id::BrokerFrame));
}

////////////////////////////////////////////////////////////////////////////////

Broker::Buy::Window_t&
MainWindow_t::
GetBrokerBuyWindow() const
{
    using namespace Broker::Window;
    return static_cast<Buy::Window_t&>(GetWindow(Id::BrokerBuyTab));
}

////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////

Ui::Window::Base_t&
MainWindow_t::
GetWindow(
    Ui::WindowId_t windowId) const
{
#if 1
    static Eq2Login::Window_t eq2LoginWindow(*this);
    static Eq2LoadingWindow_t eq2LoadingWindow;
    static TransitionWindow_t zoningWindow;
    static MainChatWindow_t   mainChatWindow;
    static Broker::Window_t   brokerWindow(*this);
    static SetPrice::Window_t setPricePopup(*this);
#endif

    switch (windowId)
    {
    case Window::Id::Eq2Login:            return eq2LoginWindow;
    case Window::Id::Eq2Loading:          return eq2LoadingWindow;
    case Window::Id::Zoning:              return zoningWindow;
    case Window::Id::MainChat:            return mainChatWindow;
    case Window::Id::BrokerFrame:         return brokerWindow;
    case Window::Id::BrokerBuyTab:        // fall through;
    case Window::Id::BrokerSellTab:       // fall through;
    case Window::Id::BrokerSalesLogTab:   return brokerWindow.GetWindow(windowId);
    case Window::Id::BrokerSetPricePopup: return setPricePopup;
    default:
        throw std::invalid_argument("MainWindow_t::GetWindow()");
    }
}

////////////////////////////////////////////////////////////////////////////////

Ui::WindowId_t
MainWindow_t::
GetWindowId(
    const CSurface& /*Surface*/,
    const POINT*    /*pptHint*/) const
{
    throw logic_error("MainWindow_t::GetWindowId() called");
}

////////////////////////////////////////////////////////////////////////////////

Ui::WindowId_t
MainWindow_t::
GetMessageWindowId(
    const DP::MessageId_t& messageId) const
{
    Ui::WindowId_t windowId = Ui::Window::Id::Unknown;
    switch (messageId)
    {
    case Message::Id::Eq2Login: windowId = Window::Id::Eq2Login;            break;
    case Message::Id::Buy:      windowId = Window::Id::BrokerBuyTab;        break;
    case Message::Id::Sell:     windowId = Window::Id::BrokerSellTab;       break;
    case Message::Id::SalesLog: windowId = Window::Id::BrokerSalesLogTab;   break;
    case Message::Id::SetPrice: windowId = Window::Id::BrokerSetPricePopup; break;
    default:
        break;
    }
    return windowId;
}

////////////////////////////////////////////////////////////////////////////////

} // Broker

////////////////////////////////////////////////////////////////////////////////
