////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// SetPriceWindow.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SetPriceWindow.h"
#include "BrokerUi.h"
#include "DdUtil.h"
#include "Log.h"
#include "MainWindow_t.h"

namespace Broker
{
namespace SetPrice
{

////////////////////////////////////////////////////////////////////////////////

static const Flag_t WindowFlags   = 0;

static const SIZE   WindowSize    = { 254, 213 }; // 273,203
static const SIZE   BorderSize    = { 1, 1 };

static const SIZE   PriceTextSize = { 140, 16 };
static const SIZE   ButtonSize    = { 10, 10 };

-188, 6 - pricetext offset from OK
-72, 139 - clear button offset fom OK

// ok (text) size 23x10
// clear (text) size 8x8
// 10 x 10 probably ok in both cases

static const int Row[5]        = { 33, 76, 108, 140, 172 };
static const int Col[4]        = { 39, 89, 139, 200 };

static const
struct Ui::Widget::Data_t s_Widgets[] =
{
    { Widget::Id::PriceText,      { RelativeRect_t::LeftTop,     23, Row[0], PriceTextSize.cx, PriceTextSize.cy } },
    { Widget::Id::OkButton,       { RelativeRect_t::LeftTop, Col[3], Row[0], ButtonSize.cx, ButtonSize.cy } },

    { Widget::Id::OneButton,      { RelativeRect_t::LeftTop, Col[0], Row[1], ButtonSize.cx, ButtonSize.cy } },
    { Widget::Id::TwoButton,      { RelativeRect_t::LeftTop, Col[1], Row[1], ButtonSize.cx, ButtonSize.cy } },
    { Widget::Id::ThreeButton,    { RelativeRect_t::LeftTop, Col[2], Row[1], ButtonSize.cx, ButtonSize.cy } },
    { Widget::Id::PlatinumButton, { RelativeRect_t::LeftTop, Col[3], Row[1], ButtonSize.cx, ButtonSize.cy } },

    { Widget::Id::FourButton,     { RelativeRect_t::LeftTop, Col[0], Row[2], ButtonSize.cx, ButtonSize.cy } },
    { Widget::Id::FiveButton,     { RelativeRect_t::LeftTop, Col[1], Row[2], ButtonSize.cx, ButtonSize.cy } },
    { Widget::Id::SixButton,      { RelativeRect_t::LeftTop, Col[2], Row[2], ButtonSize.cx, ButtonSize.cy } },
    { Widget::Id::GoldButton,     { RelativeRect_t::LeftTop, Col[3], Row[2], ButtonSize.cx, ButtonSize.cy } },

    { Widget::Id::SevenButton,    { RelativeRect_t::LeftTop, Col[0], Row[3], ButtonSize.cx, ButtonSize.cy } },
    { Widget::Id::EightButton,    { RelativeRect_t::LeftTop, Col[1], Row[3], ButtonSize.cx, ButtonSize.cy } },
    { Widget::Id::NineButton,     { RelativeRect_t::LeftTop, Col[2], Row[3], ButtonSize.cx, ButtonSize.cy } },
    { Widget::Id::SilverButton,   { RelativeRect_t::LeftTop, Col[3], Row[3], ButtonSize.cx, ButtonSize.cy } },

    { Widget::Id::ZeroButton,     { RelativeRect_t::LeftTop, (Col[0] + Col[1]) / 2, Row[4], ButtonSize.cx, ButtonSize.cy } },
    { Widget::Id::ClearButton,    { RelativeRect_t::LeftTop, Col[2], Row[4], ButtonSize.cx, ButtonSize.cy } },
    { Widget::Id::CopperButton,   { RelativeRect_t::LeftTop, Col[3], Row[4], ButtonSize.cx, ButtonSize.cy } }
};

////////////////////////////////////////////////////////////////////////////////

Window_t::Window_t(const Ui::Window_t& parent) :
  Ui::Window_t(
    Broker::Window::Id::BrokerSetPricePopup,
    parent,
    L"SetPricePopup",
    WindowFlags,
    s_Widgets,
    _countof(s_Widgets))
{
}

bool Window_t::GetWidgetRect(Ui::WidgetId_t  WidgetId, Rect_t&   WidgetRect) const {
  return Ui::Window_t::GetWidgetRect(WidgetId, MainWindow_t::GetPopupRect(), WidgetRect);
}

bool
Window_t::
FindBorder(
  const CSurface& Surface,
  const POINT& ptTab,
  Rect_t& SurfaceRect) const
{
  // TODO: could use MainWindow_t::GetPopupRect() here
  static POINT SetPriceOffset = { 0, 0 };
  if ((0 < SetPriceOffset.x) && (0 < SetPriceOffset.y))
  {
    POINT ptPopup =
    {
        ptTab.x + SetPriceOffset.x,
        ptTab.y + SetPriceOffset.y
    };
    Rect_t PopupRect(ptPopup, WindowSize);
    if (Ui::Window_t::ValidateBorders(Surface, PopupRect, BorderSize,
      BorderLowColor, BorderHighColor))
    {
      // TODO: additional validation
      LogInfo(L"%S::FindSetPricePopup(): Valid", GetWindowName());
      SurfaceRect = PopupRect;
      return true;
    }
  }
  // Search for top border
  // Explain this math to me like i'm 5.
  // 1. Rect = upper left quadrant.
  Rect_t SearchRect;
  SearchRect = Surface.GetBltRect();
  SearchRect.right /= 2;
  SearchRect.bottom /= 2;
  // 2. Rect = lower left quadrant (??)
  OffsetRect(&SearchRect, 0, SearchRect.bottom);
  // 3. W.T.F. Rect is some tiny sliver at the top left of the lower left quadrantt. (????)
  SearchRect.right -= WindowSize.cx;
  SearchRect.bottom -= WindowSize.cy;
#if 0
  static int num = 0;
  if (!num++) {
    Surface.WriteBMP(L"diag\\SetPriceWindow_search.bmp", SearchRect);
  }
#endif
  for (int Line = SearchRect.top; Line < SearchRect.bottom; ++Line) {
    for (int Pixel = SearchRect.left; Pixel < SearchRect.right; ++Pixel) {
      Rect_t BorderRect(Pixel, Line,
        Pixel + WindowSize.cx,
        Line + BorderSize.cy);
      if (Surface.CompareColorRange(BorderRect, BorderLowColor, BorderHighColor)) {
        POINT ptPopup = { Pixel, Line };
        Rect_t PopupRect(ptPopup, WindowSize);
        if (Ui::Window_t::ValidateBorders(Surface, PopupRect, BorderSize,
          BorderLowColor, BorderHighColor))
        {
          // TODO: additional validation
          LogInfo(L"%S::FindSetPricePopup(): Found @ (%d, %d)",
            GetWindowName(), ptPopup.x, ptPopup.y);
          SetPriceOffset.x = ptPopup.x - ptTab.x;
          SetPriceOffset.y = ptPopup.y - ptTab.y;
          SurfaceRect = PopupRect;
          return true;
        }
        break;
      }
    }
  }
  return false;
}

} // SetPrice
} // Broker
