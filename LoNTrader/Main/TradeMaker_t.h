///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TradeMaker_t.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif

#ifndef Include_TRADEMAKER_T_H
#define Include_TRADEMAKER_T_H

#include "TradeTypes.h"
#include "TradeMap_t.h"

///////////////////////////////////////////////////////////////////////////////
//
// TradeMaker_t
//
///////////////////////////////////////////////////////////////////////////////

class LonCard_t;
struct CardQuantity_t;
class CardCollection_t;
class CardQuantityQueue_t;

class TradeMaker_t
{

public:

    typedef std::map<size_t, TradeList_t> Map_t;        // Map of TradeLists indexed by value

private:

    static const wchar_t XmlElementName[];
    static const wchar_t XmlElementTradeMap[];
    static const wchar_t XmlElementTrades[];

//    typedef std::pair<size_t, TradeMap_t> MapElem_t;   // i.e. iterator

    Map_t     m_Map;

public:

    TradeMaker_t();

    size_t
    BuildTrades(
              bool                 bBuy,
        const CardQuantity_t&      CardQ,              // Card, and max # of cards to use per trade
              size_t               Value,              // Card value
        const CardQuantityQueue_t& AllowedCards,
              size_t               MaxPerAllowedCard,  // Max # of trades per allowed card
              size_t               BuildMax);          // Total # of trades to generate

#if 0
    size_t
    RemoveTradesWithCard(
        const LonCard_t* pCard);
#endif

    void
    ShowTrades(
        size_t Value) const;

    void
    ShowAllTrades() const;

    bool
    HasTrades(
        size_t Value) const;

    TradeList_t&
    AddTradesDestructive(
        size_t Value);

    TradeList_t&
    GetTrades(
        size_t Value);

    const TradeList_t& 
    GetTrades(
        size_t Value) const;

    Trade_t*
    FindTrade(TradeId_t TradeId);

    // Return a pointer to the TradeMap_t that contains TradeId, or NULL if
    // TradeId is not found in any TradeMap_t.
    // HACK: Sorta.

/*
    TradeList_t*
    GetTradeList(
        TradeId_t TradeId);
*/

    void
    EraseAll();

    bool
    ReadXml(
        IXmlReader*    pReader,
        const wchar_t* ElementName = XmlElementName);

    bool
    WriteXml(
        IXmlWriter*    pWriter,
        const wchar_t* ElementName = XmlElementName) const;

private:

    size_t
    RecursiveBuildCards(
        CardCollection_t&    BuildCards,
        size_t               TotalValue,
        CardQuantityQueue_t& FromCards,
        size_t               Level = 0) const;

    TradeList_t&
    GetTradeMapOrEmptyMap(
        size_t Value) const;

    void
    ShowTrades(
        Map_t::const_iterator& itMap) const;

private:

    TradeMaker_t(const TradeMaker_t&);
    TradeMaker_t& operator=(const TradeMaker_t&);
};

///////////////////////////////////////////////////////////////////////////////

#endif //  Include_TRADEMAKER_T_H

///////////////////////////////////////////////////////////////////////////////
