///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TradePoster.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif

#ifndef Include_TRADEPOSTER_H
#define Include_TRADEPOSTER_H

#include "DpHandler_t.h"
#include "TradePosterTypes.h"
#include "TradePosterData.h"
#include "AutoCs.h"
#include "AutoHand.h"

class LonCard_t;

///////////////////////////////////////////////////////////////////////////////

namespace TradePoster
{
    typedef std::multimap<CardId_t, Data_t> Map_t;
    typedef std::pair<CardId_t, Data_t>     Pair_t;

///////////////////////////////////////////////////////////////////////////////
//
// TradePoster::Manager_t
//
///////////////////////////////////////////////////////////////////////////////
   
class Manager_t :
    public DP::Handler_t
{

private:

    static volatile    LONG m_NextId;
    // NOTE: threadsafe?
    static             Id_t GetNextId()    { return InterlockedIncrement(&m_NextId); }

private:

    Map_t           m_Map;
    CardValueSet_t  m_DesiredCardValues;
    CardValueSet_t  m_ActualCardValues;
    mutable 
    CAutoCritSec    m_csMap;              // really, this is m_csData, locks everything above
    CAutoHandle     m_hTimer;

protected:

    //
    // DP::Handler_t virtual:
    //

    bool
    Initialize(
        const wchar_t* pszClass);

    virtual
    HRESULT
    MessageHandler(
        const DP::Message::Data_t* pData);

    HRESULT
    EventHandler(
        DP::Event::Data_t& Data);

    virtual
    HRESULT
    OnTransactionComplete(
        const DP::Transaction::Data_t& Data);

    // Helpers

public:

    Manager_t();

    void
    BuyCard(
        const LonCard_t* pCard,
              size_t     Low,
              size_t     High,
              size_t     Increment,
              size_t     Count,
              Flag_t     Flags = 0);

    void
    SellCard(
        const LonCard_t* pCard,
              size_t     Low,
              size_t     High,
              size_t     Increment,
              size_t     Count,
              Flag_t     Flags = 0);

    Id_t
    BuyCard(
        const CardQuantity_t&  CardQ,
              PricingPolicy_i* pPricingPolicy,
              PostingPolicy_i* pPostingPolicy)
    {
        return AddPostData(
            Data_t(GetNextId(), CardQ, Data_t::Want,
                   pPricingPolicy, pPostingPolicy));
    }

    Id_t
    SellCard(
        const CardQuantity_t&  CardQ,
              PricingPolicy_i* pPricingPolicy,
              PostingPolicy_i* pPostingPolicy)
    {
        return AddPostData(
            Data_t(GetNextId(), CardQ, Data_t::Offer,
                   pPricingPolicy, pPostingPolicy));
    }

    size_t
    GenerateTrades(
        size_t MaxPerAllowedCard) const;

    size_t
    GenerateTrades(
        Id_t   Id,
        size_t MaxPerAllowedCard) const;

    size_t
    GenerateTrades(
        Id_t   Id,
        size_t Value,
        size_t MaxPerAllowedCard) const;

    size_t
    RemoveTrades(
        Id_t   Id,
        bool   bTest);

    size_t
    RemoveTrades(
        Id_t   Id,
        size_t Value,
        bool   bTest);

    bool
    RemoveTrade(
        Id_t      Id,
        size_t    Value,
        TradeId_t TradeId,
        bool      bTest);

    bool
    Remove(
        Id_t Id);

    void
    Show(
        bool bDetail = false) const;

    void
    Show(
        Id_t Id,
        bool bDetail = false) const;

    void
    Show(
        Id_t   Id,
        size_t Value,
        bool   bDetail = false) const;

    void
    ShowAll(
        Flag_t Flags = Flag::ShowAllTrades) const;

    size_t
    WriteXmlDirectory(
        const wchar_t* pszDir) const;

    bool
    WriteXmlDirectory(
        const wchar_t* pszDir,
              Id_t     Id) const;

    bool
    ReadXmlFile(
        const wchar_t* pszFilename);

    bool
    WriteXmlDesiredCardValues(
        const wchar_t* pszDir) const;

    bool
    ReadXmlDesiredCardValues(
        const wchar_t* pszDir);

    // TODO: Move to PM
    void
    QueueSetTimer(
        size_t Seconds);

    size_t
    InitDesiredCardValues();

    size_t
    InitActualCardValues();

    size_t
    PostAllTradesAtDesiredValues(
        bool bTest);

    size_t
    PostTrades(
        Id_t      PosterId,
        size_t    Value,
        TradeId_t TradeId,
        bool      bTest);

public:

    // these should be private? and LonTrader/CmdTrades should be a friend?
    PricingPolicy_i*
    GetPricingPolicy(
        Id_t Id);

    PostingPolicy_i*
    GetPostingPolicy(
        Id_t Id);

    void
    ShowCardValues() const;

    void
    SetCardValue(
        const CardValue_t& CardValue);

private:

    bool
    OnPostTradeComplete(
        const EventPost_t::Data_t& Data);

    void
    OnGatherTradesComplete();

    static
    void
    CALLBACK
    TimerSetApc(
        ULONG_PTR Param);

    void
    SetTimer(
       size_t Seconds);

    static
    void
    CALLBACK
    TimerTickApc(
        void* pArg,
        DWORD dwTimerLowValue,
        DWORD dwTimerHighValue);

    void
    OnTimer();

    void
    OnAddTrade(
        Trade_t& Trade);

    void
    OnRemoveTrade(
        TradeId_t TradeId);

    // could also be/have "update post data"
    Id_t
    AddPostData(
       const Data_t& Data);

    Map_t::const_iterator
    MapFindId(
        Id_t Id) const;

    Map_t::iterator
    MapFindId(
        Id_t Id);

    size_t
    GenerateTrades(
        const Data_t& Data,
              size_t  MaxPerAllowedCard) const;

    PricingPolicy_i*
    NewPricingPolicy(
        size_t Low,
        size_t High,
        size_t Increment,
        Flag_t Flags) const;

    bool
    AddBoosters(
        CardQuantityQueue_t& Cards,
        bool                 bBuying);

    size_t
    InitCardValues(
        CardValueSet_t& Cards) const;

    size_t
    InitCardValues(
        Id_t            Id); // const;
//,        CardValueSet_t& Cards) const;

    size_t
    PostAllTrades(
        const CardValueSet_t& CardValues,
              bool            bTest);

private:

    Manager_t(const Manager_t&);
    Manager_t& operator=(const Manager_t&);
};

///////////////////////////////////////////////////////////////////////////////

} // TradePoster

#endif //  Include_TRADEPOSTER_H

///////////////////////////////////////////////////////////////////////////////
