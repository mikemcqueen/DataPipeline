
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//

#include "stdafx.h"
#include "LonTrader_t.h"
#include "LonCardSet_t.h"
#include "TradeManager_t.h"

namespace Services
{

LonCardSet_t&
GetCardSet()
{
    return LonTrader_t::GetCardSet();
}

TradeManager_t&
GetTradeManager()
{
    return LonTrader_t::GetTradeManager();
}

TradeExecutor_t&
GetTradeExecutor()
{
    return LonTrader_t::GetTradeExecutor();
}

};