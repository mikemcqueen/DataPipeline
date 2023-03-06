////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// Eq2BrokerImpl_t.h
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_EQ2BROKERIMPL_T_H
#define Include_EQ2BROKERIMPL_T_H

#include "SsWindow.h"
#include "TrWindowType.h"
#include "TrScrollThumb_t.h"
#include "BrokerBuyWindowManager.h"
#include "BrokerSellWindowManager.h"
#include "SetPrice.h"
#include "TxSellItems.h"

#if 0
#include "Eq2Login.h"
#include "TxLogon.h"
#include "TxSetActiveWindow.h"
#include "TxBuyTabGetItems.h"
#include "TxSellTabGetItems.h"
#include "TxGetItemPrices.h"
#include "TxGetItemsForSale.h"
#include "TxRepriceItems.h"
#include "TxSetWidgetText.h"
#include "TxBuyItem.h"
#include "TxBuySellItems.h"
#include "TxOpenBroker.h"
#endif

class MainWindow_t;

using namespace Broker;
//using namespace Broker::Transaction;

class Eq2BrokerImpl_t {
public:
  Eq2BrokerImpl_t(MainWindow_t& mainWindow);
  ~Eq2BrokerImpl_t();

  Eq2BrokerImpl_t() = delete;
  Eq2BrokerImpl_t(const Eq2BrokerImpl_t&) = delete;
  Eq2BrokerImpl_t& operator=(const Eq2BrokerImpl_t&) = delete;

  // Acquire handlers
  SsWindow::Acquire::Handler_t m_SsWindow;

  // Translate handlers
  Translate::WindowType_t      m_TrWindowType;
  TrScrollThumb_t              m_TrScroll;

  // Translate/Interpret handlers
  Buy::Window::Manager_t       buyWindowManager_;
  Sell::Window::Manager_t      sellWindowManager_;
  SetPrice::Window::Manager_t  setprice_manager_;

  Broker::Transaction::SellItems::Handler_t   tx_sellitems_;

#if 0
  Eq2Login::Window::Manager_t  m_eq2LoginWindow;

  // Transaction handlers
  Logon::Handler_t             m_txLogon;
  SetActiveWindow::Handler_t   m_txSetActiveWindow;
  BuyTabGetItems::Handler_t    m_txBuyTabGetItems;
  SellTabGetItems::Handler_t   m_txSellTabGetItems;
  SetWidgetText::Handler_t     m_txSetWidgetText;
  GetItemPrices::Handler_t     m_txGetItemPrices;
  GetItemsForSale::Handler_t   m_txGetItemsForSale;
  RepriceItems::Handler_t      m_txRepriceItems;
  BuyItem::Handler_t           m_txBuyItem;
  BuySellItems::Handler_t      m_txBuySellItems;
  OpenBroker::Handler_t        m_txOpenBroker;
#endif
};

#endif // Include_EQ2BROKERIMPL_T_H
