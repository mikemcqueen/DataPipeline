/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// LonCardSet_t.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_LONCARDSET_T_H
#define Include_LONCARDSET_T_H

#include "LonCard_t.h"
#include "AutoCs.h"
#include "LonCardMap_t.h"

class CardCollection_t;
class CardValueSet_t;

/////////////////////////////////////////////////////////////////////////////

struct NameFixup_t
{
    NameFixup_t(
        LPCWSTR pszKeywords,
        LPCWSTR pszName)
    :
        strKeywords(pszKeywords),
        strName(pszName)
    {
    }
    std::wstring strKeywords;
    std::wstring strName;

private:

    NameFixup_t();

};
typedef std::vector<NameFixup_t> NameFixupVector_t;

/////////////////////////////////////////////////////////////////////////////

class LonCardSet_t
{

friend class LonTrader_t; // CmdCards() calls ValueCards()

public:

    struct Flag
    {
        static const Flag_t SortByHighBid     = 1;
        static const Flag_t SortByLowAsk      = 2;
        static const Flag_t SortByBoughtCount = 4;
        static const Flag_t SortBySoldCount   = 8;

        static const Flag_t ShowDetail        = 0x100;
    };

    typedef std::pair<std::wstring, Card_t> Pair_t;

    static const wchar_t XmlElementName[];

private:

    NameFixupVector_t m_NameFixups;
    LonCardMap_t      m_Cards;
	IdCardMap_t       m_CardsById;
    mutable
    CAutoCritSec      m_csCards;

    mutable
    LonCardMap_t      m_NewCards;
    mutable
    CAutoCritSec      m_csNewCards;

public:

    LonCardSet_t();
    ~LonCardSet_t();

    void
    AddCard(
        const LonCard_t& Card);

    bool
    ReadCards(
        const wchar_t* pszFile);

    void
    InitRemovedTrades(
        const TradeMap_t& Trades);

    void
    WriteCards(
        bool bQuiet = false);

/*
    bool
        DbWriteCards();
*/

    const Card_t*
    Find(
        const wchar_t* Name) const;

    // Given a manufactured approximation of a real card,
    // return a pointer to the matching card in the cardlist.
    const Card_t*
    Lookup(
        const LonCard_t& Card) const;

    const Card_t*
    Lookup(
        const wchar_t* pszText,
              bool     bNewCard = false) const;

    const LonCard_t*
    Lookup(
        CardId_t id) const;

    const Card_t*
    LookupPartialAtBeginning(
        const wchar_t* pszText,
              bool     bQuiet = false) const;

    const Card_t*
    LookupPartialAtAnyPos(
        const wchar_t* pszText) const;

    const Card_t*
    Lookup(
        LonCard_t::Number_t Number) const;

    void
    ShowCards(
        bool   bValuedOnly  = false,
        bool   bDetail      = false,
        Flag_t Flags = 0,
        size_t Order        = 0) const;

    void
    ShowNewCards(
        bool bDetail) const;

    void
    ShowTransactions(
        Flag_t Flags) const;

    void
    WriteNewCards();

    void
    ValueCards(
        const Flag_t Flag = 0);

    void
    SetCardPointers(
        CardCollection_t& Cards);

    bool
    WriteXml(
        const wchar_t* pszFilename) const;

    bool
    WriteXml(
        IXmlWriter *pWriter) const;

    void
    OnRemoveTrade(
        const TradeId_t TradeId);

    // Remove all dependent TradeIds in a set:
    void
    OnRemoveTrades(
        TradeIdSet_t TradeIds);

    void
    InitTransactions(
        const TradeMap_t& Trades);

    void
    GetCardValues(
        CardValueSet_t& Cards) const;

private:

    void
    ShowCards(
    const LonCardMap_t& Cards,
          bool          bValuedOnly,
          bool          bDetail,
          Flag_t        Flags,
          size_t        Order) const;

    void
    FoilifyNewCards();

    void
    TruncateNewLootCards();

    size_t
    WriteNewCards(
        CDatabase& db);

    void
    AddNewCard(
        const wchar_t* pszText) const;

    bool
    AddNewCard(
        const LonCard_t& Card) const;

    void
    Fixup(
        std::wstring& strName) const;

    void
    RemoveDependentTradeIds(
        const TradeIdSet_t&     TradeIds,
              CardCollection_t& NoBidCards,
              CardCollection_t& NoAskCards);

    bool
    RemoveDependentTradeId(
              LonCard_t& Card,
        const bool       bBids,
        const TradeId_t  TradeId);

    size_t
    ValueCards(
        const Flag_t Flag,
        const Card_t* pCard);

    size_t
    ValueCards(
        const Flag_t Flag,
        const CardCollection_t& Collection,
              size_t            CardCount,
              size_t            Iteration);

    void
    ValueCardsLoop(
        const Flag_t Flag);

	void
    BuildIdCardMap();

private:

    LonCardSet_t(const LonCardSet_t&);
    LonCardSet_t& operator=(const LonCardSet_t&);
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_LONCARDSET_T_H

/////////////////////////////////////////////////////////////////////////////
