/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TradeValue_t.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TRADEVALUE_T_H
#define Include_TRADEVALUE_T_H

#include "TradeTypes.h"

/////////////////////////////////////////////////////////////////////////////
//
// TradeValue_t
//
/////////////////////////////////////////////////////////////////////////////

struct TradeValue_t
{
    TradeId_t TradeId;
    size_t    Value;
    size_t    Quantity;
    size_t    Order;  // e.g. Order=1 means value based directly on boosters,
                      //      Order=2 means value based on Order1 card.

    TradeValue_t() :
        TradeId(0),
        Value(0),
        Quantity(0),
        Order(0)
    { }

    TradeValue_t(
        TradeId_t Id,
        size_t    Val  = 0,
        size_t    Quan = 1,
        size_t    Ord  = 0)
    :
        TradeId(Id),
        Value(Val),
        Quantity(Quan),
        Order(Ord)
    { }

    struct SortById
    {
        bool operator()(const TradeValue_t& lhs, const TradeValue_t& rhs) const
        {
            return lhs.TradeId < rhs.TradeId;
        }
    };

    struct SortByValue
    {
        bool operator()(const TradeValue_t& lhs, const TradeValue_t& rhs) const
        {
            return lhs.Value < rhs.Value;
        }
    };

    struct CountQuantity
    {
        size_t Total;

        CountQuantity() : 
            Total(0)
        { } 

        void operator()(const TradeValue_t& tv)
        {
            Total += tv.Quantity;
        }
    };

    struct SumValues
    {
        size_t Sum;
        size_t Total;  // total count of all values
        size_t Count;  // count of non-zero values

        SumValues() : 
            Sum(0),
            Total(0),
            Count(0)
        { } 

        void operator()(const TradeValue_t& tv)
        {
            Total += tv.Quantity;
            if (0 != tv.Value)
            {
                Sum += tv.Value;
                Count += tv.Quantity;
            }
        }
    };
};

/////////////////////////////////////////////////////////////////////////////
//
// TradeValueSet_t
//
/////////////////////////////////////////////////////////////////////////////

class TradeValueSet_t :
    public std::set<
        TradeValue_t,
        TradeValue_t::SortById>
{
public:

    typedef std::pair<TradeValueSet_t::iterator, bool> Pair_t;

    void
    Show(
        const wchar_t* pszName = NULL,
        const size_t   Order   = 0) const;

    const_iterator
    Highest() const
    {
        return std::max_element(begin(), end(), TradeValue_t::SortByValue());
    }

    const_iterator
    Lowest() const
    {
        return std::min_element(begin(), end(), TradeValue_t::SortByValue());
    }

    size_t
    HighestValue() const
    {
        const_iterator it = Highest();
        return (end() == it) ? 0 : it->Value;
    }

    size_t
    LowestValue() const
    {
        const_iterator it = Lowest();
        return (end() == it) ? 0 : it->Value;
    }

    size_t
    TotalQuantity() const
    {
        return std::for_each(begin(), end(), TradeValue_t::CountQuantity()).Total;
    }

    size_t
    MeanValue(
        bool bDetail = false) const;
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TRADEVALUE_T_H

/////////////////////////////////////////////////////////////////////////////
