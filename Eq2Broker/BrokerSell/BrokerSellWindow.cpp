////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// BrokerSellWindow.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainWindow_t.h"
#include "BrokerSellWindow.h"
#include "BrokerSellTypes.h"
#include "BrokerWindow.h"
//#include "SetPriceWindow.h"
#include "DdUtil.h"
#include "Log.h"

namespace Broker::Sell {
  const Flag_t WindowFlags = Ui::Window::Flag::VerticalScroll;

  constexpr POINT  BrokerDefaultTabOffset = { -3,  41 };
  constexpr POINT  BrokerTabToTableOffset = { -66, 178 };
  constexpr POINT  MarketDefaultTabOffset = { 17,  41 };
  constexpr POINT  MarketDefaultTableOffset = { -84, 174 };
  constexpr RECT   InnerTableRect = { 0, 2, 0, 0 };
  constexpr SIZE   ButtonSize = { 106, 15 };

  constexpr Ui::Widget::Data_t brokerWidgets[] = {
    { Frame::Widget::Id::BuyTab,      { RelativeRect_t::LeftTop,     9,   -174,  9,  9 } }, // Broker Buy
    //{ Frame::Widget::Id::SalesLogTab, { RelativeRect_t::LeftTop,     123, -174,  9,  9 } }, // NOTE: BROKEN
  };

  constexpr Ui::Widget::Data_t marketWidgets[] = {
    { Frame::Widget::Id::BuyTab,      { RelativeRect_t::LeftTop,     10, -174,  9,  9 } }, // Market Browse
    //{ Frame::Widget::Id::SalesLogTab, { RelativeRect_t::LeftTop,     145, -174,  9,  9 } }, // NOTE: BROKEN
  };

  constexpr Ui::Widget::Data_t Widgets[] = {
    { Ui::Widget::Id::VScrollUp,    { RelativeRect_t::RightTop,     -22,   12, 15, 17 } },
    { Ui::Widget::Id::VScrollDown,  { RelativeRect_t::RightBottom,  -22,  -25, 15, 17 } },
    { Widget::Id::SetPriceButton,   { RelativeRect_t::LeftBottom,    16,   35, ButtonSize.cx, ButtonSize.cy } },
    { Widget::Id::ListItemButton,   { RelativeRect_t::LeftBottom,   140,   35, ButtonSize.cx, ButtonSize.cy } },
    { Widget::Id::SearchButton,     { RelativeRect_t::LeftBottom,   265,   35, ButtonSize.cx, ButtonSize.cy } },
    { Widget::Id::RemoveItemButton, { RelativeRect_t::RightBottom, -115,   35, ButtonSize.cx, ButtonSize.cy } },
  };

  Window_t::Window_t(const Ui::Window_t& parent) :
    TableWindow_t(
      kWindowId,
      parent,
      kWindowName,
      {}, // TODO { setprice_window_ },
      WindowFlags,
      std::span{ Widgets },
      BrokerTabToTableOffset,
      InnerTableRect,
      BrokerDefaultTabOffset)
  {
  }

  const Broker::Window_t& Window_t::GetBrokerWindow() const {
    return dynamic_cast<const Broker::Window_t&>(GetParent());
  }

