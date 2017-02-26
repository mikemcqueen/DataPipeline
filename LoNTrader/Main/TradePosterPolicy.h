///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TradePosterPolicy.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif

#ifndef Include_TRADEPOSTERPOLICY_H
#define Include_TRADEPOSTERPOLICY_H

#include "TradePoster.h"

class LonCard_t;
class Trade_t;

namespace TradePoster
{

    typedef unsigned long Id_t;

    namespace Flag
    {
        static const Flag_t TestPost       = 0x0001;
    }

    static const wchar_t XmlAttributeName[]  = L"Name";
    static const wchar_t XmlElementPrice[]   = L"Price";
    static const wchar_t XmlAttributeType[]  = L"Type";

//////////////////////////////////////////////////////////////////////////////
//
// Policy_i
//
//////////////////////////////////////////////////////////////////////////////

class Policy_i
{

public:

    virtual
    ~Policy_i() = 0;

    virtual
    void
    OnTradePosted(
        const Trade_t&  Trade,
              TradeId_t NewTradeId,
              size_t    Value) = 0;

    virtual
    void
    OnTradeAdded(
        const Trade_t& Trade) = 0;

    virtual
    void
    OnTradeRemoved(
        const Data_t&   Data,
              TradeId_t TradeId) = 0;

    virtual 
    const wchar_t*
    GetName() const = 0;

    virtual 
    bool
    ReadXml(
              IXmlReader* pReader,
        const wchar_t*    ElementName) = 0;
    
    virtual
    bool
    WriteXml(
              IXmlWriter* pWriter,
        const wchar_t*    ElementName) const = 0;
};

inline Policy_i::~Policy_i() {}

//////////////////////////////////////////////////////////////////////////////

class PricingPolicy_i :
    public Policy_i
{

public:

    static const wchar_t XmlElementName[];

public:

    virtual
    ~PricingPolicy_i() = 0;

    virtual
    size_t
    GetLowPrice() const = 0;

    virtual
    size_t
    GetHighPrice() const = 0;

    virtual
    size_t
    GetPriceIncrement() const = 0;

#ifdef GETPRICE
    virtual
    size_t
    GetPrice() const = 0;
#endif

    virtual
    void
    Show( 
        bool /*bDetail*/) const = 0;
};

inline PricingPolicy_i::~PricingPolicy_i() {}

///////////////////////////////////////////////////////////////////////////////

class PostingPolicy_i :
    public Policy_i
{

public:

    static const wchar_t XmlElementName[];

public:

    virtual
    ~PostingPolicy_i() = 0;

    virtual
    size_t
    GenerateTrades(
        Data_t::Type_e Type,
        size_t         Value,
        size_t         MaxPerAllowedCard = 1,
        size_t         MaxTotal = 0) = 0;

    virtual
    size_t
    PostAllTrades(
        Id_t   TradePosterId,
        size_t Value,
        Flag_t Flags) = 0;

    virtual
    size_t
    PostTrade(
        Id_t      TradePosterId,
        size_t    Value,
        TradeId_t TradeId,
        Flag_t    Flags) = 0;

    virtual
    size_t
    RemoveAllTrades(
        Id_t   TradePosterId,
        size_t Value,
        Flag_t Flags) = 0;

    virtual
    size_t
    RemoveTrade(
        Id_t      TradePosterId,
        size_t    Value,
        TradeId_t TradeId,
        Flag_t    Flags) = 0;

    virtual
    void
    EraseAllTrades() = 0;

    virtual
    void
    Show( 
        size_t Value,
        bool /*bDetail*/) const = 0;
};

inline PostingPolicy_i::~PostingPolicy_i() {}

} // namespace TradePoster

///////////////////////////////////////////////////////////////////////////////

#endif Include_TRADEPOSTERPOLICY_H

///////////////////////////////////////////////////////////////////////////////
