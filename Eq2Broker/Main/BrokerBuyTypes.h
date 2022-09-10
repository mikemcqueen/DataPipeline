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
        static const size_t RowCount             = 8;
        static const size_t CharsPerRow          = 245;
        static const size_t QuantityColumn       = 0;
        static const size_t ItemNameColumn       = 1;
        static const size_t PriceColumn          = 2;
        static const size_t LevelColumn          = 3;
        static const size_t SellerNameColumn     = 4;
        static const size_t ColumnCount          = 5;
        static const size_t CharColumnWidths[ColumnCount] = { 10, 100, 30, 5, 100 };

        // ScreenTable_t data:
        static const int PixelColumnWidths[ColumnCount] = { 50, 273, 88, 100, 0 };
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
