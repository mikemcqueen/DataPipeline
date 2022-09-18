////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// Eq2BrokerImpl_t.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Eq2BrokerImpl_t.h"
#include "DdUtil.h"

extern CDisplay* g_pDisplay;

////////////////////////////////////////////////////////////////////////////////

Eq2BrokerImpl_t::
Eq2BrokerImpl_t(
    Eq2Broker_t&          ,//broker,
    Broker::MainWindow_t& mainWindow)
:
    m_SsWindow(*g_pDisplay, mainWindow),
    m_TrWindowType(mainWindow),
    m_TrScroll(mainWindow),
    m_BuyWindow(mainWindow.GetBrokerBuyWindow())
#if 0
    m_SellWindow(mainWindow.GetBrokerSellWindow()),
    m_SetPricePopup(mainWindow.GetSetPricePopup()),
    m_eq2LoginWindow(mainWindow.GetEq2LoginWindow()),
    m_txSetActiveWindow(mainWindow),
    m_txBuyTabGetItems(broker),
    m_txSellTabGetItems(broker),
    m_txRepriceItems(mainWindow),
    m_txSetWidgetText(broker),
    m_txGetItemPrices(broker),
    m_txBuyItem(broker),
    m_txBuySellItems(broker),
    m_txLogon(broker),
    m_txOpenBroker(broker)
#endif
{
}

///////////////////////////////////////////////////////////////////////////////

Eq2BrokerImpl_t::
~Eq2BrokerImpl_t()
{
}

///////////////////////////////////////////////////////////////////////////////
