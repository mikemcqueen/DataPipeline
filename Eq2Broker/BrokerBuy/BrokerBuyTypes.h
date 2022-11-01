////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// BrokerBuyTypes.h
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_BROKERBUYTYPES_H
#define Include_BROKERBUYTYPES_H

#include "UiWindowManager.h"
#include "TextTable_t.h"
#include "BrokerUi.h"

////////////////////////////////////////////////////////////////////////////////

namespace Broker::Buy {

constexpr Ui::WindowId_t TopWindowId = Broker::Window::Id::BrokerBuyTab;

namespace Table
{
    // TextTable_t data
    constexpr auto RowCount             = 8;
    constexpr auto CharsPerRow          = 245;
    constexpr auto QuantityColumn       = 0;
    constexpr auto ItemNameColumn       = 1;
    constexpr auto PriceColumn          = 2;
    constexpr auto LevelColumn          = 3;
    constexpr auto SellerNameColumn     = 4;
    constexpr auto ColumnCount          = 5;
    constexpr int CharColumnWidths[ColumnCount] = { 10, 100, 30, 5, 100 };

    // ScreenTable_t data: TODO: all zero width last entry
    constexpr int PixelColumnWidths[ColumnCount] = { 50, 560, 145, 50, 50 };
    // RowHeightPix = 40;
} // Table

class Text_t;
using TextTable_t = TextTable3<Text_t>;

namespace Translate
{
    struct Data_t;
    class Handler_t;
} // Translate

namespace Interpret
{
    struct Data_t;
    class Handler_t;
} // Interpret

class Window_t;
namespace Window
{
    using ManagerBase_t = Ui::Window::
        Manager_t<Window_t, Translate::Handler_t, Interpret::Handler_t> ;
    class Manager_t;
} // Window

} // Broker::Buy

#endif // Include_BROKERBUYTYPES_H
