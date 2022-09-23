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
#include "MainWindow_t.h"

#include "TrWindowType.h"
#include "TrScrollThumb_t.h"

#include "BrokerBuyWindowManager.h"
#if 0
#include "BrokerSell.h"
#include "SetPrice.h"
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

#include "Character_t.h"

using namespace Broker;
using namespace Broker::Transaction;

///////////////////////////////////////////////////////////////////////////////

class Eq2Broker_t;

class Eq2BrokerImpl_t
{
public:

    // Acquire handlers
    SsWindow::Acquire::Handler_t m_SsWindow;

    // Translate handlers
    TrWindowType_t               m_TrWindowType;
    TrScrollThumb_t              m_TrScroll;

    // Translate/Interpret handlers
    Buy::Window::Manager_t       buyWindowManager_;
#if 0
    Sell::Window::Manager_t      m_SellWindow;
    SetPrice::Window::Manager_t  m_SetPricePopup;
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

public:

    Eq2BrokerImpl_t(
        Eq2Broker_t&          broker,
        Broker::MainWindow_t& mainWindow);

    ~Eq2BrokerImpl_t();

    Eq2BrokerImpl_t() = delete;
    Eq2BrokerImpl_t(const Eq2BrokerImpl_t&) = delete;
    Eq2BrokerImpl_t& operator=(const Eq2BrokerImpl_t&) = delete;
};

///////////////////////////////////////////////////////////////////////////////

#endif // Include_EQ2BROKERIMPL_T_H

///////////////////////////////////////////////////////////////////////////////
