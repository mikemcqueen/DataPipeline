///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TradeMaker_t.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TradeMaker_t.h"
#include "LonTrader_t.h"
#include "Timer_t.h"
#include "LonPlayer_t.h"
#include "LonCard_t.h"
#include "XmlUtil.h"
#include "XmlNames.h"
#include "TradeMap_t.h"

const wchar_t TradeMaker_t::XmlElementName[]     = L"TradeMaps";
const wchar_t TradeMaker_t::XmlElementTradeMap[] = L"TradeMap";
const wchar_t TradeMaker_t::XmlElementTrades[]   = L"Trades";

static const size_t MaxCardsPerTrade = 50;

static TradeMap_t  EmptyTradeMap;
static TradeList_t EmptyTradeList;

///////////////////////////////////////////////////////////////////////////////

TradeMaker_t::
TradeMaker_t()
{
}

#if 0
///////////////////////////////////////////////////////////////////////////////

size_t
TradeMaker_t::
BuildBuyTrades(
    const CardQuantity_t&      CardQ,
          size_t               Value,
    const CardQuantityQueue_t& AllowedCards,
          size_t               MaxPerAllowedCard,
          size_t               BuildMax)
{
    return BuildTrades(true, CardQ, Value, AllowedCards, MaxPerAllowedCard,
                       BuildMax);
}

///////////////////////////////////////////////////////////////////////////////

size_t
TradeMaker_t::
BuildSellTrades(
    const CardQuantity_t&      CardQ,
          size_t               Value,
    const CardQuantityQueue_t& AllowedCards,
          size_t               MaxPerAllowedCard,
          size_t               BuildMax)
{
    return BuildTrades(false, CardQ, Value, AllowedCards, MaxPerAllowedCard,
                       BuildMax);
}
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CardQ              // Card, and max # of cards to use per trade
// Value
// m_AllowedCards     // Cards allowed in trade
// MaxPerAllowedCard  // Max # of trades per allowed card (always 1)
// BuildMax           // Total # of trades to generate    (always 4)
//
// This is a mess but it's functional.
// 

size_t
TradeMaker_t::
BuildTrades(
          bool                 bBuy,
    const CardQuantity_t&      CardQ,
          size_t               Value,
    const CardQuantityQueue_t& AllowedCards,
          size_t               MaxPerAllowedCard,
          size_t               BuildMax)
{
    if (0 == Value)
    {
        ASSERT(0);
        return 0;
    }

    MaxPerAllowedCard; // unused
    TradeList_t& Trades = AddTradesDestructive(Value);
    size_t CardCount = 1; // what is this?  what is CardQ.Quantity?
    size_t BuildCount = 0;
    CardQuantityQueue_t FromCards(AllowedCards);
    while ((BuildCount < BuildMax) && (CardCount <= CardQ.Quantity))
    {
        const size_t TotalValue = CardCount * Value;
        LogAlways(L"  Building trade for %d x %ls @ %d",
                  CardCount, CardQ.pCard->GetName(), TotalValue);
        CardCollection_t BuildCards;
        size_t BuildCardsValue = 
            RecursiveBuildCards(BuildCards, TotalValue, FromCards);
        if (TotalValue == BuildCardsValue)
        {
            Trade_t Trade;
            // Trade.SetId(TradeId_t(Trades.size()));             // Necessary?
            Trade.SetUser(LonTrader_t::GetPlayer().GetName()); // Could be better.
            // TODO: BuildTrade(Trade, bBuy, CardQ, CardCount, BuildCards);
            if (bBuy)
            {
                Trade.AddWantCard(CardQ.pCard, CardCount);
                Trade.GetOfferCards().Add(BuildCards);
            }
            else
            {
                Trade.AddOfferCard(CardQ.pCard, CardCount);
                Trade.GetWantCards().Add(BuildCards);
            }
            Trades.push_back(Trade);
            BuildCount++;
        }
        else
        {
            LogAlways(L"  Build failed.");
            ++CardCount;
            FromCards = AllowedCards;
        }
    }
    LogAlways(L"  Trades Built(%d) Total(%d)", BuildCount, Trades.size());
    return BuildCount;
}

