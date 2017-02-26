///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// Trade_t.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TRADE_T_H
#define Include_TRADE_T_H

#include "CardCollection_t.h"
#include "TradeTypes.h"

class Card_t;
class CDatabase;
class DbTrades_t;

///////////////////////////////////////////////////////////////////////////////
//
// Trade_t
//
///////////////////////////////////////////////////////////////////////////////

class Trade_t
{

public:

    static const wchar_t Trade_t::XmlElementName[];
    static const wchar_t Trade_t::XmlElementOffer[];
    static const wchar_t Trade_t::XmlElementWant[];

    static const wchar_t Trade_t::XmlAttributeId[];

    struct Flag
    {
        // typedef flag_word Flag_t;
        // Flag_t::value_type
        static const Flag_t HasLootCard = 0x00000001;
        static const Flag_t Removed     = 0x00000002;
        static const Flag_t Other       = 0x00000004;
    };

    //
    // Data_t:
    //

    struct Data_t
    {
        CardCollection_t Items;
        size_t           Value;
    };

private:

    TradeId_t    m_Id;
    std::wstring m_strUser;
    Data_t       offer;
    Data_t       want;
    Flag_t       m_Flags;
    __time64_t   m_PostedTime;
    __time64_t   m_RemovedTime;

public:

    typedef std::pair<TradeId_t, Trade_t> Pair_t;

    Trade_t();

    bool
    empty() const;

    void
    Clear();
  
    //
    // Get offered card collection:
    //
    const CardCollection_t&
    GetOfferCards() const
    {
        return offer.Items;
    }

    CardCollection_t&
    GetOfferCards()
    {
        return offer.Items;
    }

    //
    // Get wanted card collection:
    //
    const CardCollection_t&
    GetWantCards() const
    {
        return want.Items;
    }

    CardCollection_t&
    GetWantCards()
    {
        return want.Items;
    }

    // TODO: Move Get/Set Value to Data_t
    // Possibly IsOffered/IsWanted as well.

    size_t
    GetOfferValue() const
    {
        return offer.Value;
    }

    void
    SetOfferValue(
        size_t Value)
    {
        offer.Value = Value;
    }

    size_t
    GetWantValue() const
    {
        return want.Value;
    }

    void
    SetWantValue(
        size_t Value)
    {
        want.Value = Value;
    }

    bool
    IsOffered(
        const Card_t* pCard,
              size_t* pCount = NULL) const;

    bool
    IsWanted(
        const Card_t* pCard,
              size_t* pCount = NULL) const;

    size_t
    GetHighBid() const;

    size_t
    GetLowAsk() const;

    void
    Show() const;

    bool
    AddOfferCard(
        const Card_t* pCard,
              size_t  Quantity = 1)
    {
        return AddCard(offer.Items, pCard, Quantity);
    }

    bool
    AddWantCard(
        const Card_t* pCard,
              size_t  Quantity = 1)
    {
        return AddCard(want.Items, pCard, Quantity);
    }

    bool
    Compare(
        const Trade_t& Trade,
              bool     bCompareUser = false) const;

    bool
    HasLootCard() const
    {
        return HasLootCard(offer.Items) || HasLootCard(want.Items);
    }

/*
    bool 
    IsRealId() const 
    {
        return 1000 < GetId();
    }
*/

    TradeId_t
    GetId() const
    {
        return m_Id;
    }

    void
    SetId(
        const TradeId_t TradeId)
    {
        m_Id = TradeId;
    }

    const wchar_t*
    GetUser() const 
    {
        return m_strUser.c_str();
    }

    void
    SetUser(
        const wchar_t* pUser)
    {
        m_strUser.assign(pUser);
    }

    Flag_t
    GetFlags() const
    {
        return m_Flags;
    }

    void
    SetFlags(
        const Flag_t Flags)
    {
        m_Flags |= Flags;
    }

    bool
    TestFlags(
        const Flag_t Flags) const
    {
        return Flags == (m_Flags & Flags);
    }

    void
    SetExclusiveFlags(
        const Flag_t Flags)
    {
        m_Flags = Flags;
    }

    // Not the most expressive name but it's currently a way of
    // saying a trade is not removed, and has loot cards.
    bool
    IsActive() const
    {
        return (0 == (m_Flags & Flag::Removed)) &&
               (0 != (m_Flags & Flag::HasLootCard));
    }

    void
    SetPostedTime()
    {
        _time64(&m_PostedTime);
    }

    void
    SetPostedTime(
        const __time64_t& time)
    {
        m_PostedTime = time;
    }

    const __time64_t&
    GetPostedTime() const
    {
        return m_PostedTime;
    }

    void
    SetRemovedTime()
    {
        _time64(&m_RemovedTime);
    }

    void
    SetRemovedTime(
        const __time64_t& time)
    {
        m_RemovedTime = time;
    }

    const __time64_t&
    GetRemovedTime() const
    {
        return m_RemovedTime;
    }

    void
    Read(
        CDatabase&  Db,
        DbTrades_t& RsTrades);

    bool
    WriteTrade(
        CDatabase&  db) const;

    bool
    ReadXmlFile(
        const wchar_t* pszFilename);

    bool
    WriteXmlFile(
        const wchar_t* pszFilename) const;

    bool
    ReadXml(
        IXmlReader* pReader);

    bool
    WriteXml(
        IXmlWriter* pWriter,
        TradeId_t   TradeId) const;

private:

    bool
    AddCard(
              CardCollection_t& Cards,
        const Card_t*           pCard,
              size_t            Quantity = 1);

    bool
    HasLootCard(
        const CardCollection_t& Cards) const;

    long
    WriteCardGroup(
              CDatabase&        db,
        const CardCollection_t& Items) const;

    long
    GetNewCardGroup(
        CDatabase& db) const;
};


///////////////////////////////////////////////////////////////////////////////

#endif // Include_TRADE_T_H

///////////////////////////////////////////////////////////////////////////////
