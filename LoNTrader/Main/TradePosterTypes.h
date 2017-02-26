///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TradePosterTypes.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif

#ifndef Include_TRADEPOSTERTYPES_H
#define Include_TRADEPOSTERTYPES_H

#include "CommonTypes.h"

namespace TradePoster
{

    typedef unsigned long Id_t;

    namespace Flag
    {
        static const Flag_t ShowBuyTrades  = 0x01;
        static const Flag_t ShowSellTrades = 0x02;
        static const Flag_t ShowAllTrades  = 0x03;
        static const Flag_t ShowDetail     = 0x10;
    }

    class PricingPolicy_i;
    class PostingPolicy_i;
    class Manager_t;
    class Data_t;
    class EventPost_t;
//    struct EventPost_t::Data_t;

} // TradePoster

///////////////////////////////////////////////////////////////////////////////

#endif // Include_TRADEPOSTERTYPES_H

///////////////////////////////////////////////////////////////////////////////
