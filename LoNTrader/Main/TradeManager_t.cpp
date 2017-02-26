///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TradeManager_t.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TradeManager_t.h"
#include "Trade_t.h"
#include "Log.h"
#include "LonCardSet_t.h"
#include "DbTrades_t.h"
#include "DbGroups_t.h"
#include "DbGroupedCards_t.h"
#include "Timer_t.h"
#include "PipelineManager.h"
#include "LonWindow_t.h"
#include "LonTrader_t.h"
#include "Services.h"
#include "TradeExecutor_t.h"
#include "PcapTrades_t.h"
#include "PostedTradesTypes.h"

///////////////////////////////////////////////////////////////////////////////

#define EXTRALOG 0
#define TRADELOG 1

bool
CardTrades_t::
operator()(const T& lhs, const T& rhs) const
{
    return lhs.pCard->GetId() < rhs.pCard->GetId();
}

///////////////////////////////////////////////////////////////////////////////

TradeManager_t::
TradeManager_t()
{
}

///////////////////////////////////////////////////////////////////////////////

TradeManager_t::
~TradeManager_t()
{
}

///////////////////////////////////////////////////////////////////////////////

HRESULT
TradeManager_t::
EventHandler(
     DP::Event::Data_t& Data)
{
    HRESULT hr = DP::Handler_t::EventHandler(Data);
    if (S_FALSE != hr)
        return hr;
    using namespace Lon::Event;
    switch (Data.Id)
    {
    case Id::AddTrade:
        AddTrade(static_cast<PostedTrades::EventAddTrade_t::Data_t&>(Data).Trade);
        break;
    default:
        return S_FALSE;
    }
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT
TradeManager_t::
ExecuteTransaction(
    const DP::Transaction::Data_t& Data)
{
    using namespace Lon::Transaction;
    switch (Data.Id)
    {
    case Id::GatherTrades:
        m_Timer.Set();
        break;
    default:
        return S_FALSE;
    }
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT
TradeManager_t::
OnTransactionComplete(
    const DP::Transaction::Data_t& Data)
{
    using namespace Lon::Transaction;
    switch (Data.Id)
    {
    case Id::GatherTrades:
        OnGatherTradesComplete(
			static_cast<const EventGatherTrades_t::Data_t&>(Data));
        break;
    default:
        return S_FALSE;
    }
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT
TradeManager_t::
MessageHandler(
    const DP::Message::Data_t* pMessage)
{
    ASSERT(NULL != pMessage);
//    if (DP::Command != pData->Format)
//        return;
    if (Lon::Message::Id::RemoveTrade == pMessage->Id)
    {
        const PcapTrades_t::AcquireData_t& 
            PcapData = static_cast<const PcapTrades_t::AcquireData_t&>(*pMessage);
        OnRemoveTrade(PcapData.TradeId);
        return S_OK;
    }
    return S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////

bool
TradeManager_t::
DoGatherTrades() const
{
    LogAlways(L"TradeManager_t::DoGatherTrades()");
    DP::Transaction::Data_t *pData = new EventGatherTrades_t::Data_t;
    GetTransactionManager().ExecuteTransaction(pData);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void
TradeManager_t::
OnGatherTradesComplete(
    const EventGatherTrades_t::Data_t& Data)
{
    m_Timer.Show(L"GatherTrades");
    m_Timer.Set();
    if (Lon::Transaction::Error::None == Data.Error)
    {
        LogAlways(L"OnGatherTradesComplete() success!");
    }
    else
        LogError(L"OnGatherTradesComplete(%d))", Data.Error);
}

///////////////////////////////////////////////////////////////////////////////

bool
TradeManager_t::
FindTrade(
    TradeId_t TradeId)
{
    CLock lock(m_csTrades);
    TradeMap_t::const_iterator it = m_Trades.find(TradeId);
    if (m_Trades.end() != it)
        return true;
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool
TradeManager_t::
GetTrade(
    TradeId_t TradeId,
    Trade_t&  Trade) const
{
    CLock lock(m_csTrades);
    TradeMap_t::const_iterator it = m_Trades.find(TradeId);
    if (m_Trades.end() == it)
        return false;
    Trade = it->second;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void
TradeManager_t::
AddTrade(
    Trade_t& Trade)
{
//    Trade.offer.Value = Trade.GetHighBid(Trade.offer.Items);
//    Trade.want.Value  = Trade.GetLowAsk(Trade.want.Items);
//    bool bRemoved = 0 != (Trade.m_Flags & Trade_t::Flag::Removed);
    bool bHasLoot = Trade.HasLootCard();
    if (bHasLoot)
        Trade.SetFlags(Trade_t::Flag::HasLootCard);
    else
        Trade.SetFlags(Trade_t::Flag::Other);

    TradeMap_t& map = m_Trades;
    CLock lock(m_csTrades);
    std::pair<TradeMap_t::iterator, bool>
        Pair = map.insert(/*map.begin(), */Trade_t::Pair_t(Trade.GetId(), Trade));
    if (!Pair.second)
    {
        LogError(L"AddTrade(%d) failed.", Trade.GetId());
        LogAlways(L"Existing:");
        Pair.first->second.Show();
        LogAlways(L"Adding:");
        Trade.Show();
    }
}

///////////////////////////////////////////////////////////////////////////////

void
TradeManager_t::
RemoveTrades(
    const TradeIdSet_t& TradeIds)
{
    {
        CLock lock(m_csTrades);
        TradeIdSet_t::const_iterator itTradeId(TradeIds.begin());
        for (; TradeIds.end() != itTradeId; ++itTradeId)
        {
            TradeMap_t::iterator it = m_Trades.find(*itTradeId);
            if (m_Trades.end() == it)
            {
                LogError(L"RemoveTrades: Invalid TradeId (%d)", *itTradeId);
                return;
            }
            // Trades removed in this manner go to lalaland, not removed trade
            // list as they were manually removed, not "sold".
/*
            Trade_t& Trade = it->second;
            Trade.m_Flags |= Trade_t::Flag::Removed;
*/
            m_Trades.erase(it);
        }
    }
    Services::GetCardSet().OnRemoveTrades(TradeIds);
}

///////////////////////////////////////////////////////////////////////////////

void
TradeManager_t::
OnRemoveTrade(
    const TradeId_t TradeId)
{
    LogInfo(L"TM::RemoveTrade(%d)", TradeId);
    {
        CLock lock(m_csTrades);
        TradeMap_t::iterator it = m_Trades.find(TradeId);
        if (m_Trades.end() == it)
            return;
        Trade_t& Trade = it->second;
        Trade.SetFlags(Trade_t::Flag::Removed);
        Trade.SetOfferValue(Trade.GetHighBid());
        Trade.SetWantValue(Trade.GetLowAsk());
        Trade.SetRemovedTime();
    }
    Services::GetCardSet().OnRemoveTrade(TradeId);
}

///////////////////////////////////////////////////////////////////////////////

void
TradeManager_t::
ShowTradeTotals() const
{
    size_t Main  = 0;
    size_t Other   = 0;
    size_t Removed = 0;

    CLock lock(m_csTrades);
    TradeMap_t::const_iterator it = m_Trades.begin();
    for(; m_Trades.end() != it; ++it)
    {
        const Trade_t& Trade = it->second;
        if (Trade.TestFlags(Trade_t::Flag::Other))
            ++Other;
        else if (Trade.TestFlags(Trade_t::Flag::Removed))
            ++Removed;
        else
        {
            ASSERT(Trade.TestFlags(Trade_t::Flag::HasLootCard));
            ++Main;
        }
    }
    LogAlways(L"Trades: Main (%d) Other (%d) Removed (%d) Total (%d)",
              Main, Other, Removed, m_Trades.size());
}

///////////////////////////////////////////////////////////////////////////////

void
TradeManager_t::
ShowTrades(
    size_t Factor) const
{
    int iFactorCount = 0;
    int iCount = 0;

    CLock lock(m_csTrades);
    TradeMap_t::const_iterator it = m_Trades.begin();
    for (; m_Trades.end() != it; ++it, ++iCount)
    {
        const Trade_t& trade = it->second;
        if (0 < Factor)
        {
            size_t HighBid = trade.GetHighBid();
            if (0 == HighBid)
                continue;

            size_t LowAsk = trade.GetLowAsk();
            if (1000 > LowAsk)
                continue;

            if (HighBid < LowAsk * Factor)
                continue;
        }
        trade.Show();
        ++iFactorCount;
    }
    LogAlways(L"Total Trades: %d, [%d:1] Factor Trades: %d\n", iCount, Factor, iFactorCount);
}

///////////////////////////////////////////////////////////////////////////////

void
TradeManager_t::
ShowTrade(
    TradeId_t TradeId) const
{
    Trade_t Trade;
    if (GetTrade(TradeId, Trade))
        Trade.Show();
}

///////////////////////////////////////////////////////////////////////////////

bool
TradeManager_t::
CompareTrade(
    TradeId_t      TradeId,
    const Trade_t& Trade) const
{
    CLock lock(m_csTrades);
    TradeMap_t::const_iterator it = m_Trades.find(TradeId);
    if (m_Trades.end() == it)
        return false;
    return it->second.Compare(Trade);
}

///////////////////////////////////////////////////////////////////////////////

size_t
TradeManager_t::
WriteAllTrades() const
{
    Timer_t Timer(L"WriteAllTrades()");
    size_t Count = 0;
    CDatabase db;
    try
    {
        Count = WriteAllTrades(db);
        LogAlways(L"Writing done (%d)", Count);
    }
    catch(CDBException *e)
    {
        LogError(L"WriteAllTrades() exception: %ls", e->m_strError);
    }
    return Count;
}

///////////////////////////////////////////////////////////////////////////////

size_t
TradeManager_t::
WriteAllTrades(
    CDatabase& db) const
{
    size_t Count = 0;
    BOOL b = db.Open(NULL, FALSE, FALSE, LonTrader_t::GetDbConnectString(), FALSE);
    ASSERT(!!b);
    {
        CLock lock(m_csTrades);
        Count  = m_Trades.WriteTrades(db);
    }
    db.Close();
    return Count;
}

///////////////////////////////////////////////////////////////////////////////

void
TradeManager_t::
HackInitTransactions() const
{
    CLock lock(m_csTrades);
    Services::GetCardSet().InitTransactions(m_Trades);
}

///////////////////////////////////////////////////////////////////////////////

size_t
TradeManager_t::
ReadAllTrades()
{
    extern DWORD ccLookupTicks;
    ccLookupTicks = 0;

    Timer_t Timer(L"ReadAllTrades()");
    CDatabase db;
    size_t Count = 0;
    try
    {
        BOOL b = db.Open(NULL, FALSE, FALSE, LonTrader_t::GetDbConnectString(), FALSE);
        ASSERT(!!b);
        Count = ReadTrades(db, Flag::AllTrades);
        LogAlways(L"  Reading done (%d) ccLookup(%d)", Count, ccLookupTicks);
//        Services::GetCardSet().InitTransactions(m_RemovedTrades);
    }
    catch(CDBException *e)
    {
        LogError(L"ReadAllTrades() exception: %ls", e->m_strError);
        throw std::runtime_error("ReadAllTrades");
    }
    db.Close();
    return Count;
}

///////////////////////////////////////////////////////////////////////////////

size_t
TradeManager_t::
ReadTrades(
    CDatabase& db,
    Flag_t     Flags)
{
    size_t Count = 0;
    DbTrades_t dbTrades(&db);
    BOOL b = dbTrades.Open(CRecordset::forwardOnly, NULL, CRecordset::readOnly);
    ASSERT(!!b);
    for (; !dbTrades.IsEOF(); dbTrades.MoveNext())
    {
        Trade_t Trade;

        Flag_t TradeFlags = dbTrades.m_flags;
        if ((0 != (TradeFlags & Trade_t::Flag::Removed)) &&
            (0 == (Flags & Flag::RemovedTrades)))
                continue;             

        if ((0 != (TradeFlags & Trade_t::Flag::Other)) &&
            (0 == (Flags & Flag::OtherTrades)))
                continue;             

        if ((0 == (TradeFlags & Trade_t::Flag::Removed)) &&
            (0 == (TradeFlags & Trade_t::Flag::Other))  &&
            (0 == (Flags & Flag::MainTrades)))
                continue;             

        Trade.Read(db, dbTrades);
        AddTrade(Trade);
        ++Count;
    }
    dbTrades.Close();
    return Count;
}

///////////////////////////////////////////////////////////////////////////////
//
// Find trades in which FindCard is the only card offered or wanted.
// The corresponding wanted or offered collection may only contain 
// cards from the AllowedCards collection, and may not contain more
// than MaxAllowed cards total.
//

size_t
TradeManager_t::
FindTrades(
          LonCard_t&        FindCard,
    const CardCollection_t& AllowedCards,
    const size_t            MaxAllowed,
    const size_t            Order) const
{
    size_t Total = 0;
    CLock lock(m_csTrades);
    TradeMap_t::const_iterator it = m_Trades.begin();
    for (; m_Trades.end() != it; ++it)
    {
        const Trade_t& Trade = it->second;
        if (!Trade.IsActive())
            continue;

        if ((MaxAllowed < Trade.GetOfferCards().size()) ||
            (MaxAllowed < Trade.GetWantCards().size()))
                continue;

        size_t Offered = 0;
        size_t Wanted = 0;
        if (!Trade.IsOffered(&FindCard, &Offered) &&
            !Trade.IsWanted(&FindCard, &Wanted))
                continue;

        // Need to do some thankin' before I support this.
        if ((0 < Offered) && (0 < Wanted))
        {
            LogInfo(L"FindTrades: Item in offer and want list, skipping (%d)",
                    Trade.GetId());
            continue;
        }

        size_t New = 0;
        if (0 < Offered)
        {
            if ((1 == Trade.GetOfferCards().size()) &&
                Trade.GetWantCards().HasOnly(AllowedCards))
            {                    
                if (TryAddAsk(FindCard, Trade, Offered, Order))
                    ++New;
            }
        }
        else
        {
            if ((1 == Trade.GetWantCards().size()) &&
                Trade.GetOfferCards().HasOnly(AllowedCards))
            {                    
                if (TryAddBid(FindCard, Trade, Wanted, Order))
                    ++New;
            }
        }
        Total += New;
    }
    return Total;
}

///////////////////////////////////////////////////////////////////////////////

bool
TradeManager_t::
TryAddAsk(
          LonCard_t& Card,
    const Trade_t&   Trade,
    const size_t     Count,
    const size_t     Order) const
{
    bool bAdded = false;

    // NOTE: This works because Card is the only item offered in Trade.
    const size_t Value = Trade.GetLowAsk() / Count;
    if (0 < Value)
    {
        TradeIdSet_t DependentTradeIds;
        if (!IsCyclicPricePath(Card, Trade, false, DependentTradeIds))
        {
            const TradeValue_t tv(Trade.GetId(), Value, Count, Order);
            Card.AddAsk(tv);
            Card.GetAsks().SetDependentTrades(DependentTradeIds);
            bAdded = true;
        }
    }
    else
    {
        // TODO: what if it's already in set, and value is reduced
        // to zero? assert that it's not in set?
        TradeValue_t tv(Trade.GetId());
        ASSERT(Card.GetAsks().TradeValues.end() == Card.GetAsks().TradeValues.find(tv));
    }
    return bAdded;
}

///////////////////////////////////////////////////////////////////////////////

bool
TradeManager_t::
TryAddBid(
          LonCard_t& Card,
    const Trade_t&   Trade,
    const size_t     Count,
    const size_t     Order) const
{
    bool bAdded = false;
    const size_t Value = Trade.GetHighBid() / Count;
    if (0 < Value)
    {
        TradeIdSet_t DependentTradeIds;
        if (!IsCyclicPricePath(Card, Trade, true, DependentTradeIds))
        {
            const TradeValue_t tv(Trade.GetId(), Value, Count, Order);
            Card.AddBid(tv);
            Card.GetBids().SetDependentTrades(DependentTradeIds);
            bAdded = true;
        }
    }
    else
    {
        // TODO: what if it's already in set, and value is reduced
        // to zero? assert that it's not in set?
        TradeValue_t tv(Trade.GetId());
        ASSERT(Card.GetBids().TradeValues.end() == Card.GetBids().TradeValues.find(tv));
    }
    return bAdded;
}

///////////////////////////////////////////////////////////////////////////////

bool
TradeManager_t::
IsCyclicPricePath(
    const Card_t&       Card,
    const Trade_t&      Trade,
          bool          bBids,
          TradeIdSet_t& DependentTradeIds) const
{
    DependentTradeIds.insert(Trade.GetId());

    CardIdSet_t  CyclicCardIds;
    CyclicCardIds.insert(Card.GetId());
    bool bCyclic = RecursiveCyclicPriceCheck(Trade, CyclicCardIds, bBids, DependentTradeIds);
    if (bCyclic)
    {
#if EXTRALOG
        LogInfo(L"Cyclic pricing found in (%d) for '%s'",
                Trade.GetId(), Card.GetName());
#endif
    }
    return bCyclic;
}

///////////////////////////////////////////////////////////////////////////////

bool
TradeManager_t::
RecursiveCyclicPriceCheck(
    const Trade_t&      Trade,
    const CardIdSet_t&  CyclicCardIds,
          bool          bBids,
          TradeIdSet_t& DependentTradeIds) const
{
    const CardCollection_t& Cards = bBids ? Trade.GetOfferCards()
                                          : Trade.GetWantCards();
    CardCollection_t::const_iterator it    = Cards.begin();
    CardCollection_t::const_iterator itEnd = Cards.end();
    // For each offered/wanted item in the trade
    for (; itEnd != it; ++it)
    {
        const LonCard_t* pCard = static_cast<const LonCard_t*>(it->pCard);
        ASSERT(NULL != pCard);
        if (NULL == pCard)
            continue;

/*
        // Questionable opt: ignore non-loot cards.
        if (!pCard->IsLootCard())
            continue;
*/

        // Everything is ultimately priced on innate value cards.
        if (0 < pCard->GetValue()) //pCard->IsBoosterPack())
            continue;

        // Ensure current card is not a cyclic reference.
        if (CyclicCardIds.end() != CyclicCardIds.find(pCard->GetId()))
        {
#if EXTRALOG
            LogInfo(L"Cyclic pricing found in (%d) for '%s' (size=%d)",
                    Trade.GetId(), pCard->GetName(), CyclicCardIds.size());
#endif
            return true; // cyclic
        }

        TradeId_t TradeId = bBids ? pCard->GetBids().TradeId
                                  : pCard->GetAsks().TradeId;
        // NOTE: Weird case that happens after a trade is removed, resulting
        // in the one and only bid for a card in that trade being removed.
        // Any other trades that contain cards priced based on the trade that
        // now has no bids, will need to be repriced.
        // I should probably calculate/set that flag earlier.
        if (0 == TradeId)
            continue;

        // Find the trade in which the high bid or low ask was determined.
        // NOTE: what if multiple trades have a matching bid/ask value
        // TODO: what if tradeid is invalid/has been removed, we need to
        //       remove tradevalue from this card's set and go to next
        //       highest bid
        TradeMap_t::const_iterator ItTrade = m_Trades.find(TradeId);
        ASSERT(m_Trades.end() != ItTrade);

        // Make a copy of the cyclic card id set, and insert the current
        // card id into it. 
        CardIdSet_t CyclicCardIdsCopy(CyclicCardIds);
        CyclicCardIdsCopy.insert(pCard->GetId());
        TradeIdSet_t TempTradeIds;
        if (RecursiveCyclicPriceCheck(ItTrade->second, CyclicCardIdsCopy, bBids, TempTradeIds))
            return true;
        TempTradeIds.insert(ItTrade->second.GetId());
        DependentTradeIds.insert(TempTradeIds.begin(), TempTradeIds.end());
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
//
// walk all trades
//    if card in (offer/want) collection
//        add tradeid to set
//

void
TradeManager_t::
GetTradesWithCards(
    const CardCollection_t& Cards,
    const bool              bOffered,
          TradeIdSet_t&     TradeIds) const
{
    CLock lock(m_csTrades);
    TradeMap_t::const_iterator it = m_Trades.begin();
    for(; m_Trades.end() != it; ++it)
    {
        const Trade_t& Trade = it->second;
        if (!Trade.IsActive())
            continue;
        const CardCollection_t&
            TradeCards = (bOffered) ? Trade.GetOfferCards()
                                    : Trade.GetWantCards();
        CardCollection_t CommonCards;
        std::insert_iterator<CardCollection_t>
            itCommonCards(CommonCards, CommonCards.begin());
        std::set_intersection(Cards.begin(), Cards.end(),
                              TradeCards.begin(), TradeCards.end(),
                              itCommonCards, CardQuantity_t());
        if (!CommonCards.empty())
        {
            LogWarning(L"GetTradesWithCards: Adding (%d)", Trade.GetId());
            TradeIds.insert(Trade.GetId());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
// Useless function - Trades aren't valued, their component cards are.
//

void
TradeManager_t::
ValueAllTrades()
{
    Timer_t Timer(L"ValueAllTrades()");
    CLock lock(m_csTrades);
    TradeMap_t::iterator it = m_Trades.begin();
    for (; m_Trades.end() != it; ++it)
    {
        Trade_t& trade = it->second;
        if (!trade.IsActive())
            continue;
        ValueOneTrade(trade);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
// Useless function - Trades aren't valued, their component cards are.
//

bool
TradeManager_t::
ValueOneTrade(Trade_t& trade)
{
trade;
#if 0
    // NOTE: probably only need to get_value if want.Value == 0
    size_t dw = trade.GetLowAsk(trade.want.Items);
    if (0 < dw)
    {
#if TRADELOG
        if (trade.want.Value > 0 && trade.want.Value != dw)
            LogInfo(L"Trade %d old want: %d != new want: %d", trade.id, trade.want.Value, dw);
#endif
        trade.want.Value = dw;
    }
    else
    {
#if TRADELOG
        if (trade.want.Value > 0)
            LogInfo(L"Trade %d old want: %d != new want zero", trade.id, trade.want.Value);
#endif
        trade.want.Value = 0;
    }

    dw = trade.GetHighBid(trade.offer.Items);
    if (0 < dw)
    {
#if TRADELOG
        if (trade.offer.Value > 0 && trade.offer.Value != dw)
            LogInfo(L"Trade %d old offer %d != new offer: %d", trade.id, trade.offer.Value, dw);
#endif
        trade.offer.Value = dw;
    }
    else
    {
#if TRADELOG
        if (trade.offer.Value > 0)
            LogInfo(L"Trade %d old offer: %d != new offer zero", trade.id, trade.offer.Value);
#endif
        trade.offer.Value = 0;
    }
    return (0 != trade.want.Value) && (0 != trade.offer.Value);
#else
    return false;
#endif
}

///////////////////////////////////////////////////////////////////////////////

bool
TradeManager_t::
WriteTradeXml(
          TradeId_t TradeId,
    const wchar_t*  pFilename)
{
    CLock lock(m_csTrades);
    TradeMap_t::const_iterator it = m_Trades.find(TradeId);
    if (m_Trades.end() == it)
    {
        LogError(L"WriteTradeXml(%d): Trade not found.", TradeId);
        return false;
    }
    wchar_t szFilename[64];
    if (NULL == pFilename)
    {
        wsprintf(szFilename, L"Trade_%d.xml", TradeId);
        pFilename = szFilename;
    }
    return it->second.WriteXmlFile(pFilename);
}

///////////////////////////////////////////////////////////////////////////////

TradeId_t 
TradeManager_t::
GetMatchingTradeId(
    const Trade_t& FindTrade) const
{
    CLock lock(m_csTrades);
    TradeMap_t::const_iterator it = m_Trades.begin();
    for(; m_Trades.end() != it; ++it)
    {
        const Trade_t& Trade = it->second;
        if (!Trade.IsActive())
            continue;
        if (Trade.Compare(FindTrade, true))
            return Trade.GetId();
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// test function
//

bool
TradeManager_t::
TestPostTrade(
   TradeId_t TradeId)
{
    LogAlways(L"TradeManager::PostTrade(%d)", TradeId);

    CLock lock(m_csTrades);
    TradeMap_t::iterator it = m_Trades.find(TradeId);
    if (m_Trades.end() == it)
        return false;
    return Services::GetTradeExecutor().PostTrade(it->second, 0, 0, true);
}

///////////////////////////////////////////////////////////////////////////////

