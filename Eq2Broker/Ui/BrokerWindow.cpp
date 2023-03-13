////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// BrokerWindow.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BrokerWindow.h"
#include "BrokerBuyWindow.h"
#include "BrokerSellWindow.h"
//#include "BrokerSalesLogTab.h"
#include "BrokerUi.h"
#include "Log.h"
#include "Rect.h"
#include "MainWindow_t.h"
#include "Resource.h"

namespace Broker {
  constexpr Flag_t kWindowFlags{};

  Window_t::Window_t(const Ui::Window_t& parent) :
    Ui::Window_t(
      Broker::Window::Id::BrokerFrame,
      parent,
      kWindowName,
      { buy_window_, sell_window_ },
      kWindowFlags),
    buy_window_(*this),
    sell_window_(*this),
    m_layout(Frame::Layout::Unknown)
  {
    LoadSurfaces();
  }

  void Window_t::LoadSurfaces() {
    //TODO: array
    extern CDisplay* g_pDisplay;
    HRESULT hr = S_OK;
    hr = g_pDisplay->CreateSurfaceFromBitmap(&m_brokerCaption,
      MAKEINTRESOURCE(IDB_BROKER_CAPTION));
    if (FAILED(hr)) {
      throw runtime_error("Create BrokerCaption surface");
    }
    hr = g_pDisplay->CreateSurfaceFromBitmap(&m_buyTabActive,
      MAKEINTRESOURCE(IDB_BROKER_BUYTAB_ACTIVE));
    if (FAILED(hr)) {
      throw runtime_error("Create BrokerBuyTabActive surface");
    }
    hr = g_pDisplay->CreateSurfaceFromBitmap(&m_buyTabInactive,
      MAKEINTRESOURCE(IDB_BROKER_BUYTAB_INACTIVE));
    if (FAILED(hr)) {
      throw runtime_error("Create BrokerBuyTabInactive surface");
    }
    hr = g_pDisplay->CreateSurfaceFromBitmap(&m_marketCaption,
      MAKEINTRESOURCE(IDB_MARKET_CAPTION));
    if (FAILED(hr)) {
      throw runtime_error("Create MarketCaption surface");
    }
    hr = g_pDisplay->CreateSurfaceFromBitmap(&m_browseTabActive,
      MAKEINTRESOURCE(IDB_MARKET_BROWSETAB_ACTIVE));
    if (FAILED(hr)) {
      throw runtime_error("Create BrowseTabActive surface");
    }
    hr = g_pDisplay->CreateSurfaceFromBitmap(&m_browseTabInactive,
      MAKEINTRESOURCE(IDB_MARKET_BROWSETAB_INACTIVE));
    if (FAILED(hr)) {
      throw runtime_error("Create BrowseTabInactive surface");
    }
    hr = g_pDisplay->CreateSurfaceFromBitmap(&m_sellTabActive,
      MAKEINTRESOURCE(IDB_BROKER_SELLTAB_ACTIVE));
    if (FAILED(hr)) {
      throw runtime_error("Create BrokerSellTabActive surface");
    }
    hr = g_pDisplay->CreateSurfaceFromBitmap(&m_sellTabInactive,
      MAKEINTRESOURCE(IDB_BROKER_SELLTAB_INACTIVE));
    if (FAILED(hr)) {
      throw runtime_error("Create BrokerSellTabInactive surface");
    }
    hr = g_pDisplay->CreateSurfaceFromBitmap(&m_salesLogTabActive,
      MAKEINTRESOURCE(IDB_BROKER_SALESLOGTAB_ACTIVE));
    if (FAILED(hr)) {
      // throw runtime_error("Create BrokerSalesLogTabActive surface");
    }
    hr = g_pDisplay->CreateSurfaceFromBitmap(&m_salesLogTabInactive,
      MAKEINTRESOURCE(IDB_BROKER_SALESLOGTAB_INACTIVE));
    if (FAILED(hr)) {
      // throw runtime_error("Create BrokerSalesLogTabInactive surface");
    }
  }

  const TabWindow_t& Window_t::GetTabWindow(Tab_t Tab) const {
    using namespace Broker::Window;
    switch (Tab) {
    case Tab::Id::Buy:  return GetBrokerBuyWindow();
    case Tab::Id::Sell: return GetBrokerSellWindow();
      //case Tab::Id::SalesLog: return static_cast<TabWindow_t&>(GetWindow(Id::BrokerSalesLog));
    default:
      throw std::invalid_argument("BrokerWindow::GetTabWindow()");
    }
  }

  ////////////////////////////////////////////////////////////////////////////////
  // Get the window id of the topmost window on the supplied surface.
  // Determine if the supplied surface has a broker window up, and if so, which
  // tab is active. Pass the surface to the active tab window class to determine
  // what the active window is in the context of that tab (e.g. maybe a set price
  // popup is active in the sell tab).

