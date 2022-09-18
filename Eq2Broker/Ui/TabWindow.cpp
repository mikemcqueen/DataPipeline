////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// TabWindow.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TabWindow.h"
#include "BrokerUi.h" // todo: remove this dependency; tabwindow -> dp\ui; derive BrokerTabWindow
#include "UiInput.h"
#include "DdUtil.h"

using namespace Broker;

////////////////////////////////////////////////////////////////////////////////
//
// TabWindow_t
//
////////////////////////////////////////////////////////////////////////////////

TabWindow_t::
TabWindow_t(
    Ui::WindowId_t            WindowId,
    const Ui::Window_t&       parent,
    const wchar_t*            pWindowName,
          Flag_t              Flags,
    const Ui::Widget::Data_t* pWidgets,
          size_t              WidgetCount,
          POINT               tabOffset)
: 
    Ui::Window_t(
        WindowId,
        parent,
        pWindowName,
        Flags,
        pWidgets,
        WidgetCount),
    m_TabOffset(tabOffset)
{
}

////////////////////////////////////////////////////////////////////////////////

bool
TabWindow_t::
FindTab(
    const CSurface& surface,
    const Rect_t&   tabAreaRect,
    const CSurface& tabSurface,
    const POINT&    ptOrigin,
          POINT&    ptTabFound) const
{
    POINT ptHint =
    {
        ptOrigin.x + m_TabOffset.x,
        ptOrigin.y + m_TabOffset.y
    };
    if (surface.FindSurfaceInRect(tabSurface, tabAreaRect, ptTabFound, &ptHint))
    {
        // NOTE: if ptHint == ptFoundTab then Hint was successful
        m_TabOffset.x = ptTabFound.x - ptOrigin.x;
        m_TabOffset.y = ptTabFound.y - ptOrigin.y;
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
// TableWindow_t
//
////////////////////////////////////////////////////////////////////////////////

TableWindow_t::
TableWindow_t(
          Ui::WindowId_t      WindowId,
    const Ui::Window_t&       parent,
    const wchar_t*            pWindowName,
          Flag_t              Flags,
    const Ui::Widget::Data_t* pWidgets,
          size_t              widgetCount,
          POINT               tableOffset,
    const RECT&               innerTableRect,
          POINT               tabOffset)
:
    TabWindow_t(
        WindowId,
        parent,
        pWindowName,
        Flags,
        pWidgets,
        widgetCount,
        tabOffset),
    m_TableOffset(tableOffset),
    m_InnerTableRect(innerTableRect)
{
}

////////////////////////////////////////////////////////////////////////////////

Ui::WindowId_t
TableWindow_t::
GetWindowId(
    const CSurface& Surface,
    const POINT* pptHint) const
{
    if (nullptr == pptHint) {
        throw invalid_argument("TableWindow_t::GetWindowId()");
    }
    Rect_t TableRect;
    SIZE ScrollOffsets = { 0, 0 };
    if (FindTable(Surface, *pptHint, TableRect, ScrollOffsets))
    {
        m_TableRect = TableRect;
        POINT ptOrigin = { m_TableRect.left, m_TableRect.top };
        const_cast<TableWindow_t*>(this)->SetLastOrigin(ptOrigin);
        GetClientRect(m_ClientRect, m_TableRect, ScrollOffsets);
        return Ui::Window_t::GetWindowId();
    }
    return Ui::Window::Id::Unknown;
}

////////////////////////////////////////////////////////////////////////////////

bool
TableWindow_t::
GetWidgetRect(
    Ui::WidgetId_t WidgetId,
    Rect_t&        WidgetRect) const
{
    return Ui::Window_t::GetWidgetRect(WidgetId, GetTableRect(), WidgetRect);
}

////////////////////////////////////////////////////////////////////////////////

bool
TableWindow_t::
UpdateScrollPosition(
    Ui::Scroll::Bar_t ScrollBar,
    const CSurface&   Surface)
{
    using namespace Ui;
    // TODO: Rather imperfect solution, using GetTableRect():
    const Rect_t& TableRect = GetTableRect(); 
    if (Scroll::Bar::Vertical == ScrollBar)
    {
        if (GetFlags().Test(Ui::Window::Flag::VerticalScroll))
        {
            //TODO: maybe: all this rect discovery could live in GetVScrollPosition()
            Rect_t ScrollUpRect; // = m_VScrollUpRect.GetRelativeRect(TableRect);
            Rect_t ScrollDownRect; // = m_VScrollDownRect.GetRelativeRect(TableRect);
            if (Base_t::GetWidgetRect(Ui::Widget::Id::VScrollUp, TableRect, ScrollUpRect) &&
                Base_t::GetWidgetRect(Ui::Widget::Id::VScrollDown, TableRect, ScrollDownRect))
            {
                ::InflateRect(&ScrollUpRect, -2, -2);
                ::InflateRect(&ScrollDownRect, -2, -2);
                Scroll::Position_t ScrollPos = /*Ui::Window_t::*/GetVertScrollPos(Surface, ScrollUpRect, ScrollDownRect);
                //LogAlways(L"TableWindow_t::UpdateScrollPosition(): VScrollPos (%d)", ScrollPos);
                SetScrollPosition(ScrollBar, ScrollPos);
                return true;
            }
            LogError(L"%ls::UpdateScrollPosition(): Missing veritcal scroll rects",
                     GetWindowName());
            throw std::logic_error("TableWindow_t::UpdateScrollPosition(): Missing vertical scroll rects");
        }
    }
    else
    {
        //Flag = Flag::HorizontalScroll;
        //pScrollPos = &m_HorzScrollPos;
        throw std::logic_error("TableWindow_t::UpdateScrollPosition() horizontal scrolling not implemented");
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

bool
TableWindow_t::
ClickRow(
    size_t Row) const
{
    size_t RowHeight = Broker::Table::RowHeight + Broker::Table::GapSizeY;

    Rect_t RowRect = GetClientRect();
    RowRect.top += Row * RowHeight;
    int bottom = RowRect.top + RowHeight;
    if (bottom < RowRect.bottom)
    {
        RowRect.bottom = bottom;
    }
    // LogAlways(L"Row (%d) Rect = { %d, %d, %d, %d}", Row, RowRect.left, RowRect.top, RowRect.right, RowRect.bottom);
    Ui::Input_t Input;
    Input.Click(GetHwnd(), RowRect.Center());
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
TableWindow_t::
FindTable(
    const CSurface& Surface,
    const POINT&    ptTab,
          Rect_t&   TableRect,
          SIZE&     ScrollOffsets) const
{
    using namespace Broker;
    using namespace Broker::Table;
    POINT ptTable =
    {
        ptTab.x + m_TableOffset.x,
        ptTab.y + m_TableOffset.y
    };

    if ((0 < m_TableSize.cx) && (0 < m_TableSize.cy)) {
        Rect_t LastKnownTableRect(ptTable, m_TableSize);
        if (ValidateTable(Surface, LastKnownTableRect, ScrollOffsets))
        {
            LogInfo(L"%ls::FindTable(): Valid", GetWindowName());
            TableRect = LastKnownTableRect;
            return true;
        }
        // Don't clear table size. Just because we couldn't validate the table
        // doesn't mean it's not still the table size. There may be a another
        // window on top obscuring the table and preventing validation.
        //
        // m_TableSize.cx = 0;
        // m_TableSize.cy = 0;
    }

    DWORD Flags = 1;// set to 1 to debug
    const int TopBorderWidth = Surface.GetWidthInColorRange(
        ptTable.x, ptTable.y, BorderLowColor, BorderHighColor, Flags);
    if (MinTableWidth > TopBorderWidth)
    {
        LogInfo(L"%ls::FindTable(): TopBorderWidth(%d) < Min(%d) @ (%d, %d)",
                GetWindowName(),
                TopBorderWidth, MinTableWidth, ptTable.x, ptTable.y);
        return false;
    }
    // TODO: use CompareColorRange(Rect)
    // ValidateBrokerTableBorder()
    auto y = 1;
    for (; (y < BorderSize.cy) && (ptTable.y + y < int(Surface.GetHeight())); ++y)
    {
        const int BorderWidth = Surface.GetWidthInColorRange(
            ptTable.x, ptTable.y + y, BorderLowColor, BorderHighColor);
        if (BorderWidth != TopBorderWidth)
        {
            LogInfo(L"%ls::FindTable(): Top BorderWidth(%d) @ (%d, %d) Row(%d) TopBorderWidth(%d)",
                GetWindowName(),
                BorderWidth, ptTable.x, ptTable.y + y, y, TopBorderWidth);
            return false;
        }
    }
    if (y < BorderSize.cy)
    {
        LogInfo(L"%ls::FindTable(): top border out of view?",
                GetWindowName());
        return false;
    }
    auto tableHeight = 0;
    for (; ptTable.y + y < int(Surface.GetHeight()); ++tableHeight, ++y)
    {
        const int borderWidth = Surface.GetWidthInColorRange(
            ptTable.x, ptTable.y + y, BorderLowColor, BorderHighColor);
        if (borderWidth < BorderSize.cx)
        {
            LogInfo(L"%ls::FindTable(): Left BorderWidth (%d) < %d, tableHeight (%d) @ (%d, %d)",
                    GetWindowName(),
                    borderWidth, BorderSize.cx, tableHeight, ptTable.x, ptTable.y + y);
            return false;
        }
        if (borderWidth == TopBorderWidth)
            break;
    }
    if (tableHeight < 10) {
        LogInfo(L"%ls::FindTable(): small tableHeight (%d)",
            GetWindowName(), tableHeight);
        return false;
    }
    // ValidateBrokerTableBorder()
    auto bottomBorderHeight = 0;
    for (; (bottomBorderHeight < BorderSize.cy) && (ptTable.y + y < int(Surface.GetHeight())); ++bottomBorderHeight, ++y)
    {
        const int borderWidth = Surface.GetWidthInColorRange(
            ptTable.x, ptTable.y + y, BorderLowColor, BorderHighColor, Flags);
        if (borderWidth != TopBorderWidth)
        {
            LogInfo(L"%ls::FindTable(): Bottom BorderWidth (%d) @ (%d, %d) Row (%d) TopBorderWidth (%d)",
                    GetWindowName(),
                    borderWidth, ptTable.x, ptTable.y + y, y, TopBorderWidth);
            return false;
        }
    }
    if (bottomBorderHeight < BorderSize.cy)
    {
        LogInfo(L"%ls::FindTable(): bottom border out of view? height (%d) of (%d)",
            GetWindowName(), bottomBorderHeight, BorderSize.cy);
        return false;
    }
    SIZE TableSize = { TopBorderWidth, tableHeight + BorderSize.cy * 2 };
    Rect_t ProposedTableRect(ptTable, TableSize);
    //if (ValidateClient(Surface, ProposedTableRect, ScrollOffsets))
    {
        m_TableSize = TableSize;
        TableRect = ProposedTableRect;
        return true;
    }
    //return false;
}

////////////////////////////////////////////////////////////////////////////////

void
TableWindow_t::
GetClientRect(
          Rect_t& ClientRect,
    const Rect_t& TableRect,
    const SIZE&   ScrollOffsets) const
{
    using namespace Broker::Table;
    const int x = TableRect.left + BorderSize.cx + m_InnerTableRect.left + ScrollOffsets.cx;
    const int y = TableRect.top  + BorderSize.cy + m_InnerTableRect.top  + ScrollOffsets.cy;
    ::SetRect(&ClientRect, x, y,
            x + TableRect.Width() - DoubleBorderSize.cx - ScrollOffsets.cx -
                m_InnerTableRect.left - m_InnerTableRect.right,
            y + TableRect.Height() - DoubleBorderSize.cy - ScrollOffsets.cy -
                m_InnerTableRect.top - m_InnerTableRect.bottom);
}

////////////////////////////////////////////////////////////////////////////////

bool
TableWindow_t::
ValidateTable(
    const CSurface& Surface,
    const Rect_t&   TableRect,
          SIZE&     ScrollOffsets) const
{
    return ValidateBorders(Surface, TableRect) && 
           ValidateClient(Surface, TableRect, ScrollOffsets);
}

////////////////////////////////////////////////////////////////////////////////

bool
TableWindow_t::
ValidateBorders(
    const CSurface& Surface,
    const Rect_t&   TableRect) const
{
    using namespace Broker::Table;

    Rect_t BorderRect = TableRect;
    BorderRect.bottom = BorderRect.top + BorderSize.cy;
    if (!ValidateBorder(Surface, BorderRect, L"Top"))
    {
        return false;
    }
    OffsetRect(&BorderRect, 0, TableRect.Height() - BorderSize.cy);
    if (!ValidateBorder(Surface, BorderRect, L"Bottom"))
    {
        return false;
    }

    BorderRect = TableRect;
    BorderRect.right = BorderRect.left + BorderSize.cx,
    BorderRect.top += BorderSize.cy;
    BorderRect.bottom -= BorderSize.cy;
    if (!ValidateBorder(Surface, BorderRect, L"Left"))
    {
        return false;
    }
    OffsetRect(&BorderRect, TableRect.Width() - BorderSize.cx, 0);
    if (!ValidateBorder(Surface, BorderRect, L"Rignt"))
    {
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
TableWindow_t::
ValidateBorder(
    const CSurface& Surface,
    const Rect_t&   BorderRect,
    const wchar_t*  pBorderName) const
{
    if (!Surface.CompareColorRange(BorderRect, BorderLowColor, BorderHighColor))
    {
        LogWarning(L"%ls::ValidateBorder(): %ls border no longer matches",
                   GetWindowName(), pBorderName);
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
TableWindow_t::
ValidateClient(
    const CSurface& Surface,
    const Rect_t&   TableRect,
          SIZE&     ScrollOffsets) const
{
    using namespace Broker::Table;

    ScrollOffsets.cx = ScrollOffsets.cy = 0;
    // Get default client rect with no scroll offsets
    Rect_t ClientRect;
    GetClientRect(ClientRect, TableRect, ScrollOffsets);
    // Get the scroll offsets
    GetScrollOffsets(Surface, ClientRect, ScrollOffsets);
    // Get the real client rect using real scroll offsets
    Rect_t GridRect;
    GetClientRect(GridRect, TableRect, ScrollOffsets);
    Surface.WriteBMP(L"diag\\table_gridrect.bmp", GridRect);
    const int Bottom = GridRect.bottom;
    GridRect.bottom = GridRect.top + GapSizeY;
    for (size_t Line = 0; Bottom >= GridRect.bottom; ++Line)
    {
//        if (!Surface.CompareColorRange(GridRect, BkLowColor, BkHighColor))
        if (!Surface.CompareColor(GridRect, Black))
        {
            LogWarning(L"%ls::ValidateClient(): Line (%d) @ (%d, %d) doesn't match",
                       GetWindowName(), Line, GridRect.left, GridRect.top);
            return false;
        }
        OffsetRect(&GridRect, 0, RowHeight + GapSizeY);
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

void
TableWindow_t::
GetScrollOffsets(
    const CSurface&   /*Surface*/,
    const Rect_t&     /*TableRect*/,
          SIZE&       ScrollOffsets) const
{
    ScrollOffsets.cx = ScrollOffsets.cy = 0;
}

////////////////////////////////////////////////////////////////////////////////
