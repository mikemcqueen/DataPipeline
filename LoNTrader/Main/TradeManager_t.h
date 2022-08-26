/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TradeManager_t.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TRADEMANAGER_T_H
#define Include_TRADEMANAGER_T_H

#include "DpHandler_t.h"
#include "TransactionManager.h"
#include "Trade_t.h"
#include "TradeMap_t.h"
#include "CardCollection_t.h"
#include "AutoCs.h"
#include "Timer_t.h"
#include "LonMessageTypes.h"

class LonCard_t;

/////////////////////////////////////////////////////////////////////////////

typedef std::vector<const Trade_t*> TradePtrVector_t;

struct CardTrades_t
{
    const Card_t*    pCard;
    TradePtrVector_t Offered;
    TradePtrVector_t Wanted;

    CardTrades_t()                : pCard(NULL) {}
    CardTrades_t(const Card_t* c) : pCard(c) {}

    typedef CardTrades_t T;
    bool operator()(const T& lhs, const T& rhs) const;
};

typedef std::set<CardTrades_t, CardTrades_t>  CardTradesSet_t;

/////////////////////////////////////////////////////////////////////////////

class TradeManager_t final :
    public DP::Handler_t
{

public:

    struct Flag
    {
        static const Flag_t MainTrades     = 0x1;
        static const Flag_t OtherTrades    = 0x2;
        static const Flag_t RemovedTrades  = 0x4;        
        static const Flag_t AllTrades      = 0x7;
    };

    class EventGatherTrades_t
    {
    public:
        struct Data_t :
            public Lon::Transaction::Data_t
        {
            Data_t() :
                Lon::Transaction::Data_t(
                    Lon::Transaction::Id::GatherTrades,
                    Lon::Window::PostedTradesWindow,
                    sizeof(Data_t))
            { }
        } m_Data;
    };

private:

    TradeMap_t    m_Trades;
    mutable CAutoCritSec  m_csTrades;
    mutable Timer_t       m_Timer;

public:

    TradeManager_t();
    ~TradeManager_t();

    //
    // DP::Handler_t virtual:
    //

    HRESULT
    MessageHandler(
        const DP::Message::Data_t* pData) override;

    HRESULT
    EventHandler(
        DP::Event::Data_t& Data) override;

    HRESULT
    ExecuteTransaction(
        const DP::Transaction::Data_t& Data) override;

    HRESULT
    OnTransactionComplete(
        const DP::Transaction::Data_t& Data) override;

    // Helpers:

    bool
    DoGatherTrades() const;

    bool
    GetTrade(
        TradeId_t Id,
        Trade_t&  Trade) const;

    bool
    FindTrade(
        TradeId_t Id);

    size_t
    FindTrades(
              LonCard_t&        FindCard,
        const CardCollection_t& AllowedCards,
        const size_t            MaxAllowed,
        const size_t            Order) const;

    void
    AddTrade(
        Trade_t& Trade);

    void
    RemoveTrades(
        const TradeIdSet_t& TradeIds);

    void
    ShowMissedTradeNumbers() const;

    void
    ShowTradeTotals() const;

    void
    ShowTrades(
        size_t Factor = 1) const;

    void
    ShowTrade(
        TradeId_t Id) const;

    size_t
    WriteAllTrades() const;

    size_t
    ReadAllTrades();

    size_t
    ReadTrades(
        CDatabase& db,
        Flag_t     Flags);

    void
    ValueAllTrades();

    bool
    CompareTrade(
              TradeId_t TradeId,
        const Trade_t&  Trade) const;

    bool
    WriteTradeXml(
              TradeId_t TradeId,
        const wchar_t*  pFilename = NULL);

    void
    GetTradesWithCards(
        const CardCollection_t& Cards,
        const bool              bOffered,
              TradeIdSet_t&     TradeIds) const;

    TradeId_t 
    GetMatchingTradeId(
        const Trade_t& FindTrade) const;

    void
    HackInitTransactions() const;

    ///////////////////////////////////////////////////////////////////
    //
    // Test code
    //

    // test function
    bool
    TestPostTrade(
        TradeId_t TradeId);

    ///////////////////////////////////////////////////////////////////

private:

    void
    OnGatherTradesComplete(
        const EventGatherTrades_t::Data_t& Data);

    void
    OnRemoveTrade(
        const TradeId_t TradeId);

    bool
    FindTrade(
        TradeId_t                   TradeId,
        TradeMap_t::const_iterator& it) const;

    bool
    TryAddAsk(
              LonCard_t& Card,
        const Trade_t&   Trade,
        const size_t     Count,
        const size_t     Order) const;

    bool
    TryAddBid(
              LonCard_t& Card,
        const Trade_t&   Trade,
        const size_t     Count,
        const size_t     Order) const;

    bool
    IsCyclicPricePath(
        const Card_t&       Card,
        const Trade_t&      Trade,
              bool          bBids,
              TradeIdSet_t& DependentTradeIds) const;

    bool
    RecursiveCyclicPriceCheck(
        const Trade_t&      Trade,
        const CardIdSet_t&  CyclicCardIds,
              bool          bBids,
              TradeIdSet_t& DependentTradeIds) const;

    bool
    ValueOneTrade(
        Trade_t& Trade);

    size_t
    WriteAllTrades(
        CDatabase& Db) const;

private:

    TradeManager_t(const TradeManager_t&);
    TradeManager_t& operator=(const TradeManager_t&);
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TRADEMANAGER_T_H

/////////////////////////////////////////////////////////////////////////////
