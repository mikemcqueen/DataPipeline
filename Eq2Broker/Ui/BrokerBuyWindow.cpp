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
static const RECT   InnerTableRect           = {   0,  5, 0, 0 };

static const Ui::Widget::Data_t s_brokerWidgets[] = 
{
    { Frame::Widget::Id::SellTab,        { RelativeRect_t::LeftTop,       72, -98,    9,    9 } }, // Broker Buy
    { Frame::Widget::Id::SalesLogTab,    { RelativeRect_t::LeftTop,      122, -98,    9,    9 } }, // Broker Buy
};

static const Ui::Widget::Data_t s_marketWidgets[] = 
{
    { Frame::Widget::Id::SellTab,        { RelativeRect_t::LeftTop,       94,  -98,    9,    9 } }, // Market Browse
    { Frame::Widget::Id::SalesLogTab,    { RelativeRect_t::LeftTop,      145,  -98,    9,    9 } }, // Market Browse
};

static const Ui::Widget::Data_t s_widgets[] =
{
    { Widget::Id::SearchLabel,           { RelativeRect_t::LeftTop,       12,  -70,    9,    9 } },
    { Widget::Id::SearchEdit,            { RelativeRect_t::LeftTop,       53,  -73,  620,   16 } },
    { Widget::Id::FindButton,            { RelativeRect_t::LeftTop,      717,  -70,    9,    9 } },
    { Widget::Id::SearchDropdown,        { RelativeRect_t::LeftTop,      778,  -70,  133,   12 } },
    { Widget::Id::PageNumber,            { RelativeRect_t::CenterBottom,   0,   13,  200,   20 } },
    { Widget::Id::NextButton,            { RelativeRect_t::RightBottom, -180,   18,   26,    8 } },
    { Widget::Id::BuyButton,             { RelativeRect_t::RightBottom,  -76,   66,   21,   12 } },
};

const size_t s_widgetCount = _countof(s_widgets);

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
        s_widgets,
        s_widgetCount,
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
    Rect_t&        rect) const
{
    const MainWindow_t& mainWindow = GetMainWindow();//static_cast<const MainWindow_t&>(GetParent());
    if ((Frame::Widget::Id::SellTab == widgetId) ||
        (Frame::Widget::Id::SalesLogTab == widgetId))
    {
        switch (mainWindow.GetBrokerWindow().GetLayout())
        {
        case Frame::Layout::Broker:
            return Ui::Window_t::GetWidgetRect(widgetId, GetTableRect(), rect,
                       s_brokerWidgets, _countof(s_brokerWidgets));
        case Frame::Layout::Market:
            return Ui::Window_t::GetWidgetRect(widgetId, GetTableRect(), rect,
                       s_marketWidgets, _countof(s_marketWidgets));
        default:
            throw logic_error("BrokerBuyWindow::GetWidgetRect() Invalid layout");
        }
    }
    else
    {
        return TableWindow_t::GetWidgetRect(widgetId, rect);
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
