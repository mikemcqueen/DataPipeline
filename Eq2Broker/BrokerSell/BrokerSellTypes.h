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
#include "Price_t.h"

namespace Broker::Sell {
  constexpr auto kWindowId = Window::Id::BrokerSell;
  constexpr auto kWindowName = "BrokerSell";
  constexpr auto kMsgName = "msg::broker_sell";

  namespace Table {
    constexpr auto RowCount = 10;
    constexpr auto CharsPerRow = 150;
    constexpr auto QuantityColumn = 0; // TODO: enum
    constexpr auto ItemNameColumn = 1;
    constexpr auto ListedColumn = 2;
    constexpr auto PriceColumn = 3;
    constexpr auto ColumnCount = 4;
    constexpr int CharColumnWidths[ColumnCount] = { 10, 100,  10, 30 };

    // TableInfo_t data: TODO: all zero width last entry
    constexpr int PixelColumnWidths[ColumnCount] = { 42, 782, 42, 105 };

    struct RowData_t {
      std::string item_name;
      int item_quantity;
      Price_t item_price;
      bool selected;
      bool item_listed;
      Rect_t rect;
      // std::unique_ptr<int> enforce_no_copies;
    };
    using RowVector = std::vector<RowData_t>;
  }

  class Text_t;
  using TextTable_t = TextTable3<Text_t>;

  namespace Translate {
    namespace Legacy {
      struct Data_t;
    }
    struct data_t;
    class Handler_t;
  } // Translate

  namespace Interpret {
    namespace Legacy {
      struct Data_t;
    }
    struct data_t;
    class Handler_t;
  } // Interpret

  class Window_t;
#if 0
  namespace Window {
    using ManagerBase_t = Ui::Window::Manager_t<
      Window_t, Translate::Handler_t, Interpret::Handler_t>;
    class Manager_t;
  }
#endif
} // namespace Broker::Sell

#endif // Include_BROKERSELLTYPES_H
