///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// BrokerSellTypes.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_BROKERSELLTYPES_H
#define Include_BROKERSELLTYPES_H

#include "UiWindowManager.h"
#include "TextTable_t.h"
#include "BrokerUi.h"

///////////////////////////////////////////////////////////////////////////////

namespace Broker
{
namespace Sell
{
    static const Ui::WindowId_t TopWindowId = Broker::Window::Id::BrokerSellTab;

    namespace Table
    {
        static const size_t RowCount             = 7;
        static const size_t CharsPerRow          = 150;
        static const size_t QuantityColumn       = 0;
        static const size_t ItemNameColumn       = 1;
        static const size_t ListedColumn         = 2;
        static const size_t PriceColumn          = 3;
        static const size_t ColumnCount          = 4;
        static const size_t CharColumnWidths[ColumnCount] =  { 10, 100,  10, 30 };

        // ScreenTable_t data
        static const size_t PixelColumnWidths[ColumnCount] = { 44, 0, 48, 100 };
    }

    typedef TextTableData_t<Table::RowCount, Table::CharsPerRow, Table::ColumnCount> Text_t;
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
    }

} // Sell
} // Broker

///////////////////////////////////////////////////////////////////////////////

#endif // Include_BROKERSELLTYPES_H

///////////////////////////////////////////////////////////////////////////////
