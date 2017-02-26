///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// LonCardSet_t.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LonCardSet_t.h"
#include "DbCards_t.h"
#include "Services.h"
#include "TradeManager_t.h"
#include "LonTrader_t.h"
#include "Timer_t.h"
#include "Log.h"
#include "CardCollection_t.h"
#include "Trade_t.h"

#include "boost/bind.hpp"

///////////////////////////////////////////////////////////////////////////////

LonCardSet_t::
LonCardSet_t() :
    m_Cards(L"Main Deck"),
    m_NewCards(L"New Cards")
{
}

///////////////////////////////////////////////////////////////////////////////

LonCardSet_t::
~LonCardSet_t()
{
}

///////////////////////////////////////////////////////////////////////////////

void
LonCardSet_t::
ShowCards(
    bool   bValuedOnly,
    bool   bDetail,
    Flag_t Flags,
    size_t Order) const
{
   CLock lock(m_csCards);
   ShowCards(m_Cards, bValuedOnly, bDetail, Flags, Order);
}

///////////////////////////////////////////////////////////////////////////////

void
LonCardSet_t::
ShowCards(
    const LonCardMap_t& Cards,
          bool          bValuedOnly,
          bool          bDetail,
          Flag_t        Flags,
          size_t        Order) const
{
    int common   = 0;
    int uncommon = 0;
    int rare     = 0;
    int foil     = 0;
    int loot     = 0;
    int fixed    = 0;
    int promo    = 0;
    int m        = 0;

    Flag_t ShowFlags = 0;
    std::vector<const LonCard_t*> SortedCards;
    LonCardMap_t::const_iterator it = Cards.begin();
    for(; Cards.end() != it; ++it)
        SortedCards.push_back(&it->second);

    if (Flag::SortByHighBid == Flags)
    {
        struct SortByHighBid
        {
            bool operator()(const LonCard_t* lhs, const LonCard_t* rhs)
            {
                return lhs->GetHighBid() < rhs->GetHighBid();
            }
        };
        std::sort(SortedCards.begin(), SortedCards.end(), SortByHighBid());
        ShowFlags = LonCard_t::Flag::ShowValues;
    }
    else if (Flag::SortByLowAsk == Flags)
    {
        struct SortByLowAsk
        {
            bool operator()(const LonCard_t* lhs, const LonCard_t* rhs)
            {
                return lhs->GetLowAsk() < rhs->GetLowAsk();
            }
        };
        std::sort(SortedCards.begin(), SortedCards.end(), SortByLowAsk());
        ShowFlags = LonCard_t::Flag::ShowValues;
    }
    if (bDetail)
        ShowFlags |= LonCard_t::Flag::ShowDetail;

    std::vector<const LonCard_t*>::const_iterator
        itSorted = SortedCards.begin();
    for(; SortedCards.end() != itSorted; ++itSorted)
    {
        const LonCard_t& card = **itSorted;

        if (bValuedOnly && (0 == card.GetHighBid()))
            continue;

        LonCard_t::Number_t cn = card.GetNumber();
        if (cn.foil)
        {
            ++foil;
        }
        else
        {
            switch (cn.type)
            {
            case L'C':           ++common;   break;
            case L'U':           ++uncommon; break;
            case L'R':           ++rare;     break;
            case L'F':           ++fixed;    break;

            case L'M':           ++m;        break;
            case L'P':           ++promo;    break;
            case CardType::Loot: ++loot;     break;
            default:             ASSERT(0);  break;
            }
        }
        card.ShowValue(ShowFlags, Order);
    }
    if (bDetail)
    {
        LogAlways(L"Total: %d; Common: %d, Uncommon: %d, Rare: %d, "
                  L"Foil: %d, Fixed: %d, Promo: %d, Raid: %d, Loot: %d",
                  Cards.size(), common, uncommon, rare,
                  foil, fixed, promo, m, loot);
    }
}

///////////////////////////////////////////////////////////////////////////////

