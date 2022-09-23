////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// DcrBase_t.cpp
//
// EQ2 broker DCR base class.
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DcrBase_t.h"
#include "Charset_t.h"
#include "DdUtil.h"
#include "Log.h"
#include "Macros.h"
#include "BrokerUi.h"

#include "TextTable_t.h"

namespace Broker
{

/////////////////////////////////////////////////////////////////////////////

DcrBase_t::
DcrBase_t(
    int id,
    TextTable_i* pText,
    const TableWindow_t& tableWindow,
    const TableParams_t& tableParams,
    std::span<const int> columnWidths,
    std::span<const RECT> textRects)
    :
    DcrTable_t(
        id,
        pText,
        tableParams,
        columnWidths,
        textRects),
    m_tableWindow(
        tableWindow)
{ }

/////////////////////////////////////////////////////////////////////////////

DcrBase_t::
~DcrBase_t() = default;

////////////////////////////////////////////////////////////////////////////

void
test() {
    constexpr auto RowCount = 1;
    constexpr auto CharsPerRow = 100;
    constexpr auto ColumnCount = 5;
    constexpr int CharsPerColumn[] = { 5, 55, 10, 5, 25 };

    using td_t = NewTextTableData_t<RowCount, CharsPerRow, ColumnCount>;
    
    td_t td{ std::span{ CharsPerColumn } };

    td.fill();
    td.show();
}

////////////////////////////////////////////////////////////////////////////

bool
DcrBase_t::
Initialize()
{
    //test();
    return DcrTable_t::Initialize();
}

////////////////////////////////////////////////////////////////////////////////

size_t
DcrBase_t::
GetSelectedRow(
    CSurface& surface,
    const Rect_t&   tableRect) const
{
    auto rowHeight = GetScreenTable().GetRowHeight();
    const size_t width = 4;
    const size_t height = 1;
    Rect_t selectRect;
    selectRect.left = tableRect.left + tableRect.Width() / 2 - width / 2;
    selectRect.right = selectRect.left + width;
    selectRect.top = tableRect.top;
    selectRect.bottom = selectRect.top + height;
    size_t selectedRow = 0;
    for (size_t row = 1; selectRect.top + rowHeight <= tableRect.bottom; ++row)
    {
        using namespace Broker::Table; // HACK
        if (surface.CompareColorRange(selectRect, SelectedLowColor, SelectedHighColor))
        {
            if (0 < selectedRow)
            {
                LogError(L"DcrBase_t::GetSelectedRow() Two rows selected (%d,%d)",
                         selectedRow, row);
                return 0;
            }
            selectedRow = row;
        }
        ::OffsetRect(&selectRect, 0, rowHeight);
    }
    return selectedRow;
}

} // Broker

  ////////////////////////////////////////////////////////////////////////////////