  Ui::WindowId_t Window_t::GetWindowId(
    const CSurface& surface,
    const POINT* /* pptHint = nullptr */) const
  {
    using namespace Ui::Window;
    auto windowId = Id::Unknown;
    POINT ptCaption;
    if (IsLocatedOn(surface, LocateBy::AnyMeans, &ptCaption)) {
      POINT ptTab;
      Tab_t Tab = FindActiveTab(surface, ptCaption, ptTab);
      if (Tab::Id::None != Tab) {
        const Ui::Window_t& tabWindow = GetTabWindow(Tab);
        LogInfo(L"BrokerWindow::GetWindowId() Found %S Tab @ (%d, %d)",
          tabWindow.GetWindowName(), ptTab.x, ptTab.y);
        // Now that we have verified the tab is active, validate
        // the table, or determine if a popup is active
        windowId = tabWindow.GetWindowId(surface, &ptTab);
      }
    }
    return windowId;
  }

  bool Window_t::IsLocatedOn(
    const CSurface& surface,
    Flag_t flags,
    POINT* pptOrigin) const
  {
    static const CSurface* pLastCaption = nullptr;
    static const CSurface Window_t::* captions[] = { // TODO: std::array
        &Window_t::m_marketCaption,
        &Window_t::m_brokerCaption,
    };

    using namespace Ui::Window;
    if (flags.Test(LocateBy::LastOriginMatch) && pLastCaption
      && CompareLastOrigin(surface, *pLastCaption, pptOrigin))
    {
      return true;
    }
    if (flags.Test(LocateBy::OriginSearch)) {
      // I'm trying to be wayy too clever here
      size_t index = 0;
      const CSurface* pCaption = pLastCaption ? pLastCaption :
        &(this->*captions[index++]);
      for (;;) {
        // Look for caption bitmap on the supplied surface
        if ((!index || (pCaption != pLastCaption)) &&
          OriginSearch(surface, *pCaption, pptOrigin))
        {
          SetLayout((pCaption == &m_marketCaption) ? Frame::Layout::Market
            : Frame::Layout::Broker);
          pLastCaption = pCaption;
          return true;
        }
        if (_countof(captions) == index) {
          break;
        }
        pCaption = &(this->*captions[index++]);
      }
    }
    return false;
  }

  void Window_t::GetOriginSearchRect(
    const CSurface& surface,
    Rect_t& rect) const
  {
    // search the upper left 1/9 of the screen
    rect = surface.GetBltRect();
    rect.right /= 3;
    rect.bottom /= 3;
  }

#if 1
  void Window_t::SetLayout(Frame::Layout_t layout) const {
    if (layout != m_layout) {
      m_layout = layout;
      GetBrokerBuyWindow().SetLayout(layout);
      GetBrokerSellWindow().SetLayout(layout);
      //mainWindow.GetBrokerSalesLogWindow().SetLayout(layout);
    }
  }
#endif

  Tab_t Window_t::FindActiveTab(
    const CSurface& surface,
    const POINT     ptOrigin,
    POINT& ptFoundTab) const
  {
    struct TabSurface_t {
      Tab_t           tab;
      const CSurface* pSurface;
    };
    static TabSurface_t lastTab = { Tab::Id::None, nullptr };

    // First, try the last known tab window
    if (Tab::Id::None != lastTab.tab) {
      if (nullptr != lastTab.pSurface) {
        if (GetTabWindow(lastTab.tab).FindTab(surface, GetTabAreaRect(ptOrigin),
          *lastTab.pSurface, ptOrigin, ptFoundTab))
        {
          return lastTab.tab;
        }
      }
      else {
        throw logic_error("BrokerWindow::FindActiveTab() lastTab.pSurface is nullptr");
      }
    }
    // Next, try all the other tabs
    const CSurface* pBuyTab = nullptr;
    switch (m_layout) {
    case Frame::Layout::Broker:
      pBuyTab = &m_buyTabActive;
      break;
    case Frame::Layout::Market:
      pBuyTab = &m_browseTabActive;
      break;
    default:
      throw logic_error("BrokerWindow::FindActiveTab() Invalid layout");
    }
    const TabSurface_t allTabs[] = {
        Tab::Id::Buy,       pBuyTab,
        Tab::Id::Sell,      &m_sellTabActive,
        Tab::Id::SalesLog,  &m_salesLogTabActive,
    };
    for (size_t index = 0; index < _countof(allTabs); ++index) {
      if (allTabs[index].tab != lastTab.tab) {
        if (GetTabWindow(allTabs[index].tab)
          .FindTab(surface, GetTabAreaRect(ptOrigin),
            *allTabs[index].pSurface, ptOrigin, ptFoundTab))
        {
          lastTab = allTabs[index];
          return lastTab.tab;
        }
      }
    }
    // TODO: throw
    LogError(L"BrokerWindow::FindActiveTab(): No tab active");
    return Tab::Id::None;
  }

  const Rect_t& Window_t::GetTabAreaRect(const POINT ptOrigin) const {
    // TODO: get rid of literals
    // TODO: lock?
    // TODO: based on layout, calc area using m_brokerCaption?
    constexpr SIZE tabArea = { 230, 60 };
    static Rect_t     tabAreaRect;

    int x = max(0, ptOrigin.x - (tabArea.cx - m_marketCaption.GetWidth()) / 2);
    int y = ptOrigin.y + m_marketCaption.GetHeight();
    SetRect(&tabAreaRect, x, y, x + tabArea.cx, y + tabArea.cy);
    return tabAreaRect;
  }

} // Broker