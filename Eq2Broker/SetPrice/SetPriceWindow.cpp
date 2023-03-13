////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// SetPriceWindow.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SetPriceWindow.h"
#include "SetPriceTypes.h"
#include "SetPriceWidgets.h"
#include "BrokerUi.h"
#include "DdUtil.h"
#include "Log.h"
#include "resource.h"

namespace Broker::SetPrice {
  constexpr Flag_t kWindowFlags = 0;

  constexpr SIZE   WindowSize = { 254, 213 }; // 273,203
  constexpr SIZE   BorderSize = { 1, 1 };

  Window_t::Window_t(const Ui::Window_t& parent) :
    Ui::Window_t(
      kWindowId,
      parent,
      kWindowName,
      {},
      kWindowFlags,
      std::span{ Widgets })
  {
    LoadSurfaces();
  }

  void Window_t::LoadSurfaces() {
    extern CDisplay* g_pDisplay;
    HRESULT hr = g_pDisplay->CreateSurfaceFromBitmap(&origin_surface_,
      MAKEINTRESOURCE(IDB_SETPRICE_OKBUTTON));
    if (FAILED(hr)) {
      throw runtime_error("SetPriceWindow(): Create SetPrice_OkButton surface");
    }
  }

  void Window_t::GetOriginSearchRect(
    const CSurface& surface,
    Rect_t& rect) const
  {
    // should be using ClientRect here i think, but they're equal for now
    rect = surface.GetBltRect();
    // 2nd quartile horizontally
    rect.left += rect.right / 4;
    rect.right /= 2;
    // bottom 3rd
    rect.top += rect.bottom * 2 /  3;
  }

  bool Window_t::FindBorder(
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
} // namespace Broker::SetPrice
