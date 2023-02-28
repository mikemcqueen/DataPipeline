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

namespace Broker::Sell {
  constexpr auto kWindowId = Broker::Window::Id::BrokerSell;
  constexpr auto kWindowName = "BrokerSell"sv;

  namespace Table {
    constexpr auto RowCount = 7;
    constexpr auto CharsPerRow = 150;
    constexpr auto QuantityColumn = 0;
    constexpr auto ItemNameColumn = 1;
    constexpr auto ListedColumn = 2;
    constexpr auto PriceColumn = 3;
    constexpr auto ColumnCount = 4;
    constexpr int CharColumnWidths[ColumnCount] = { 10, 100,  10, 30 };

    // TableInfo_t data: TODO: all zero width last entry
    constexpr int PixelColumnWidths[ColumnCount] = { 44, 0, 48, 100 };
  }

  class Text_t;
  using TextTable_t = TextTable3<Text_t>;

  namespace Translate {
    struct Data_t;
    class Handler_t;
  } // Translate

  namespace Interpret {
    struct Data_t;
    class Handler_t;
  } // Interpret

  class Window_t;
  namespace Window {
    using  ManagerBase_t = Ui::Window::Manager_t<
      Window_t, Translate::Handler_t, Interpret::Handler_t>;
    class Manager_t;
  }
} // namespace Broker::Sell

#endif // Include_BROKERSELLTYPES_H
