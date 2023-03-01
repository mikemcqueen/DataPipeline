////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// MainWindow_t.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef INCLUDE_MAINWINDOW_T_H
#define INCLUDE_MAINWINDOW_T_H

#include "UiWindow.h"
#include "BrokerUi.h"
#include "BrokerBuyTypes.h"
#include "BrokerSellTypes.h"
//#include "SetPriceTypes.h"
//#include "Eq2LoginTypes.h"
#include "DpMessage.h"

class MainWindow_t : public Ui::Window::WithHandle_t {
private:
  // TODO haxoid
  static Rect_t m_PopupRect;

public:
  MainWindow_t();

  Broker::Window_t& GetBrokerWindow() const;
  Broker::Buy::Window_t& GetBrokerBuyWindow() const;
  Broker::Sell::Window_t& GetBrokerSellWindow() const;
#if 0
  Broker::SetPrice::Window_t& GetSetPricePopup() const;
  Broker::Eq2Login::Window_t& GetEq2LoginWindow() const;
#endif

  // Ui::Window_t virtual:
  Ui::Window::Base_t& GetWindow(Ui::WindowId_t WindowId) const override;

  Ui::WindowId_t GetWindowId(
    const CSurface& Surface,
    const POINT* pptHint) const override;

  Ui::WindowId_t GetMessageWindowId(const DP::MessageId_t& messageId) const;

  /*
  static const Rect_t& GetPopupRect() { return m_PopupRect; }

  static void SetPopupRect(const Rect_t& Rect) { m_PopupRect = Rect; }
*/

private:
  MainWindow_t(const MainWindow_t&);
  MainWindow_t& operator=(const MainWindow_t&);
};

#endif // INCLUDE_MAINWINDOW_T_H