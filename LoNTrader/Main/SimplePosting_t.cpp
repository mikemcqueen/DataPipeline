///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// SimplePosting_t.cpp
//
// Simple trade posting policy.
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SimplePosting_t.h"
#include "LonCard_t.h"
#include "XmlUtil.h"
#include "XmlFile.h"
#include "Log.h"
#include "TradeExecutor_t.h"
#include "TradeManager_t.h"
#include "LonTrader_t.h"
#include "LonPlayer_t.h"
#include "Services.h"

namespace TradePoster
{

const wchar_t SimplePosting_t::PolicyName[]             = L"Simple";
const wchar_t SimplePosting_t::XmlElementAllowedCards[] = L"AllowedCards";
//const wchar_t SimplePosting_t::XmlElementTradeMaps[]    = L"TradeMaps";

///////////////////////////////////////////////////////////////////////////////
//
// SimplePosting_t
//
///////////////////////////////////////////////////////////////////////////////

SimplePosting_t::
SimplePosting_t(
    const CardQuantity_t& CardQ)
:
    m_CardQ(CardQ)
{
}

///////////////////////////////////////////////////////////////////////////////

SimplePosting_t::
SimplePosting_t(
    const CardQuantity_t&      CardQ,
    const CardQuantityQueue_t& Cards)
:
    m_CardQ(CardQ),
    m_AllowedCards(Cards)
{
}

///////////////////////////////////////////////////////////////////////////////

size_t
SimplePosting_t::
GenerateTrades(
    Data_t::Type_e Type,
    size_t         Value,
    size_t         MaxPerAllowedCard,
    size_t         MaxTotal)
{
    CLock lock(m_csTrades);
    LogAlways(L"++GenerateTrades(%d,%d)", Value, MaxPerAllowedCard);
    size_t TradeCount =
        m_TradeMaker.BuildTrades(
            (Data_t::Want == Type), // bBuy
            m_CardQ,            // Card, and max # of cards to use per trade
            Value,
            m_AllowedCards,     // Cards allowed in trade
            MaxPerAllowedCard,  // Max # of trades per allowed card
            MaxTotal);          // Total # of trades to generate
    LogAlways(L"--GenerateTrades(%d,%d)", Value, TradeCount);
    return TradeCount;
}

///////////////////////////////////////////////////////////////////////////////
//
// 
//

size_t
SimplePosting_t::
PostAllTrades(
    Id_t   TradePosterId,  // This represents a card, and either buy or sell
    size_t Value,
    Flag_t Flags)
{
    // We can't iterate from begin() to end() because ReplaceTradeId
    // will invalidate our iterator. So, build a trade id set and iterate
    // through that instead.
    const bool bTest = 1 == (Flag::TestPost & Flags);
    size_t Count = 0;
    CLock lock(m_csTrades);
    TradeList_t& Trades = GetTrades(Value);
    TradeList_t::iterator it = Trades.begin();
    for (; Trades.end() != it; ++it)
    {
        if (PostOrReplaceTrade(*it, TradePosterId, Value, bTest))
            ++Count;
    }
    return Count;
}

///////////////////////////////////////////////////////////////////////////////

size_t
SimplePosting_t::
PostTrade(
    Id_t      TradePosterId,
    size_t    Value,
    TradeId_t TradeId,
    Flag_t    Flags)
{
#if 1
    TradePosterId;Value;TradeId;Flags;
    LogError(L"TODO: PostTrade() not implemeneted");
    return 0;
#else
    CLock lock(m_csTrades);
    size_t Count = 0;
    bool bTest = 1 == (Flag::TestPost & Flags);
    if (0 != TradeId)
    {
        TradeMap_t::iterator it = GetTrades(Value).find(TradeId);
        if (GetTrades().end() != it)
        {
            if (PostOrReplaceTrade(it, bTest, TradePosterId))
                ++Count;
        }
        else
            LogError(L"Simple: Trade not found (%d)", TradeId);
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////

size_t
SimplePosting_t::
RemoveAllTrades(
    Id_t   TradePosterId,
    size_t Value,
    Flag_t Flags)
{
TradePosterId;
    // We can't iterate from begin() to end() because ReplaceTradeId
    // will invalidate our iterator. So, build a trade id set and iterate
    // through that instead.
    // Actually we probably could iterate from GetTrades().begin() -> .end()
    // now, since transactions are ApcQueued, and we have the lock.
    const bool bTest = 1 == (Flag::TestPost & Flags);
    size_t Count = 0;
    CLock lock(m_csTrades);
    // TODO: HACK: this is bogus, GetTrades() should probably return a pointer,
    // or should call HasTrades(Value) first, or GetTrades could return
    // bogus empty trade set
    TradeIdSet_t TradeIds(GetTrades(Value));
    TradeIdSet_t::const_iterator it = TradeIds.begin();
    for (; TradeIds.end() != it; ++it)
    {
        ASSERT(GetTrades(Value).HasTrade(*it));
        Services::GetTradeExecutor().RemoveTrade(*it, bTest);
        ++Count;
    }
    return Count;
}

///////////////////////////////////////////////////////////////////////////////

size_t
SimplePosting_t::
RemoveTrade(
    Id_t      TradePosterId,
    size_t    Value,
    TradeId_t TradeId,
    Flag_t    Flags)
{
TradePosterId; Value; Flags; 
//    size_t Count = 0;
//    const bool bTest = 1 == (Flag::TestPost & Flags);
    if (0 != TradeId)
    {
/*
        TradeMap_t::iterator it = GetTrades().find(TradeId);
        if (GetTrades().end() != it)
        {
            if (PostOrReplaceTrade(it, bTest, TradePosterId))
                ++Count;
        }
        else
*/
    }
    LogError(L"SimplePosting_t::RemoveTrade(%d) Not implemented", TradeId);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void
SimplePosting_t::
EraseAllTrades()
{
    CLock lock(m_csTrades);
    m_TradeMaker.EraseAll();
}

///////////////////////////////////////////////////////////////////////////////
//
// Return true if a trade post was executed.
//

bool
SimplePosting_t::
PostOrReplaceTrade(
    Trade_t& Trade,
    Id_t     TradePosterId,
    size_t   Value,
    bool     bTest)
{
    // Bail if trade id is already posted and active 
    Trade_t PostedTrade;
    if (0 < Trade.GetId())
    {
        if (Services::GetTradeManager().GetTrade(Trade.GetId(), PostedTrade))
        {
            if (0 == (Trade_t::Flag::Removed & PostedTrade.GetFlags()))
                return false;
        }
        else
        {
            // TODO: If RealId() && !In trade manager? Reset Id to zero? Log a message?
            LogError(L"Valid Trade Id not in TradeManager");
        }
    }

    // An active trade doesn't exist by the supplied Id. Try to find a match
    // of the username/offer/want items.
    TradeId_t MatchId = Services::GetTradeManager().GetMatchingTradeId(Trade);
    if (0 != MatchId)
    {
        // A matching trade is already posted. Replace the trade in our map 
        // with the matching posted trade id.
        Trade.SetId(MatchId);
        return false;
    }

    // See if we have the Offer cards to re-issue the same trade.
    if (!bTest &&
        !Trade.GetOfferCards().
            HasOnly(LonTrader_t::GetPlayer().GetYourCards(), true))
    {
        return false;
    }

    // No matching trade found, and we have the cards to cover the post.
    Services::GetTradeExecutor().PostTrade(Trade, TradePosterId, Value, bTest);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void
SimplePosting_t::
OnTradePosted(
    const Trade_t&  Trade,
          TradeId_t NewTradeId,
          size_t    Value)
{
    CLock lock(m_csTrades);
    // TODO: if (0 < Value) { 
    // HACK: This is goofy and messy. Trade and pTrade are actually the
    //       same thing here.

    const_cast<Trade_t&>(Trade).SetId(NewTradeId);
    Value;
/*
    TradeList_t& Trades = m_TradeMaker.GetTrades(Value);
    Trade_t* pTrade = Trades.Find(Trade);
    if (NULL == pTrade)
    {
        LogError(L"Trade Id (%d) not found", Trade.GetId());
    }
*/
}

///////////////////////////////////////////////////////////////////////////////

void
SimplePosting_t::
OnTradeAdded(
    const Trade_t& Trade)
{
    Trade;
}

///////////////////////////////////////////////////////////////////////////////

void
SimplePosting_t::
OnTradeRemoved(
    const TradePoster::Data_t& Data,
          TradeId_t            TradeId)
{
#if 0
Data; TradeId;
LogError(L"OnTradeRemoved Not Implemented.");
return;
#else
    Data;
    // so what do we do here.  are we calling every trade poster data for every 
    // remove?  don't need to do that (based on current implementation, with timer,
    // anyway).  can at least check if trade username from trade poster is our
    // username.. not really sure of teh point of that optimization, may want
    // to measure it.
    // Is TradePoster::Data_t any use to us here?

    CLock lock(m_csTrades);
    Trade_t* pTrade = m_TradeMaker.FindTrade(TradeId);
    if (NULL != pTrade)
    {
        // If one of our trades was removed, update our internal TradeId to
        // a "non-posted" value.
        pTrade->SetId(0);

        // TODO: Policy should have a quota and check if it's met.
        // This will be called on the next TradePoster::ApcTimerTick()
        // PostTrade(NewTradeId, 0, Data.Id);
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////

#if 0
TradeId_t
SimplePosting_t::
GetNewTradeId(
    const TradeMap_t& TradeMap) const
{
    TradeId_t TradeId = TradeMap.size();
    for (; MaxTradeCount > TradeId; ++TradeId)
    {
        TradeMap_t::const_iterator it = TradeMap.find(TradeId);
        if (TradeMap.end() == it)
            return TradeId;
    }
    throw std::out_of_range("GetNewTradeId()");
}
#endif

///////////////////////////////////////////////////////////////////////////////

void
SimplePosting_t::
AddAllowedCard(
    const CardQuantity_t& CardQ)
{
    m_AllowedCards.push_back(CardQ);
}

///////////////////////////////////////////////////////////////////////////////

bool
SimplePosting_t::
RemoveAllowedCard(
    const CardQuantity_t& CardQ)
{
    LogAlways(L"RemoveAllowedCard(%ls,%d)",
              CardQ.pCard->GetName(), CardQ.Quantity);
    CardQuantityQueue_t::iterator it =
        std::find_if(
            m_AllowedCards.begin(),
            m_AllowedCards.end(),
            CardQuantity_t::CompareCardId(CardQ.CardId));
    if (m_AllowedCards.end() != it)
    {
        m_AllowedCards.erase(it);
        LogAlways(L"%ls removed", CardQ.pCard->GetName());
        return true;
    }
    LogError(L"%ls not found.", CardQ.pCard->GetName());
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void
SimplePosting_t::
Show(
    size_t Value,
    bool   bDetail) const
{
    CLock lock(m_csTrades);
    LogAlways(L"%ls - AllowedCards(%d)", // TradeCount(%ld)",
              GetName(), m_AllowedCards.size()); // , GetTrades().size());

    if (bDetail)
    {
//LogWarning(L"Show detail Not implemented");
#if 1
        // Flag_t ShowFlags = 0;
        // Don't show card detail
        // ShowFlags |= ShowFlags::Detail;
        m_AllowedCards.Show(L"AllowedCards");
        if (0 < Value)
            m_TradeMaker.ShowTrades(Value);
        else
            m_TradeMaker.ShowAllTrades();
#endif
    }
}

///////////////////////////////////////////////////////////////////////////////

bool
SimplePosting_t::
ReadXmlFile(
    const wchar_t* pszFilename)
{
    XmlFile_t<SimplePosting_t> XmlFile(this, pszFilename);
    return XmlFile.Read();
}

///////////////////////////////////////////////////////////////////////////////

bool
SimplePosting_t::
WriteXmlFile(
    const wchar_t* pszFilename) const
{
    XmlFile_t<SimplePosting_t> XmlFile(this, pszFilename);
    return XmlFile.Write();
}

///////////////////////////////////////////////////////////////////////////////
//
// <Simple>
//   <AllowedCards Count="1">
//   </AllowedCards>
//   <TradeMaps Count="1">
//   </TradeMaps>
// </Simple>
//

bool
SimplePosting_t::
ReadXml(
          IXmlReader* pReader,
    const wchar_t*    ElementName)
{
    // <Simple>
    if (!ReadNextXml(pReader, XmlNodeType_Element, ElementName))
        return false;
    {
        CLock lock(m_csTrades);
        // <AllowedCards>
        if (!m_AllowedCards.ReadXml(pReader, XmlElementAllowedCards))
            return false;
        // <TradeMaps>
        if (!m_TradeMaker.ReadXml(pReader))
            return false;
    }
    // </Simple>
    return ReadNextXml(pReader, XmlNodeType_EndElement, ElementName);
}

///////////////////////////////////////////////////////////////////////////////
//
// <Simple>
//   <AllowedCards Count="1">
//   </AllowedCards>
//   <TradeMaps Count="1">
//   <TradeMaps>
// </Simple>
//

bool
SimplePosting_t::
WriteXml(
          IXmlWriter* pWriter,
    const wchar_t*    ElementName) const
{
    HRESULT hr;

    // <Simple>
    hr = pWriter->WriteStartElement(NULL, ElementName, NULL);
    if (FAILED(hr))
    {
        LogError(L"WriteStartElement() failed %08x", hr);
        return false;
    }
    {
        CLock lock(m_csTrades);
        // <AllowedCards>
        if (!m_AllowedCards.WriteXml(pWriter, XmlElementAllowedCards))
            return false;
        // <TradeMaps>
        if (!m_TradeMaker.WriteXml(pWriter))
            return false;
    }
    // </Simple>
    hr = pWriter->WriteFullEndElement();
    if (FAILED(hr))
    {
        LogError(L"WriteEndElement() failed %08x", hr);
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

} // TradePoster
