////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// BrokerBuyWindow.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BrokerBuyWindow.h"
#include "BrokerUi.h"
#include "Log.h"
#include "MainWindow_t.h"
#include "BrokerWindow.h"
#include "BrokerBuyWidgets.h"

namespace Broker::Buy {

static const Flag_t WindowFlags;

constexpr POINT BrokerDefaultTabOffset{ -55, 41 }; // Broker Buy
constexpr POINT BrokerDefaultTableOffset{ -11, 103 };
constexpr POINT MarketDefaultTabOffset{ -54, 41 }; // Market Browse
constexpr POINT MarketDefaultTableOffset{ -11, 98 };
constexpr RECT InnerTableRect{ 0,  2, 0, 0 }; // wat ees thees magicks?

#if 0
/* static */
const Ui::Widget::Data_t&
Window_t::
GetWidgetData(
    Ui::WidgetId_t widgetId)
{
    for (size_t widget = 0; widget < s_widgetCount; ++widget)
    {
        if (s_widgets[widget].WidgetId == widgetId)
        {
            return s_widgets[widget];
        }
    }
    throw invalid_argument("BrokerBuyTab::GetWidgetData()");
}
#endif

////////////////////////////////////////////////////////////////////////////////

Window_t::
Window_t(
    const Ui::Window_t& parent)
:
    TableWindow_t(
        Broker::Window::Id::BrokerBuy,
        parent,
        L"BrokerBuy",
        WindowFlags,
        std::span{ Widgets },
        BrokerDefaultTableOffset,
        InnerTableRect,
        BrokerDefaultTabOffset)
{
}

////////////////////////////////////////////////////////////////////////////////

MainWindow_t&
Window_t::
GetMainWindow() const
{
    // TODO: almost certainly not good
    return const_cast<MainWindow_t&>(dynamic_cast<const MainWindow_t&>(GetParent()));
}

////////////////////////////////////////////////////////////////////////////////
//
// This function is Wrong and shouldn't be here. The "BuyWindow" should be
// modeled as "not having tabs". Tabs (and this implementation) should be
// owned by BrokerFrameWindow.
//
bool
Window_t::
GetWidgetRect(
    Ui::WidgetId_t widgetId,
    Rect_t* pRect) const
{
    const MainWindow_t& mainWindow = GetMainWindow();//static_cast<const MainWindow_t&>(GetParent());
    if ((Frame::Widget::Id::SellTab == widgetId) ||
        (Frame::Widget::Id::SalesLogTab == widgetId))
    {
        switch (mainWindow.GetBrokerWindow().GetLayout()) {
        case Frame::Layout::Broker:
            return Ui::Window_t::GetWidgetRect(widgetId, GetTableRect(), pRect,
                std::span{ BrokerTabs });
        case Frame::Layout::Market:
            return Ui::Window_t::GetWidgetRect(widgetId, GetTableRect(), pRect,
                std::span{ MarketTabs });
        default:
            throw logic_error("BrokerBuyWindow::GetWidgetRect() Invalid layout");
        }
    } else {
        return TableWindow_t::GetWidgetRect(widgetId, pRect);
    }
}

////////////////////////////////////////////////////////////////////////////////

void
Window_t::
SetLayout(
    Frame::Layout_t layout)
{
    switch (layout) {
    case Frame::Layout::Broker:
        SetOffsets(BrokerDefaultTabOffset, BrokerDefaultTableOffset);
        break;
    case Frame::Layout::Market:
        SetOffsets(MarketDefaultTabOffset, MarketDefaultTableOffset);
        break;
    default:
        throw logic_error("BrokerBuyWindow::SetLayout() Invalid layout");
    }
}

} // namespace Broker::Buy
