///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// SimplePosting_t.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif

#ifndef Include_SIMPLEPOSTING_T_H
#define Include_SIMPLEPOSTING_T_H

#include "TradePosterPolicy.h"
#include "TradeMaker_t.h"
#include "AutoCs.h"

namespace TradePoster
{

///////////////////////////////////////////////////////////////////////////////

class SimplePosting_t :
    public PostingPolicy_i
{

public:

    static const size_t  MaxTradeCount = 500;

    static const wchar_t XmlElementAllowedCards[];
    static const wchar_t XmlElementTrades[];
    static const wchar_t PolicyName[];

private:

    TradeMaker_t           m_TradeMaker;
    CardQuantity_t         m_CardQ;
    CardQuantityQueue_t    m_AllowedCards;
    mutable
    CAutoCritSec           m_csTrades;      // m_csData; locks m_TradeMaker and m_AllowedCards
//    size_t                 m_PostedValue;

public:

    explicit
    SimplePosting_t(
        const CardQuantity_t&  CardQ);

    explicit
    SimplePosting_t(
        const CardQuantity_t&      CardQ,
        const CardQuantityQueue_t& AllowedCards);

    //
    // PostingPolicy_i virtual:
    //

    virtual const wchar_t* GetName() const { return PolicyName; }

    virtual
    size_t
    GenerateTrades(
        Data_t::Type_e Type,
        size_t         Value,
        size_t         MaxPerAllowedCard = 1,
        size_t         MaxTotal = 0);

    virtual
    size_t
    PostAllTrades(
        Id_t   TradePosterId,
        size_t Value,
        Flag_t Flags);

    virtual
    size_t
    PostTrade(
        Id_t      TradePosterId,
        size_t    Value,
        TradeId_t TradeId,
        Flag_t    Flags);

    virtual
    size_t
    RemoveAllTrades(
        Id_t   TradePosterId,
        size_t Value,
        Flag_t Flags);

    virtual
    size_t
    RemoveTrade(
        Id_t      TradePosterId,
        size_t    Value,
        TradeId_t TradeId,
        Flag_t    Flags);

    virtual
    void
    EraseAllTrades();

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
        const TradePoster::Data_t& Data,
              TradeId_t            TradeId);

    virtual
    void
    Show(
        size_t Value,
        bool   bDetail) const;

    virtual
    bool
    ReadXml(
              IXmlReader* pReader,
        const wchar_t*    ElementName = PolicyName);

    virtual
    bool
    WriteXml(
              IXmlWriter* pWriter,
        const wchar_t*    ElementName = PolicyName) const;

public:

    //
    // Helpers:
    //

    void
    AddAllowedCard(
        const CardQuantity_t& CardQ);

    bool
    RemoveAllowedCard(
        const CardQuantity_t& CardQ);


private:

    bool
    ReadXmlFile(
        const wchar_t* pszFilename);

    bool
    WriteXmlFile(
        const wchar_t* pszFilename) const;
//              size_t   Value) const;

private:

    bool
    PostOrReplaceTrade(
        Trade_t& Trade,
        Id_t     TradePosterId,
        size_t   Value,
        bool     bTest);

#if 0
    TradeId_t
    GetNewTradeId(
        const TradeMap_t& Trades) const;
#endif

    const TradeList_t&  GetTrades(size_t Value) const { return m_TradeMaker.GetTrades(Value); }
    TradeList_t&        GetTrades(size_t Value)       { return m_TradeMaker.GetTrades(Value); }

private:

    SimplePosting_t();
    SimplePosting_t(const SimplePosting_t&);
    SimplePosting_t& operator=(const SimplePosting_t&);
};

} // TradePoster

///////////////////////////////////////////////////////////////////////////////

#endif // SIMPLEPOSTING_T_H

///////////////////////////////////////////////////////////////////////////////
