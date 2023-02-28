///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// BrokerSellText.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_BROKERSELLTEXT_H
#define Include_BROKERSELLTEXT_H

#include "BrokerSellTypes.h"
#include "Price_t.h"

namespace Broker::Sell {

  using TextBase_t = NewTextTableData_t<Table::RowCount, Table::CharsPerRow, Table::ColumnCount>;

  class Text_t : public TextBase_t {
  public:

    Text_t() = default;

    explicit Text_t(const TextBase_t& textBase) : TextBase_t(textBase)
    { }

    Text_t(span<const int> charsPerColumn) : TextBase_t(charsPerColumn)
    { }
  };
} // namespace Broker::Sell

#endif // Include_BROKERSELLTEXT_H