void
LonCardSet_t::
ShowTransactions(
    Flag_t Flags) const
{
    struct Data_t
    {
        const LonCard_t* pCard;
        size_t           Income;

        explicit
        Data_t(
            const LonCard_t* Card)
        : 
            pCard(Card),
            Income(0)
        { }
    };

    CLock lock(m_csCards);
    std::vector<Data_t> SortedCards;
    LonCardMap_t::const_iterator it = m_Cards.begin();
    for(; m_Cards.end() != it; ++it)
    {
        const LonCard_t& Card = it->second;
        if (Card.IsLootCard())
            SortedCards.push_back(Data_t(&it->second));
    }

    struct CalculateIncome
    {
        void operator()(Data_t& Data)
        {
            const LonCard_t* pCard = Data.pCard;
            // Must have sold.
            if (0 == Data.pCard->m_Sold.size())
                return;
            size_t BuyPrice = pCard->GetHighBid();
            if (0 == BuyPrice)
            {
                LogWarning(L"Buy price zero for %ls", pCard->GetName());
                return;
            }
            size_t TotalSold  = pCard->m_Sold.TotalQuantity();
            Data.Income = TotalSold * (pCard->GetLowAsk() - BuyPrice);
        }
    };
    std::for_each(SortedCards.begin(), SortedCards.end(), CalculateIncome());

    if (0 != (Flag::SortByBoughtCount & Flags))
    {
        struct SortByBoughtCount
        {
            bool operator()(const Data_t& lhs, const Data_t& rhs)
            {
                return lhs.pCard->m_Bought.TotalQuantity() < rhs.pCard->m_Bought.TotalQuantity();
            }
        };
        std::sort(SortedCards.begin(), SortedCards.end(), SortByBoughtCount());
    }
    else if (0 != (Flag::SortBySoldCount & Flags))
    {
        struct SortBySoldCount
        {
            bool operator()(const Data_t& lhs, const Data_t& rhs)
            {
                return lhs.pCard->m_Sold.TotalQuantity() < rhs.pCard->m_Sold.TotalQuantity();
            }
        };
        std::sort(SortedCards.begin(), SortedCards.end(), SortBySoldCount());
    }
    else
    {
        struct SortByTotalIncome
        {
            bool operator()(const Data_t& lhs, const Data_t& rhs)
            {
                return lhs.Income < rhs.Income;
            }
        };
        std::sort(SortedCards.begin(), SortedCards.end(), SortByTotalIncome());
    }

    Flag_t ShowFlags = LonCard_t::Flag::ShowTransactions;
    if (0 != (Flag::ShowDetail & Flags))
        ShowFlags |= LonCard_t::Flag::ShowDetail;
    std::vector<Data_t>::const_iterator itSorted(SortedCards.begin());
    for(; SortedCards.end() != itSorted; ++itSorted)
    {
        const LonCard_t& Card = *itSorted->pCard;
        if (0 == Card.GetTransactionCount())
            continue;
        Card.ShowTransactions(ShowFlags, itSorted->Income);
    }
}

///////////////////////////////////////////////////////////////////////////////

/*
bool
LonCardSet_t::
DbWriteCards()
{
    CDatabase db;
    try
    {
        db.Open(NULL, FALSE, FALSE, GetDbConnectString());
        DbCards_t rs(&db);
        rs.Open(CRecordset::dynaset, NULL, CRecordset::none);
        LonCardMap_t::iterator it(m_Cards.begin());
        while (!rs.IsEOF())
        {
            const Card_t& card = it->second;
            ASSERT(card.GetId() == (DWORD)rs.m_cardid);

            rs.Edit();
            rs.m_value = (long)card.GetValue();
            rs.Update();
            rs.MoveNext();
            ++it;
        }
        rs.Close();
        db.Close();
        return true;
    }
    catch(CDBException *e)
    {
        LogError(L"DbWriteCards() exception: %ls", e->m_strError);
    }
    return false;
}
*/

///////////////////////////////////////////////////////////////////////////////

