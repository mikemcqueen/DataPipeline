////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// Broker(Frame)Window.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef INCLUDE_BROKERWINDOW_H
#define INCLUDE_BROKERWINDOW_H

#include "UiWindow.h"
#include "BrokerUi.h"
#include "BrokerBuyWindow.h"
#include "BrokerSellWindow.h"
#include "DdUtil.h"

namespace Broker { //TODO: ::Frame {
  class Window_t : public Ui::Window_t {
  public:
    // this tab shit is goofy. nix it.
    static bool IsTabWindow(Ui::WindowId_t windowId);
    static Ui::WidgetId_t GetTabWidgetId(Ui::WindowId_t windowId);

    explicit Window_t(const Ui::Window_t& parent);

    // Ui::Window_t virtual:
    //Ui::Window::Base_t& GetWindow(Ui::WindowId_t windowId) const override;

    Ui::WindowId_t GetWindowId(
      const CSurface& Surface,
      const POINT* pptHint = nullptr) const override;

    bool IsLocatedOn(
      const CSurface& Surface,
      Flag_t    flags,
      POINT* pptOrigin = nullptr) const override;

    void GetOriginSearchRect(
      const CSurface& surface,
      Rect_t& rect) const override;

    // 

    Tab_t FindActiveTab(
      const CSurface& Surface,
      const POINT    ptOrigin,
      POINT& ptTab) const;

    const Rect_t& GetTabAreaRect(const POINT ptOrigin) const;
    const TabWindow_t& GetTabWindow(Tab_t Tab) const;
    Frame::Layout_t GetLayout() const { return m_layout; }

    auto& GetBrokerBuyWindow() const { return buy_window_; }
    auto& GetBrokerSellWindow() const { return sell_window_; }
/*
    auto& GetBrokerBuyWindow() const { return broker_buy_; }
    using namespace Broker::Window;
      return dynamic_cast<const Buy::Window_t&>(GetWindow(Id::BrokerBuy));
    }
    const Sell::Window_t& GetBrokerSellWindow() const {
      using namespace Broker::Window;
      return dynamic_cast<const Sell::Window_t&>(GetWindow(Id::BrokerBuy));
    }
*/

  private:
    void LoadSurfaces();
    void SetLayout(Frame::Layout_t layout) const;

  private:
    Buy::Window_t buy_window_;
    Sell::Window_t sell_window_;

    // Broker window
    CSurface  m_brokerCaption;
    CSurface  m_buyTabActive;
    CSurface  m_buyTabInactive;

    // Market window
    CSurface  m_marketCaption;
    CSurface  m_browseTabActive;
    CSurface  m_browseTabInactive;

    // Broker + Market windows
    CSurface  m_sellTabActive;
    CSurface  m_sellTabInactive;
    CSurface  m_salesLogTabActive;
    CSurface  m_salesLogTabInactive;

    mutable Frame::Layout_t m_layout;
  };
} // Broker

#endif // INCLUDE_BROKERWINDOW_H