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
//#include "BrokerSellWindow.h"
//#include "BrokerSalesLogTab.h"
#include "BrokerUi.h"
#include "Log.h"
#include "Rect.h"
#include "MainWindow_t.h"
#include "Resource.h"

////////////////////////////////////////////////////////////////////////////////

//extern CDisplay* g_pDisplay;

namespace Broker
{

static const Flag_t WindowFlags;

////////////////////////////////////////////////////////////////////////////////
// Static functions

/* static */
bool
Window_t::
IsTabWindow(
    Ui::WindowId_t windowId)
{
    using namespace Window;
    switch (windowId)
    {
    case Id::BrokerBuyTab:
    case Id::BrokerSellTab:
    case Id::BrokerSalesLogTab:
        return true;
    default:
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////

/*static*/
Ui::WidgetId_t
Window_t::
GetTabWidgetId(
    Ui::WindowId_t windowId)
{
    using namespace Window;
    switch (windowId)
    {
    case Id::BrokerBuyTab:      return Frame::Widget::Id::BuyTab;
    case Id::BrokerSellTab:     return Frame::Widget::Id::SellTab;
    case Id::BrokerSalesLogTab: return Frame::Widget::Id::SalesLogTab;
    default:
        throw invalid_argument("BrokerWindow::GetTabWidgetId()");
    }
}

////////////////////////////////////////////////////////////////////////////////
// Constructor

Window_t::
Window_t(
    const Ui::Window_t& parent)
:
    Ui::Window_t(
        Broker::Window::Id::BrokerFrame,
        parent,
        L"BrokerFrame",
        WindowFlags),
    m_layout(Frame::Layout::Unknown)
{
    loadSurfaces();
}

////////////////////////////////////////////////////////////////////////////////

void
Window_t::
loadSurfaces()
{
    //TODO: array
    extern CDisplay* g_pDisplay;
    HRESULT hr = S_OK;
    hr = g_pDisplay->CreateSurfaceFromBitmap(&m_brokerCaption, MAKEINTRESOURCE(IDB_BROKER_CAPTION));
    if (FAILED(hr))
    {
        throw runtime_error("Create BrokerCaption surface");
    }
    hr = g_pDisplay->CreateSurfaceFromBitmap(&m_buyTabActive, MAKEINTRESOURCE(IDB_BROKER_BUYTAB_ACTIVE));
    if (FAILED(hr))
    {
        throw runtime_error("Create BrokerBuyTabActive surface");
    }
    hr = g_pDisplay->CreateSurfaceFromBitmap(&m_buyTabInactive, MAKEINTRESOURCE(IDB_BROKER_BUYTAB_INACTIVE));
    if (FAILED(hr))
    {
        throw runtime_error("Create BrokerBuyTabInactive surface");
    }
    hr = g_pDisplay->CreateSurfaceFromBitmap(&m_marketCaption, MAKEINTRESOURCE(IDB_MARKET_CAPTION));
    if (FAILED(hr))
    {
        throw runtime_error("Create MarketCaption surface");
    }
    hr = g_pDisplay->CreateSurfaceFromBitmap(&m_browseTabActive, MAKEINTRESOURCE(IDB_MARKET_BROWSETAB_ACTIVE));
    if (FAILED(hr))
    {
        throw runtime_error("Create BrowseTabActive surface");
    }
    hr = g_pDisplay->CreateSurfaceFromBitmap(&m_browseTabInactive, MAKEINTRESOURCE(IDB_MARKET_BROWSETAB_INACTIVE));
    if (FAILED(hr))
    {
        throw runtime_error("Create BrowseTabInactive surface");
    }
    hr = g_pDisplay->CreateSurfaceFromBitmap(&m_sellTabActive, MAKEINTRESOURCE(IDB_BROKER_SELLTAB_ACTIVE));
    if (FAILED(hr))
    {
        throw runtime_error("Create BrokerSellTabActive surface");
    }
    hr = g_pDisplay->CreateSurfaceFromBitmap(&m_sellTabInactive, MAKEINTRESOURCE(IDB_BROKER_SELLTAB_INACTIVE));
    if (FAILED(hr))
    {
        throw runtime_error("Create BrokerSellTabInactive surface");
    }
    hr = g_pDisplay->CreateSurfaceFromBitmap(&m_salesLogTabActive, MAKEINTRESOURCE(IDB_BROKER_SALESLOGTAB_ACTIVE));
    if (FAILED(hr))
    {
//        throw runtime_error("Create BrokerSalesLogTabActive surface");
    }
    hr = g_pDisplay->CreateSurfaceFromBitmap(&m_salesLogTabInactive, MAKEINTRESOURCE(IDB_BROKER_SALESLOGTAB_INACTIVE));
    if (FAILED(hr))
    {
//        throw runtime_error("Create BrokerSalesLogTabInactive surface");
    }
}

////////////////////////////////////////////////////////////////////////////////

Ui::Window_t&
Window_t::
GetWindow(
    Ui::WindowId_t windowId) const
{
    static Buy::Window_t      buyTab(GetParent());
//    static Sell::Window_t     sellTab(GetParent());
//    static SalesLog::Window_t salesLogTab(GetParent());

    using namespace Broker::Window;
    switch (windowId)
    {
    case Id::BrokerBuyTab:      return buyTab;
//    case Id::BrokerSellTab:     return sellTab;
//    case Id::BrokerSalesLogTab: return salesLogTab;
    default:
        throw std::invalid_argument("BrokerWindow::GetWindow()");
    }
}

////////////////////////////////////////////////////////////////////////////////

TabWindow_t&
Window_t::
GetTabWindow(
    Tab_t Tab) const
{
    using namespace Broker::Window;
    switch (Tab)
    {
    case Tab::Id::Buy:      return static_cast<TabWindow_t&>(GetWindow(Id::BrokerBuyTab));
    case Tab::Id::Sell:     return static_cast<TabWindow_t&>(GetWindow(Id::BrokerSellTab));
    case Tab::Id::SalesLog: return static_cast<TabWindow_t&>(GetWindow(Id::BrokerSalesLogTab));
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
Ui::WindowId_t
Window_t::
GetWindowId(
    const CSurface& surface,
    const POINT*    /*pptHint*/) const
{
    ///using namespace Ui::Window;
    Ui::WindowId_t windowId = Ui::Window::Id::Unknown;
    POINT ptCaption;
    if (IsLocatedOn(surface, Ui::Window::Locate::CompareLastOrigin | Ui::Window::Locate::Search, &ptCaption))
    {
        POINT ptTab;
        Tab_t Tab = FindActiveTab(surface, ptCaption, ptTab);
        if (Tab::Id::None != Tab)
        {
            Ui::Window_t& tabWindow = GetTabWindow(Tab);
            LogInfo(L"BrokerWindow::GetWindowId() Found %ls Tab @ (%d, %d)",
                    tabWindow.GetWindowName(), ptTab.x, ptTab.y);
            // Now that we have verified the tab is active, validate
            // the table, or determine if a popup is active
            windowId = tabWindow.GetWindowId(surface, &ptTab);
        }
    }
    return windowId;
}

////////////////////////////////////////////////////////////////////////////////

bool
Window_t::
IsLocatedOn(
    const CSurface& surface,
          Flag_t    flags,
          POINT*    pptOrigin) const
{
    static const CSurface* pLastCaption = nullptr;
    static const CSurface Window_t::* captions[] =
    {
        &Window_t::m_marketCaption,
        &Window_t::m_brokerCaption,
    };

    using namespace Ui::Window;
    if (flags.Test(Locate::CompareLastOrigin) && (nullptr != pLastCaption) &&
        CompareLastOrigin(surface, *pLastCaption, pptOrigin))
    {
        return true;
    }
    if (flags.Test(Locate::Search))
    {
        size_t index = 0;
        const CSurface* pCaption = pLastCaption;
        if (nullptr == pCaption) {
            pCaption = &(this->*captions[index++]);
        }
        for (;;) {
            // Look for caption bitmap on the supplied surface
            if (((0 == index) || (pCaption != pLastCaption)) &&
                    OriginSearch(surface, *pCaption, pptOrigin)) {
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

////////////////////////////////////////////////////////////////////////////////

void
Window_t::
GetOriginSearchRect(
    const CSurface& surface,
          Rect_t&   rect) const
{
    // search the upper left 1/9 of the screen
    rect = surface.GetBltRect();
    rect.right /= 3;
    rect.bottom /= 3;
}

////////////////////////////////////////////////////////////////////////////////

MainWindow_t&
Window_t::
GetMainWindow() const
{
    return const_cast<MainWindow_t&>(dynamic_cast<const MainWindow_t&>(GetParent()));
}

////////////////////////////////////////////////////////////////////////////////

#if 1
void
Window_t::
SetLayout(
    Frame::Layout_t layout) const
{
    if (layout != m_layout)
    {
        m_layout = layout;
        const MainWindow_t& mainWindow = GetMainWindow();//static_cast<const MainWindow_t&>(GetParent());
        mainWindow.GetBrokerBuyWindow().SetLayout(layout);
//       mainWindow.GetBrokerSellWindow().SetLayout(layout);
//        mainWindow.GetBrokerSalesLogWindow().SetLayout(layout);
    }
}
#endif

////////////////////////////////////////////////////////////////////////////////

Tab_t
Window_t::
FindActiveTab(
    const CSurface& surface,
    const POINT&    ptOrigin,
          POINT&    ptFoundTab) const
{
    struct TabSurface_t
    {
        Tab_t           tab;
        const CSurface* pSurface;
    };
    static TabSurface_t lastTab = { Tab::Id::None, nullptr };

    // First, try the last known tab window
    if (Tab::Id::None != lastTab.tab)
    {
        if (nullptr != lastTab.pSurface)
        {
            if (GetTabWindow(lastTab.tab).FindTab(surface, GetTabAreaRect(ptOrigin),
                    *lastTab.pSurface, ptOrigin, ptFoundTab))
            {
                return lastTab.tab;
            }
        }
        else
        {
            throw logic_error("BrokerWindow::FindActiveTab() lastTab.pSurface is nullptr");
        }
    }
    // Next, try all the other tabs
    const CSurface* pBuyTab = nullptr;
    switch (m_layout)
    {
    case Frame::Layout::Broker:
        pBuyTab = &m_buyTabActive;
        break;
    case Frame::Layout::Market:
        pBuyTab = &m_browseTabActive;
        break;
    default:
        throw logic_error("BrokerWindow::FindActiveTab() Invalid layout");
    }
    const TabSurface_t allTabs[] =
    {
        Tab::Id::Buy,       pBuyTab,
        Tab::Id::Sell,      &m_sellTabActive,
        Tab::Id::SalesLog,  &m_salesLogTabActive,
    };
    for (size_t index = 0; index < _countof(allTabs); ++index)
    {
        if (allTabs[index].tab != lastTab.tab)
        {
            if (GetTabWindow(allTabs[index].tab).
                FindTab(surface, GetTabAreaRect(ptOrigin),
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

////////////////////////////////////////////////////////////////////////////////

const Rect_t&
Window_t::
GetTabAreaRect(
    const POINT& ptOrigin) const
{
    // TODO: get rid of literals
    // TODO: lock?
    // TODO: based on layout, calc area using m_brokerCaption?
    static const SIZE tabArea = { 230, 60 };
    static Rect_t     tabAreaRect;

    int x = max(0, ptOrigin.x - (tabArea.cx - m_marketCaption.GetWidth()) / 2);
    int y = ptOrigin.y + m_marketCaption.GetHeight();
    SetRect(&tabAreaRect, x, y, x + tabArea.cx, y + tabArea.cy);
    return tabAreaRect;
}

////////////////////////////////////////////////////////////////////////////////

} // Broker

////////////////////////////////////////////////////////////////////////////////