///////////////////////////////////////////////////////////////////////////////
//
// RecursiveBuildCards
//
// This function attempts to add cards to the collection BuildCards, taken
// from the card queue FromCards, until the total value of the cards in
// BuildCards is equal to the supplied TotalValue.
//
// If the TotalValue is not exactly matched, BuildCards and FromCards remain
// unchanged and zero is returned.
//
// If the TotalValue is exactly matched, BuildCards contains the cards that
// add up to TotalValue, those same cards are removed from FromCards, and
// TotalValue is returned.
//

size_t
TradeMaker_t::
RecursiveBuildCards(
    CardCollection_t&    BuildCards,
    size_t               TotalValue,
    CardQuantityQueue_t& FromCards,
    size_t               Level) const
{
    if (FromCards.empty())
        return 0;
    const Card_t& Card = *FromCards.front().pCard;
    // TODO: validate all allowed cards in BuildSellTrades
    ASSERT(0 < Card.GetValue());
    if (0 == Card.GetValue())
        return 0;

    size_t MaxCards = FromCards.front().Quantity;
    ASSERT(0 < MaxCards);
    MaxCards = min(MaxCards, TotalValue / Card.GetValue());
    MaxCards = min(MaxCards, MaxCardsPerTrade - BuildCards.GetTotalQuantity());
    CardQuantityQueue_t FromCardsCopy(FromCards);
    FromCardsCopy.pop_front();
    for (size_t CardCount = MaxCards; 1 <= CardCount; --CardCount)
    {
        ASSERT(CardCount * Card.GetValue() <= TotalValue);
        size_t RemainingValue = TotalValue - CardCount * Card.GetValue();
        CardCollection_t BuildCardsCopy(BuildCards);
        BuildCardsCopy.Add(CardQuantity_t(&Card, CardCount));

        LogAlways(L"  %*lsAdding %d x %ls @ %d, Remaining (%d)",
                  Level * 2, L" ", CardCount, Card.GetName(),
                  Card.GetValue(), RemainingValue);

        if (0 < RemainingValue)
        {
            const size_t Value =
                RecursiveBuildCards(
                    BuildCardsCopy, 
                    RemainingValue,
                    FromCardsCopy,
                    Level + 1);
            ASSERT(Value <= RemainingValue);
            RemainingValue -= Value;
        }
        if (0 == RemainingValue)
        {
            BuildCards = BuildCardsCopy;
            FromCards = FromCardsCopy;
            return TotalValue;
        }
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

#if 0
size_t
TradeMaker_t::
RemoveTradesWithCard(
    const LonCard_t* pCard)
{
    ASSERT(NULL != pCard);
    size_t Count = 0;
    TradeMap_t::iterator it = m_Trades.begin();
    while (m_Trades.end() != it)
    {
        const Trade_t& Trade = it->second;
        if (Trade.IsOffered(pCard) || Trade.IsWanted(pCard))
        {
            it = m_Trades.erase(it);
            ++Count;
        }
        else
            ++it;
    }
    LogAlways(L"RemoveTradesWithCard(): Removed (%d) Remaining (%d)",
              Count, m_Trades.size());
    return Count;
}
#endif

///////////////////////////////////////////////////////////////////////////////

TradeList_t&
TradeMaker_t::
GetTrades(
    size_t Value)
{
    // It could be argued that we should only return a (const) EmptyTradeMap
    // from the (const) method.  That any non-cost accessing of a trademap
    // needs to call HasTrades first?  Kinda arbitrary, but prevents us
    // accidentally horking EmptyTrades, and EmptyTrades has a real practical
    // value from code clarity/conciseness pov.
    return GetTradeMapOrEmptyMap(Value);
    // return (HasTradeMap(Value)) ? m_Map.find(Value)->second : EmptyTradeMap;
}

///////////////////////////////////////////////////////////////////////////////

const TradeList_t& 
TradeMaker_t::
GetTrades(
    size_t Value) const
{
    return GetTradeMapOrEmptyMap(Value);
    // return (HasTradeMap(Value)) ? m_Map.find(Value)->second : EmptyTradeMap;
}

///////////////////////////////////////////////////////////////////////////////

TradeList_t&
TradeMaker_t::
GetTradeMapOrEmptyMap(
    size_t Value) const
{
    Map_t::iterator it = const_cast<Map_t&>(m_Map).find(Value);
    return (m_Map.end() != it) ? it->second : EmptyTradeList;
}

///////////////////////////////////////////////////////////////////////////////

bool
TradeMaker_t::
HasTrades(
    size_t Value) const
{
    return m_Map.end() != m_Map.find(Value);
}

///////////////////////////////////////////////////////////////////////////////

Trade_t*
TradeMaker_t::
FindTrade(
    TradeId_t TradeId)
{
    Map_t::iterator it = m_Map.begin();
    for (; m_Map.end() != it; ++it)
    {
        TradeList_t& Trades = it->second;
        TradeList_t::iterator itTrade = Trades.Find(TradeId);
        if (Trades.end() != itTrade)
            return &*itTrade;
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

TradeList_t&
TradeMaker_t::
AddTradesDestructive(
    size_t Value)
{
    Map_t::iterator it = m_Map.find(Value);
    if (m_Map.end() != it)
    {
        LogWarning(L"TradeMaker::AddTradeMapDestructive(%d) Erasing old TradeMap (size=%d)",
                   Value, it->second.size());
        m_Map.erase(it);
    }
    std::pair<Map_t::iterator, bool> Result = 
        m_Map.insert(std::pair<size_t, TradeList_t>(Value, EmptyTradeList));
    if (!Result.second)
        throw std::logic_error("TradeMaker::AddTradesDestructive() insert failed");
    return Result.first->second;
}

///////////////////////////////////////////////////////////////////////////////

void
TradeMaker_t::
ShowAllTrades() const
{
    Map_t::const_iterator it = m_Map.begin();
    for (; m_Map.end() != it; ++it)
    {
        ShowTrades(it);
    }
}

///////////////////////////////////////////////////////////////////////////////

void
TradeMaker_t::
ShowTrades(
    Map_t::const_iterator& itMap) const
{
    wchar_t szTrades[64];
    _snwprintf_s(szTrades, _TRUNCATE, L"Trades(%d)", itMap->first);
    itMap->second.Show(szTrades);
}

///////////////////////////////////////////////////////////////////////////////

void
TradeMaker_t::
ShowTrades(
    size_t Value) const
{
    Map_t::const_iterator it = m_Map.find(Value);
    if (m_Map.end() == it)
    {
        LogWarning(L"ShowTrades(%d) No trades found", Value);
        return;
    }
    ShowTrades(it);
}

///////////////////////////////////////////////////////////////////////////////

void
TradeMaker_t::
EraseAll()
{
    LogAlways(L"EraseAll() Size(%d)", m_Map.size());
    m_Map.clear();
}

///////////////////////////////////////////////////////////////////////////////
//
// <TradeMaps Count="2">
//   <TradeMap Value="1000">
//     <Trades Count="1">
//       <Trade>
//       </Trade>
//     </Trades>
//   </TradeMap>
//   <TradeMap Value=2000">
//   </TradeMap>
// <TradeMaps>
//

bool
TradeMaker_t::
ReadXml(
          IXmlReader* pReader,
    const wchar_t*    ElementName)
{
    HRESULT hr;

    // <TradeMaps>
    if (!ReadNextXml(pReader, XmlNodeType_Element, ElementName))
        return false;
    // Attribute: Count="N"
    hr = pReader->MoveToAttributeByName(XmlNames::AttributeCount, NULL);
    if (S_OK != hr)
    {
        LogError(L"ReadXml: MoveToAttributeByName(Count) failed (%08x)", hr);
        return false;
    }
    const wchar_t* pszCount = NULL;
    hr = pReader->GetValue(&pszCount, NULL);
    if ((S_OK != hr) || (NULL == pszCount))
    {
        LogError(L"ReadXml: GetValue(Count) failed (%08x)", hr);
        return false;
    }
    size_t Count = _wtoi(pszCount);

    hr = pReader->MoveToElement();
    if (S_OK != hr)
    {
        LogError(L"ReadXml: MoveToElement() failed (%08x)", hr);
        return false;
    }
    while (0 < Count--)
    {
        // <TradeMap>
        if (!ReadNextXml(pReader, XmlNodeType_Element, XmlElementTradeMap))
            return false;

        // Attribute: Value="N"
        hr = pReader->MoveToAttributeByName(XmlNames::AttributeValue, NULL);
        if (S_OK != hr)
        {
            LogError(L"ReadXml: MoveToAttributeByName(Value) failed (%08x)", hr);
            return false;
        }
        const wchar_t* pszValue;
        hr = pReader->GetValue(&pszValue, NULL);
        if ((S_OK != hr) || (NULL == pszValue))
        {
            LogError(L"ReadXml: GetValue(Value) failed (%08x)", hr);
            return false;
        }
        size_t Value = _wtoi(pszValue);
        hr = pReader->MoveToElement();
        if (S_OK != hr)
        {
            LogError(L"ReadXml: MoveToElement() failed (%08x)", hr);
            return false;
        }
        TradeList_t& Trades = AddTradesDestructive(Value);

        // <Trades>
        if (!Trades.ReadXml(pReader, XmlElementTrades))
            return false;

        // </TradeMap>
        if (!ReadNextXml(pReader, XmlNodeType_EndElement, XmlElementTradeMap))
            return false;
    }

    // </TradeMaps>
    return ReadNextXml(pReader, XmlNodeType_EndElement, ElementName);
}

///////////////////////////////////////////////////////////////////////////////
//
// <TradeMaps Count="2">
//   <TradeMap Value="1000">
//     <Trades Count="1">
//       <Trade>
//       </Trade>
//     </Trades>
//   </TradeMap>
//   <TradeMap Value=2000">
//   </TradeMap>
// <TradeMaps>
//

bool
TradeMaker_t::
WriteXml(
          IXmlWriter* pWriter,
    const wchar_t*    ElementName) const
{
    HRESULT hr;
    // <TradeMaps>
    hr = pWriter->WriteStartElement(NULL, ElementName, NULL);
    if (FAILED(hr))
    {
        LogError(L"WriteStartElement() failed %08.8lx", hr);
        return false;
    }

    // Attribute: Count="N"
    wchar_t szCount[32];
    _snwprintf_s(szCount, _TRUNCATE, L"%d", m_Map.size());
    hr = pWriter->WriteAttributeString(NULL, XmlNames::AttributeCount, NULL, szCount);
    if (FAILED(hr))
    {
        LogError(L"WriteAttributeString() failed %08.8lx", hr);
        return false;
    }

    for (Map_t::const_iterator it = m_Map.begin(); m_Map.end() != it; ++it)
    {
        // <TradeMap>
        hr = pWriter->WriteStartElement(NULL, XmlElementTradeMap, NULL);
        if (FAILED(hr))
        {
            LogError(L"WriteStartElement(TradeMap) failed %08.8lx", hr);
            return false;
        }

        // Attribute: Count="N"
        wchar_t szValue[32];
        _snwprintf_s(szValue, _TRUNCATE, L"%d", it->first);
        hr = pWriter->WriteAttributeString(NULL, XmlNames::AttributeValue, NULL, szValue);
        if (FAILED(hr))
        {
            LogError(L"WriteAttributeString(Value) failed %08.8lx", hr);
            return false;
        }

        // <Trades>
        it->second.WriteXml(pWriter, XmlElementTrades);

        // </TradeMap>
        pWriter->WriteFullEndElement();
        if (FAILED(hr))
        {
            LogError(L"WriteEndElement() failed %08.8lx", hr);
            return false;
        }
    }

    // </TradeMaps>
    hr = pWriter->WriteFullEndElement();
    if (FAILED(hr))
    {
        LogError(L"WriteEndElement() failed %08.8lx", hr);
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
