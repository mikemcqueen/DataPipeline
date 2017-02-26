
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// App services.  Another way of saying globals.
//

#pragma once

class LonCardSet_t;
class TradeManager_t;
class TradeExecutor_t;

namespace Services
{

    LonCardSet_t&     GetCardSet();

    TradeManager_t&   GetTradeManager();

    TradeExecutor_t&  GetTradeExecutor();

};