  Ui::WindowId_t Window_t::GetWindowId(
    const CSurface& Surface,
    const POINT* pptHint) const
  {
    Ui::WindowId_t WindowId = TableWindow_t::GetWindowId(Surface, pptHint); // , SurfaceRect);
    if (Ui::Window::Id::Unknown != WindowId)
    {
      return WindowId;
    }
    // State: the broker sell tab window is active, but something is preventing
    //        its table from being validated.
    Rect_t popupRect;
    if (nullptr == pptHint)
    {
      throw invalid_argument("BrokerSellWindow::GetWindowId()");
    }
    /*
    if (GetMainWindow().GetSetPricePopup().FindBorder(Surface, *pptHint, popupRect))
    {
      WindowId = Broker::Window::Id::BrokerSetPricePopup;
      GetMainWindow().SetPopupRect(popupRect);
    }
    */
    return WindowId;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // NOTE: Called from acquire handler before any image pre-processing occurs
  // NOTE: i doubt that this is truely called from acquire handler anymore
  void Window_t::GetScrollOffsets(
    const CSurface& Surface,
    const Rect_t& TableRect,
    SIZE& ScrollOffsets) const
  {
    bool bExtraLog = false;

    ScrollOffsets.cx = ScrollOffsets.cy = 0;
    // Determine the vertical scroll offset by looking for the first line of
    // non-black pixels within the "Listed" column.
    // TODO: ScreenTable_t should probably support "GetColumnRect(ColumnName)"
    using namespace Table;
    Rect_t ListedRect = TableRect;
    ListedRect.left = ListedRect.right - PixelColumnWidths[PriceColumn]
      - PixelColumnWidths[ListedColumn];
    ListedRect.right = ListedRect.left + PixelColumnWidths[ListedColumn];
    ListedRect.bottom = ListedRect.top + 1;
    //LogAlways(L"ListedRect = { %d, %d, %d, %d }", ListedRect.left, ListedRect.top, ListedRect.right, ListedRect.bottom);
    constexpr auto DefaultOffsetLine = 19;
    constexpr auto MaxCheckLines = Broker::Table::RowHeight * 2;
    for (int Line = 0; Line < MaxCheckLines;) {
      int skipLines = 1;
      if (!Surface.CompareColorRange(ListedRect, BkLowColor, BkHighColor)) {
        // this says "don't recognize first table row if it is even 1 pixel
        // scrolled up (which is probably a little harsh)
        if (DefaultOffsetLine <= Line) {
          ScrollOffsets.cy = Line - DefaultOffsetLine;
          if (bExtraLog) {
            //Surface.WriteBMP(L"Diag\\BrokerSellWindow_ListedRect.bmp", ListedRect);
            LogAlways(L"BrokerSellWindow::GetScrollOffsets(): Match @ Line(%d) Y(%d) OffsetY(%d)",
              Line, TableRect.top + Line, ScrollOffsets.cy);
          }
          break;
        } else {
          // we found non-background-color (text) pixels, but at too early
          // a line, so skip over this line of text - since we know this is
          // always a single line of text it's skipping over the entire row m
          skipLines = Broker::Table::CharHeight + Broker::Table::RowGapSize;
        }
      }
      OffsetRect(&ListedRect, 0, skipLines);
      Line += skipLines;
    }
  }

  bool Window_t::GetWidgetRect(
    Ui::WidgetId_t widgetId,
    Rect_t* rect) const
  {
    if ((Frame::Widget::Id::BuyTab == widgetId) ||
      (Frame::Widget::Id::SalesLogTab == widgetId))
    {
      switch (GetBrokerWindow().GetLayout()) {
      case Frame::Layout::Broker:
        return Ui::Window_t::GetWidgetRect(widgetId, GetTableRect(), rect,
          std::span{ brokerWidgets });
      case Frame::Layout::Market:
        return Ui::Window_t::GetWidgetRect(widgetId, GetTableRect(), rect,
          std::span{ marketWidgets });
      default:
        throw logic_error("BrokerBuyWindow::GetWidgetRect() Invalid layout");
      }
    } else {
      return TableWindow_t::GetWidgetRect(widgetId, rect);
    }
  }

  void Window_t::SetLayout(Frame::Layout_t layout) const {
    switch (layout) {
    case Frame::Layout::Broker:
      SetOffsets(BrokerDefaultTabOffset, BrokerTabToTableOffset);
      break;
    case Frame::Layout::Market:
      SetOffsets(MarketDefaultTabOffset, MarketDefaultTableOffset);
      break;
    default:
      throw logic_error("BrokerSellWindow::SetLayout() Invalid layout");
    }
  }

} // namespace Broker::Sell
