///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// SimpleRangePricing_t.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif

#ifndef Include_SIMPLERANGEPRICING_T_H
#define Include_SIMPLERANGEPRICING_T_H

#include "TradePosterPolicy.h"

namespace TradePoster
{

///////////////////////////////////////////////////////////////////////////////

class SimpleRangePricing_t :
    public PricingPolicy_i
{
    friend class Data_t;

public:

    static const wchar_t PolicyName[];
    static const wchar_t XmlElementHigh[];
    static const wchar_t XmlElementLow[];
    static const wchar_t XmlElementIncrement[];
    static const wchar_t XmlElementCurrent[];

private:

    size_t m_LowPrice;
    size_t m_HighPrice;
    size_t m_PriceIncrement;
    size_t m_CurrentPrice;

public:

    SimpleRangePricing_t(
        size_t LowPrice,
        size_t HighPrice,
        size_t PriceIncrement);

    //
    // PricingPolicy_i virtual:
    //

    virtual
    void
    OnTradePosted(
        const Trade_t&  Trade,
              TradeId_t NewTradeId,
              size_t    Value);

    virtual
    void
    OnTradeAdded(
        const Trade_t& Trade);

    virtual
    void
    OnTradeRemoved(
        const Data_t&   Data,
              TradeId_t TradeId);

    virtual 
    const wchar_t*
    GetName() const
    {
        return PolicyName;
    }

    virtual
    void
    Show(
        bool bDetail = true) const;

    virtual 
    bool
    ReadXml(
        IXmlReader*    pReader,
        const wchar_t* ElementName = PolicyName);
    
    virtual
    bool
    WriteXml(
        IXmlWriter*    pWriter,
        const wchar_t* ElementName = PolicyName) const;

    virtual
    size_t
    GetLowPrice() const
    {
        return m_LowPrice;
    }

    virtual
    size_t
    GetHighPrice() const
    {
        return m_HighPrice;
    }

    virtual
    size_t
    GetPriceIncrement() const
    {
        return m_PriceIncrement;
    }

#ifdef GETPRICE
    virtual
    size_t
    GetPrice() const
    {
        return m_CurrentPrice;
    }
#endif

    void
    SetPrice(
        size_t Low,
        size_t High,
        size_t Step,
        size_t Current);

private:
 
    // Manager_t uses this:
    SimpleRangePricing_t();

};

///////////////////////////////////////////////////////////////////////////////

}

#endif // Include_SIMPLERANGEPRICING_T_H

///////////////////////////////////////////////////////////////////////////////
