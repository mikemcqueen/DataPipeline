////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// BrokerSalesLogWindow.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BrokerSalesLogTab.h"
#include "BrokerUi.h"
#include "Log.h"
#include "DdUtil.h"

////////////////////////////////////////////////////////////////////////////////

namespace Broker
{
namespace SalesLog
{

static const Flag_t WindowFlags;

static const POINT  DefaultTabOffset   = {  -55, 41 };
static const POINT  DefaultTableOffset = { -11, 98 };
static const RECT   InnerTableRect     = { 0, 5, 0, 0 };

#if 0
static const Ui::Widget::Data_t s_widgets[] =
{
//    { Ui::Widget::Id::VScrollUp,    { RelativeRect_t::RightTop,     -22,   12, 15, 17 } },
//    { Ui::Widget::Id::VScrollDown,  { RelativeRect_t::RightBottom,  -22,  -25, 15, 17 } },
//    { Frame::Widget::Id::BuyTab,    { RelativeRect_t::LeftTop,       11, -178,  9,  9 } },
//    { Frame::Widget::Id::SellTab, { RelativeRect_t::LeftTop,       11, -178,  9,  9 } },
};
#endif

////////////////////////////////////////////////////////////////////////////////

Window_t::
Window_t(
    const Ui::Window_t& parent)
:
    TableWindow_t(
        Broker::Window::Id::BrokerSalesLogTab,
        parent,
        L"BrokerSalesLogTab",
        WindowFlags,
        nullptr,
        0,
        DefaultTableOffset,
        InnerTableRect,
        DefaultTabOffset)
{
}

////////////////////////////////////////////////////////////////////////////////

} // SalesLog
} // Broker

////////////////////////////////////////////////////////////////////////////////
