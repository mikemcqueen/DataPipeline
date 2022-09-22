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

namespace Broker
{
namespace Buy
{

////////////////////////////////////////////////////////////////////////////////

static const Flag_t WindowFlags;

static const POINT  BrokerDefaultTabOffset   = { -55, 41 }; // Broker Buy
static const POINT  BrokerDefaultTableOffset = { -11, 103 };
static const POINT  MarketDefaultTabOffset   = { -54, 41 }; // Market Browse
static const POINT  MarketDefaultTableOffset = { -11, 98 };
static const RECT   InnerTableRect           = {   0,  2, 0, 0 }; // wat ees thees magicks?

//const size_t s_widgetCount = _countof(s_widgets);

////////////////////////////////////////////////////////////////////////////////

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
        Broker::Window::Id::BrokerBuyTab,
        parent,
        L"BrokerBuyTab",
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
    return const_cast<MainWindow_t&>(dynamic_cast<const MainWindow_t&>(GetParent()));
}

////////////////////////////////////////////////////////////////////////////////

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
        switch (mainWindow.GetBrokerWindow().GetLayout())
        {
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
    switch (layout)
    {
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

////////////////////////////////////////////////////////////////////////////////

} // Buy
} // Broker

////////////////////////////////////////////////////////////////////////////////