bool
LonCardSet_t::
ReadCards(
    const wchar_t* pszFile)
{
pszFile;
    using namespace std;

    CLock lock(m_csCards);
    CDatabase db;
    try
    {
        db.Open(NULL, FALSE, FALSE, LonTrader_t::GetDbConnectString(), FALSE);
        CRecordset rs(&db);

        wchar_t szSql[] = L"SELECT cardid, cardname, value FROM cards_t";
        rs.Open(CRecordset::forwardOnly, szSql, CRecordset::readOnly);
        while (!rs.IsEOF())
        {
            CDBVariant varId;
            CDBVariant varValue;
            CString strName;
            rs.GetFieldValue(L"cardid", varId);
            rs.GetFieldValue(L"cardname", strName);
            rs.GetFieldValue(L"value", varValue);
            AddCard(LonCard_t(varId.m_lVal, strName, varValue.m_lVal));
            rs.MoveNext();
        }
        rs.Close();

        wchar_t szFixups[] = L"SELECT keywords, cardname FROM namefixup_t";
        rs.Open(CRecordset::forwardOnly, szFixups, CRecordset::readOnly);
        while (!rs.IsEOF())
        {
            CString strKeywords;
            CString strName;
            rs.GetFieldValue(L"keywords", strKeywords);
            rs.GetFieldValue(L"cardname", strName);
            NameFixup_t fixup(strKeywords, strName);
            m_NameFixups.push_back(fixup);
            rs.MoveNext();
        }
        rs.Close();
//ShowCards();
    }
    catch(CDBException* pdbe)
    {
        LogError(L"ReadCards () exception: %ls", pdbe->m_strError);
        return false;
    }
	// HACK: but good for performance.
	BuildIdCardMap();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void
LonCardSet_t::
AddCard(
    const LonCard_t& Card)
{
    m_Cards.insert(/*m_Cards.begin(), */LonCard_t::Pair_t(Card.GetName(), Card));
}

///////////////////////////////////////////////////////////////////////////////

void
LonCardSet_t::
BuildIdCardMap()
{
    CLock lock(m_csCards);
    LonCardMap_t::const_iterator it = m_Cards.begin();
    for (; m_Cards.end() != it; ++it)
    {
		const LonCard_t& Card = it->second;
		m_CardsById.insert(IdCardMap_t::Pair_t(Card.GetId(), &Card));
	}
}

///////////////////////////////////////////////////////////////////////////////

void
LonCardSet_t::
Fixup(
    std::wstring& strName) const
{
    NameFixupVector_t::const_iterator it = m_NameFixups.begin();
    for (; it != m_NameFixups.end(); ++it)
    {
        std::wstring str(it->strKeywords);
        size_t begin = 0;
        size_t end;
        while (str.npos != begin)
        {
            end = str.find(L' ', begin);
            if (str.npos != end)
                str[end++] = L'\0';
            if (str.npos == strName.find(&str[begin]))
                break;
            begin = end;
        }
        if (str.npos == begin)
        {
            strName.assign(it->strName);
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

const Card_t*
LonCardSet_t::
Find(
    const wchar_t* pName) const
{
    ASSERT(NULL != pName);
    CLock lock(m_csCards);
    LonCardMap_t::const_iterator it = m_Cards.find(pName);
    if (m_Cards.end() != it)
        return &it->second;
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//
// this function is a retarded mess
// For lookup of card given UI name, possibly needs cleanup.
//
const Card_t*
LonCardSet_t::
Lookup(
    const wchar_t* pszText,
    bool           bFullName /*= false*/) const
{
    if (NULL == pszText || L'\0' == pszText[0])
        return NULL;

    // Pass 1.
    bool bTruncated = false;
    std::wstring strName(pszText);
    const size_t Length = strName.length();
    if (LonCard_t::UiNameCleanup(strName))
    {
        if (strName.length() != Length)
        {
            const Card_t* pCard  = LookupPartialAtBeginning(strName.c_str(), true);
            if (NULL != pCard)
                return pCard;
            bTruncated = true;
        }
        else
        {
            const Card_t* pCard = Find(strName.c_str());
            if (NULL != pCard)
                return pCard;
        }
    }

    // Pass 1 failed.
    // Remove everything after the last comma, if any.
    bool bComma = false;
    size_t Pos = strName.find_last_of(L',');
    if (strName.npos != Pos)
    {
        bComma = true;
        strName.erase(Pos);
    }

    // Pass2.
    if (!bTruncated || bComma)
    {
        const Card_t* pCard  = LookupPartialAtBeginning(strName.c_str(), true);
        if (NULL != pCard)
            return pCard;
    }

    if (bFullName)
    {
        // Add it to new-card collection, and return a pointer to "unknown card".
        AddNewCard(pszText);
        strName.assign(L"UNKNOWN_CARD");
        const Card_t* pCard = Find(strName.c_str());
        if (NULL != pCard)
            return pCard;
        ASSERT(false);
    }
    LogError(L"LonCardSet_t::Lookup() failed: '%s'", strName.c_str());
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

const Card_t*
LonCardSet_t::
LookupPartialAtBeginning(
    LPCWSTR pszText,
    bool    bQuiet /*=false*/) const
{
    if (NULL == pszText || L'\0' == pszText[0])
        return NULL;

    CLock lock(m_csCards);
    LonCardMap_t::const_iterator it = m_Cards.begin();
    for (; m_Cards.end() != it; ++it)
    {
        // this says "look for the supplied text at the beginning of the string"
        if (0 == it->first.find(pszText))
            return &it->second;
    }
    if (!bQuiet)
        LogError(L"LookupPartialAtBeginning() failed: '%ls'", pszText);
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

const Card_t*
LonCardSet_t::
LookupPartialAtAnyPos(
    const wchar_t* pszText) const
{
    if (NULL == pszText || L'\0' == pszText[0])
        return NULL;

    CLock lock(m_csCards);
    LonCardMap_t::const_iterator it = m_Cards.begin();
    for (; m_Cards.end() != it; ++it)
    {
        // this says "look for the supplied text at any position in the string"
        if (it->first.npos != it->first.find(pszText))
            return &it->second;
    }
    LogError(L"LookupPartialAtAnyPos() failed: '%s'", pszText);
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

const LonCard_t*
LonCardSet_t::
Lookup(
    CardId_t Id) const
{
    CLock lock(m_csCards);
    IdCardMap_t::const_iterator it = m_CardsById.find(Id);
    if (m_CardsById.end() != it)
        return it->second;
    LogError(L"Card ID lookup failed: %d", Id);
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

const Card_t*
LonCardSet_t::
Lookup(
    LonCard_t::Number_t Number) const
{
    CLock lock(m_csCards);
    LonCardMap_t::const_iterator it = m_Cards.begin();
    for (; m_Cards.end() != it; ++it)
    {
        const LonCard_t& card = it->second;
        if (card.GetNumber() == Number)
            return &card;
    }
//    LogError(L"Card ID lookup failed: %d", id);
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////

void
LonCardSet_t::
ValueCards(
    const Flag_t Flag)
{
    Timer_t Timer(L"ValueCards()");

    // Clear all values.
/*
    {
        LonCardMap_t::iterator it(m_Cards.begin());
        for (; m_Cards.end() != it; ++it)
        {
            LonCard_t* pCard = &it->second;
            pCard->SetValue(0);
        }
    }
*/
    // First, value all cards on Boosters.
    const Card_t* pBooster;
    pBooster = LookupPartialAtAnyPos(L"Oathbound Booster");
    if (NULL == pBooster)
        return;
    const_cast<Card_t*>(pBooster)->SetValue(1000);
    ValueCards(Flag, pBooster);

    pBooster = LookupPartialAtAnyPos(L"Forsworn Booster");
    if (NULL == pBooster)
        return;
    const_cast<Card_t*>(pBooster)->SetValue(1000);
    ValueCards(Flag, pBooster);

    pBooster = LookupPartialAtAnyPos(L"Inquisitor Booster");
    if (NULL == pBooster)
        return;
    const_cast<Card_t*>(pBooster)->SetValue(1000);
    ValueCards(Flag, pBooster);

    pBooster = LookupPartialAtAnyPos(L"Oathbreaker Booster");
    if (NULL == pBooster)
        return;
    const_cast<Card_t*>(pBooster)->SetValue(1000);
    ValueCards(Flag, pBooster);

    pBooster = LookupPartialAtAnyPos(L"Event Pass");
    if (NULL == pBooster)
        return;
    const_cast<Card_t*>(pBooster)->SetValue(400);
    ValueCards(Flag, pBooster);

    ValueCardsLoop(Flag);
}

///////////////////////////////////////////////////////////////////////////////
//
// Flag = 0 means value all cards.
//

size_t
LonCardSet_t::
ValueCards(
    const Flag_t  Flag,
    const Card_t* pFindCard)
{
    CardCollection_t Collection;
    CardQuantity_t Card(pFindCard);
    Collection.insert(Card);
    return ValueCards(Flag, Collection, 1, 1);
}

///////////////////////////////////////////////////////////////////////////////

void
LonCardSet_t::
ValueCardsLoop(
    const Flag_t Flag)
{
    size_t Order = 1;
    size_t Count;
    size_t Max = 20;
    do
    {
        // TODO: optimization: add to Coll as we go along , instead of
        // re-constructing it each time.  And do one pass through it
        // at first to confirm all cards still have bids ( could that
        // ever not be true?
        Count = 0;
        CardCollection_t Coll;
        LonCardMap_t::iterator it = m_Cards.begin();
        for (; m_Cards.end() != it; ++it)
        {
            const LonCard_t& Card = it->second;
            if (0 < Card.GetHighBid())
            {
                CardQuantity_t quant(&Card);
                Coll.insert(quant);
            }
        }
        if (!Coll.empty())
            Count = ValueCards(Flag, Coll, Coll.size(), ++Order);

        if (0 == --Max)
        {
            LogWarning(L"  Recursive valuation dectected, aborting...");
            break;
        }
    } while (0 < Count);
}

///////////////////////////////////////////////////////////////////////////////

size_t
LonCardSet_t::
ValueCards(
    const Flag_t Flag,
    const CardCollection_t& Collection,
          size_t            CardCount,
          size_t            Order)
{
    if (1 == Collection.size())
    {
        LogInfo(L"ValueCards(%d): Begin '%ls'",
                  Order, Collection.begin()->pCard->GetName());
    }
    else
    {
        LogInfo(L"ValueCards(%d): Begin Collection (%d)",
                  Order, Collection.size());
    }

    size_t BidsChanged = 0;
    size_t AsksChanged = 0;
//    Timer_t Timer(L"  ValueCards");
    LonCardMap_t::iterator it = m_Cards.begin();
    for (; m_Cards.end() != it; ++it)
    {
        LonCard_t& Card = (LonCard_t&)it->second;

        if ((0 != Flag) && !Card.CheckFlag(Flag))
            continue;

        // Big optimization win: don't value non-loot cards.
        if (!Card.IsLootCard())
            continue;

        // Don't price cards with innate value.
        if (0 < Card.GetValue()) //Card.IsBoosterPack())
            continue;

        // TODO: Fix this blight. Use Card_t::DummyCard?
        if (0 == wcscmp(Card.GetName(), L"UNKNOWN_CARD"))
            continue;

        size_t HighBid = Card.GetHighBid();
        size_t LowAsk = Card.GetLowAsk();
        size_t TradeCount = Services::GetTradeManager().FindTrades(Card, Collection, CardCount, Order);
        if (0 < TradeCount)
        {
            if (Card.GetHighBid() != HighBid)
                ++BidsChanged;
            if (Card.GetLowAsk() != LowAsk)
                ++AsksChanged;
        }
    }
//    ShowCards(true, false, true);
    size_t TotalChanged = BidsChanged + AsksChanged;
    LogInfo(L"ValueCards(%d): End Changed=%d (Bids=%d, Asks=%d)",
            Order, TotalChanged, BidsChanged, AsksChanged);
    return TotalChanged;
}

///////////////////////////////////////////////////////////////////////////////

void
LonCardSet_t::
OnRemoveTrade(
    const TradeId_t TradeId)
{
    TradeIdSet_t TradeIds;
    TradeIds.insert(TradeId);
    OnRemoveTrades(TradeIds);
}

///////////////////////////////////////////////////////////////////////////////

void
LonCardSet_t::
OnRemoveTrades(
    TradeIdSet_t TradeIds)
{
    Timer_t Timer(L"OnRemoveTrades()");
    TradeIds.Show();
    CLock lock(m_csCards);
    while (!TradeIds.empty())
    {
        CardCollection_t NoBidCards;
        CardCollection_t NoAskCards;
        RemoveDependentTradeIds(TradeIds, NoBidCards, NoAskCards);
        TradeIds.clear();
        if (!NoBidCards.empty())
        {
            Services::GetTradeManager().
                GetTradesWithCards(NoBidCards, true, TradeIds);
        }
        if (!NoAskCards.empty())
        {
            Services::GetTradeManager().
                GetTradesWithCards(NoAskCards, false, TradeIds);
        }
    }

    // reprice all cards flagged for repricing
    // TODO: return count: if (!Value..) return;
    LogInfo(L"  Repricing flagged cards");
    ValueCards(LonCard_t::Flag::Reprice);

    //    ValueFlaggedCards

    // reprice all cards based on collection
    LogInfo(L"  Repricing all cards");
    ValueCardsLoop(0);

    // clear reprice flag
    // TODO: LonCardMap_t::ClearFlag()?
    LonCardMap_t::iterator it = m_Cards.begin();
    for (; m_Cards.end() != it; ++it)
        it->second.ClearFlag(LonCard_t::Flag::Reprice);
}

///////////////////////////////////////////////////////////////////////////////
//
// Remove the specified TradeIds from the DependentTradeIds lists for 
// all cards.
//
// walk card list
//     find/remove tradeids in bid/ask dependent tradeid lists
//         flag card for repricing
//

void
LonCardSet_t::
RemoveDependentTradeIds(
    const TradeIdSet_t&     TradeIds,
          CardCollection_t& NoBidCards,
          CardCollection_t& NoAskCards)
{
    size_t Count = 0;
    for (LonCardMap_t::iterator it = m_Cards.begin(); m_Cards.end() != it; ++it)
    {
        LonCard_t& Card = it->second;
        TradeIdSet_t::const_iterator itTradeId = TradeIds.begin();
        for (; TradeIds.end() != itTradeId; ++itTradeId)
        {
            bool bReprice = false;
            if (Card.RemoveDependentTradeId(true, *itTradeId))
            {
                LogWarning(L"Removed Bids (%d) '%ls'", *itTradeId, Card.GetName());
                if (Card.GetBids().TradeValues.empty())
                {
                    LogWarning(L"  Adding to NoBidCards");
                    CardQuantity_t Quant(&Card);
                    NoBidCards.insert(Quant);
                }
                bReprice = true;
            }
            if (Card.RemoveDependentTradeId(false, *itTradeId))
            {
                LogWarning(L"Removed Asks (%d) '%ls'", *itTradeId, Card.GetName());
                if (Card.GetAsks().TradeValues.empty())
                {
                    LogWarning(L"  Adding to NoAskCards");
                    CardQuantity_t Quant(&Card);
                    NoAskCards.insert(Quant);
                }
                bReprice = true;
            }
            if (bReprice)
            {
                Card.SetFlag(LonCard_t::Flag::Reprice);
                ++Count;
            }
        }
    }
    LogWarning(L"%d cards flagged for repricing.", Count);
}

///////////////////////////////////////////////////////////////////////////////

void
LonCardSet_t::
SetCardPointers(
    CardCollection_t& Cards)
{
    CardCollection_t::iterator it = Cards.begin();
    for (; Cards.end() != it; ++it)
    {
        const Card_t* pCard = Lookup(it->CardId);
        ASSERT(NULL != pCard);
		// NOTE set elements are apparently immutable now, so just commenting out all mutations to get it comipiling
		//        it->pCard = pCard;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
// Called after load from DB.  Takes TradeManager_t::m_Trades as  parameter.
// Initializes the "m_Bought" and "m_Sold" sets for each card.
//
// TODO: Get rid of const 
//

void
LonCardSet_t::
InitTransactions(
    const TradeMap_t& Trades)
{
    size_t Count = 0;
    TradeMap_t::const_iterator it = Trades.begin();
    for (; Trades.end() != it; ++it)
    {
        const Trade_t& Trade = it->second;
        if (0 == (Trade.GetFlags() & Trade_t::Flag::Removed))
            continue;

        const CardCollection_t& Offered = Trade.GetOfferCards();
        size_t OfferValue = 0;
        if (1 == Offered.size())
            OfferValue = Trade.GetWantValue();
        CardCollection_t::const_iterator itOffered = Offered.begin();
        for (; Offered.end() != itOffered; ++itOffered)
        {
            LonCard_t& Card = (LonCard_t&)*itOffered->pCard;
            // TradeValueSet_t::Pair_t pr =
            Card.m_Sold.insert(
                TradeValue_t(Trade.GetId(), OfferValue, itOffered->Quantity));
        }

        const CardCollection_t& Want = Trade.GetWantCards();
        size_t WantValue = 0;
        if (1 == Want.size())
            WantValue = Trade.GetOfferValue();
        CardCollection_t::const_iterator itWant = Want.begin();
        for (; Want.end() != itWant; ++itWant)
        {
            LonCard_t& Card = (LonCard_t&)*itWant->pCard;
            // TradeValueSet_t::Pair_t pr =
            Card.m_Bought.insert(
                TradeValue_t(Trade.GetId(), WantValue, itWant->Quantity));
        }
    }
    LogAlways(L"InitTransactions (%d)", Count);
    //TODO: return Count;
}

///////////////////////////////////////////////////////////////////////////////

void
LonCardSet_t::
GetCardValues(
    CardValueSet_t& Cvs) const
{
    CLock lock(m_csCards);
    CardValueSet_t::iterator itCvs = Cvs.begin();
    for (; Cvs.end() != itCvs; ++itCvs)
    {
        LonCardMap_t::const_iterator
            itCards = m_Cards.find(itCvs->pCard->GetName());
        ASSERT(m_Cards.end() != itCards);
        if (m_Cards.end() == itCards)
            continue;
        const LonCard_t& Card = itCards->second;
        //for each ask trade 
        //  if only boosters
        //      if lowask =0, low ask = value, else low ask = min (low ask, value)
        size_t LowAsk = 0;
        const TradeValueSet_t& Asks = Card.GetAsks().TradeValues;
        TradeValueSet_t::const_iterator itAsks = Asks.begin();
        for (; Asks.end() != itAsks; ++itAsks)
        {
            Trade_t Trade;
            if (!Services::GetTradeManager().GetTrade(itAsks->TradeId, Trade))
            {
                ASSERT(false);
                continue;
            }
            const CardCollection_t& WantCards = Trade.GetWantCards();
            CardCollection_t::const_iterator
                itNonBooster = std::find_if(
                    WantCards.begin(),
                    WantCards.end(), 
                    !boost::bind(CardQuantity_t::IsBooster(), _1));
            if (WantCards.end() != itNonBooster) // i.e. "if want cards contains a booster"
                continue;
            if ((0 == LowAsk) || (itAsks->Value < LowAsk))
            {
                const CardCollection_t& OfferCards = Trade.GetOfferCards();
                LowAsk = itAsks->Value;
                if (1000 * WantCards.GetTotalQuantity() !=
                    LowAsk * OfferCards.GetTotalQuantity())
                {
                    LogError(L"Trade(%d): LowAsk (%d) x OfferQuantity (%d) = %d",
                        Trade.GetId(), LowAsk, OfferCards.GetTotalQuantity(),
                        1000 * WantCards.GetTotalQuantity());
                }
            }
        }
        //for each bid trade
        //  if only boosters
        //       hibid = max(hibid,value)
        size_t HighBid = 0;
        const TradeValueSet_t& Bids = Card.GetBids().TradeValues;
        TradeValueSet_t::const_iterator itBids = Bids.begin();
        for (; Bids.end() != itBids; ++itBids)
        {
            Trade_t Trade;
            if (!Services::GetTradeManager().GetTrade(itBids->TradeId, Trade))
            {
                ASSERT(false);
                continue;
            }
            const CardCollection_t& OfferCards = Trade.GetOfferCards();
            const CardCollection_t::const_iterator
                itNonBooster = std::find_if(
                    OfferCards.begin(),
                    OfferCards.end(), 
                    !boost::bind(CardQuantity_t::IsBooster(), _1));
            if (OfferCards.end() != itNonBooster) // i.e. "if offer cards contains a booster"
                continue;
            if (itBids->Value > HighBid)
            {
                const CardCollection_t& WantCards = Trade.GetWantCards();
                HighBid = itBids->Value;
                ASSERT(1000 * OfferCards.GetTotalQuantity() ==
                    HighBid * WantCards.GetTotalQuantity());
                if (1000 * OfferCards.GetTotalQuantity() !=
                    HighBid * WantCards.GetTotalQuantity())
                {
                    LogError(L"Trade(%d): HighBid (%d) x WantTotal (%d) = %d",
                        Trade.GetId(), HighBid, WantCards.GetTotalQuantity(),
                        1000 * OfferCards.GetTotalQuantity());
                }
            }
        }
        if ((0 < LowAsk) && (0 < HighBid) && (LowAsk <= HighBid))
        {
            // Should never happen.
            LogError(L"GetCardValues() %ls: LowAsk (%d) < HighBid (%d)",
                Card.GetName(), LowAsk, HighBid);
        }
        else
        {
// NOTE set elements are apparently immutable now, so just commenting out all mutations to get it comipiling
//            itCvs->SellAt = LowAsk;
//            itCvs->BuyAt = HighBid;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
