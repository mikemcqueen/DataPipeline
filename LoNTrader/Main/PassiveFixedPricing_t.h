///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// PassiveFixedPricing_t.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif

#ifndef Include_PASSIVEFIXEDPRICING_T_H
#define Include_PASSIVEFIXEDPRICING_T_H

#include "TradePosterPolicy.h"

class LonCard_t;

///////////////////////////////////////////////////////////////////////////////

namespace TradePoster
{

class PassiveFixedPricing_t :
    public PricingPolicy_i
{
    friend class Data_t;

public:

    static const wchar_t PolicyName[];

private:

    size_t m_Price;

public:

    PassiveFixedPricing_t(
        size_t Price)
    :
        m_Price(Price)
    { }

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
    size_t
    GetLowPrice() const
    {
        return m_Price;
    }

    virtual
    size_t
    GetHighPrice() const
    {
        return m_Price;
    }

    virtual
    size_t
    GetPriceIncrement() const
    {
        return 0;
    }

#ifdef GETPRICE
    virtual
    size_t
    GetPrice() const
    {
        return m_Price;
    }
#endif

    virtual
    void
    Show(
        bool bDetail) const;

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

private:
 
    // Manager_t uses this:
    PassiveFixedPricing_t();

};

} // TradePoster

///////////////////////////////////////////////////////////////////////////////

#endif // Include_PASSIVEFIXEDPRICING_T_H

///////////////////////////////////////////////////////////////////////////////
