///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// LonTraderImpl_t.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_LONTRADERIMPL_T_H
#define Include_LONTRADERIMPL_T_H

#include "SsTrades_t.h"
#include "PcapTrades_t.h"

#include "TrScrollThumb_t.h"
#include "TrPrompts_t.h"

#include "DcrTradeBuilder.h"
#include "TiTradeBuilder.h"
#include "TradeBuilder.h"

#include "DcrPostedTrades.h"
#include "TiPostedTrades.h"
#include "PostedTrades.h"

#include "DcrConfirmTrade.h"
#include "TiConfirmTrade.h"
#include "ConfirmTrade.h"

#include "DcrTradeDetail.h"
#include "TiTradeDetail.h"
#include "TradeDetail.h"

#include "TradeManager_t.h"
#include "TradePoster.h"
#include "TradeExecutor_t.h"

#include "LonCardSet_t.h"

#include "LonPlayer_t.h"

///////////////////////////////////////////////////////////////////////////////

struct LonTraderImpl_t
{
    LonTraderImpl_t(
        const wchar_t*     pszUsername);

    static
    LonPlayer_t             m_Player;

    static
    LonCardSet_t            s_CardSet;

    static
    TradeManager_t          m_TradeManager;

    static
    TradeExecutor_t         m_TradeExecutor;

    TradePoster::Manager_t  m_TradePoster;

    PostedTrades::Manager_t m_PostedTrades;
    TradeDetail::Manager_t  m_TradeDetail;

    TradeBuilder::Manager_t m_TradeBuilder;
    ConfirmTrade::Manager_t m_ConfirmTrade;

    SsTrades_t              m_SsTrades;
    PcapTrades_t            m_PcapTrades;

    TrScrollThumb_t         m_TrScroll;
    TrPrompts_t             m_TrPrompts;

private:

    LonTraderImpl_t();
    LonTraderImpl_t(const LonTraderImpl_t&);
    LonTraderImpl_t& operator=(const LonTraderImpl_t&);
};

///////////////////////////////////////////////////////////////////////////////

#endif // Include_LONTRADERIMPL_T_H

///////////////////////////////////////////////////////////////////////////////
