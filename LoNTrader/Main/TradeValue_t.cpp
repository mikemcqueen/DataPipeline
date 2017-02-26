///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TradeValue_t.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TradeValue_t.h"
#include "TradeManager_t.h"
#include "Trade_t.h"
#include "Services.h"
#include "Log.h"

///////////////////////////////////////////////////////////////////////////////
//
// TradeValueSet_t
//
///////////////////////////////////////////////////////////////////////////////

size_t
TradeValueSet_t::
MeanValue(
    bool bDetail) const
{
    if (empty())
        return 0;
    TradeValue_t::SumValues
        sv = std::for_each(begin(), end(), TradeValue_t::SumValues());
    if (0 == sv.Count)
        return 0;
    size_t Mean = sv.Sum / sv.Count;
    if (bDetail)
    {
        LogAlways(L"Sum = %d, Count = %d, Total = %d, Mean = %d",
            sv.Sum, sv.Count, sv.Total, Mean);
    }
    return Mean;
}

///////////////////////////////////////////////////////////////////////////////

void
TradeValueSet_t::
Show(
    const wchar_t* pszName,
    const size_t   Order) const
{
    const_iterator it = begin();
    LogAlways(L"%s (%d):", (NULL == pszName) ? L"TradeValueSet_t" : pszName, size());
    for (; end() != it; ++it)
    {
        const TradeValue_t& tv = *it;
        if ((0 == Order) || (tv.Order == Order))
        {
            Trade_t Trade;
            if (Services::GetTradeManager().GetTrade(tv.TradeId, Trade))
            {
                LogAlways(L"  Trade (%8d) [%-20ls] Quantity (%d), Value (%d), Order (%d)",
                          tv.TradeId, Trade.GetUser(), tv.Quantity, tv.Value, tv.Order);
            }
            else
            {
                LogError(L"TradeValueSet_t::Show() Trade not found (%d)", tv.TradeId);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
