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

namespace Broker
{
namespace Buy
{
    static const Ui::WindowId_t TopWindowId = Broker::Window::Id::BrokerBuyTab;

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
        static const size_t CharColumnWidths[ColumnCount] = { 10, 100, 30, 5, 100 };

        // ScreenTable_t data:
        static const int PixelColumnWidths[ColumnCount] = { 50, 560, 145, 50, 0 };
        // RowHeightPix = 40;
    }

    class Text_t;
    typedef TextTable2<Text_t> TextTable_t;

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
        typedef Ui::Window::Manager_t<
                    Window_t,
                    Translate::Handler_t,
                    Interpret::Handler_t> ManagerBase_t;
        class Manager_t;
    } // Window

} // Buy
} // Broker

////////////////////////////////////////////////////////////////////////////////

#endif // Include_BROKERBUYTYPES_H

////////////////////////////////////////////////////////////////////////////////
