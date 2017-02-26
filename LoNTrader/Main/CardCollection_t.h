/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// CardCollection_t.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_CARDCOLLECTION_T_H
#define Include_CARDCOLLECTION_T_H

#include "Card_t.h"
#include "CommonTypes.h"

class CDatabase;
class Card_t;
class CardQuantityQueue_t;
class CardCollection_t;

/////////////////////////////////////////////////////////////////////////////
//
// CardId_t, CardIdSet_t
//
/////////////////////////////////////////////////////////////////////////////

typedef unsigned long      CardId_t;
typedef std::set<CardId_t> CardIdSet_t;

/////////////////////////////////////////////////////////////////////////////
//
// CardValue_t
//
/////////////////////////////////////////////////////////////////////////////

struct CardValue_t
{
    static const wchar_t XmlElementName[];
    static const wchar_t XmlElementBuyAt[];
    static const wchar_t XmlElementSellAt[];

public:

    const Card_t* pCard;
    size_t        BuyAt;
    size_t        SellAt;

    explicit
    CardValue_t(
        const Card_t* pC = NULL)
        :
        pCard(pC),
        BuyAt(0),
        SellAt(0)
    { }

    CardValue_t(
        const CardValue_t& cv)
    {
        *this = cv;
    }

    CardValue_t&
    operator=(
        const CardValue_t& cv)
    {
//        std::copy(&cv, &cv + 1, this);
        memcpy(this, &cv, sizeof(cv));
        return *this;
    }

    bool
    operator()(
        const CardValue_t& lhs,
        const CardValue_t& rhs) const;

    bool
    operator==(
//        const CardValue_t& lhs,
        const CardValue_t& rhs) const;

    void
    Show() const;

    bool
    WriteXml(
              IXmlWriter* pWriter,
        const wchar_t*    ElementName = XmlElementName) const;

    bool
    ReadXml(
              IXmlReader* pReader,
        const wchar_t*    ElementName = XmlElementName);
};

/////////////////////////////////////////////////////////////////////////////
//
// CardValueSet_t
//
/////////////////////////////////////////////////////////////////////////////

class CardValueSet_t :
    public std::set<CardValue_t, CardValue_t>
{

public:

    static const wchar_t XmlElementName[];

    // Construct an empty set.

    CardValueSet_t() {}

    // Construct a set from a container; all values default to zero.

    template<class T>
    CardValueSet_t(
        const T& Cards);

    // Add cards from a container; all values default to zero.

    template<class T>
    size_t
    AddCards(
        const T& Cards);

    // Add or change a card value.

    void
    SetCardValue(
        const CardValue_t& CardValue);

    // Show all card values.

    void
    Show(
        const wchar_t* pHeader = NULL) const;

    bool
    WriteXmlFile(
        const wchar_t* pszFilename) const;

    size_t
    ReadXmlFile(
        const wchar_t* pszFilename);

    bool
    WriteXml(
              IXmlWriter* pWriter,
        const wchar_t*    ElementName = XmlElementName) const;

    bool
    ReadXml(
              IXmlReader* pReader,
        const wchar_t*    ElementName = XmlElementName);
};

/////////////////////////////////////////////////////////////////////////////
//
// CardQuantity_t
//
/////////////////////////////////////////////////////////////////////////////

struct CardQuantity_t
{
    static const wchar_t XmlElementName[];
    static const wchar_t XmlElementQuantity[];

public:

    const Card_t* pCard;
    CardId_t      CardId;
    size_t        Quantity;

    explicit
    CardQuantity_t(
        const Card_t* pC,
              size_t  Count = 1);

    CardQuantity_t(
        CardId_t Id = 0,
        size_t   Count = 1);

    bool
    Compare(
        const CardQuantity_t& Other) const
    {
        return (CardId == Other.CardId) && (Quantity == Other.Quantity);
    }

    bool
    operator()(
        const CardQuantity_t& lhs,
        const CardQuantity_t& rhs) const
    {
        return lhs.CardId < rhs.CardId;
    }

    bool
    WriteXml(
              IXmlWriter* pWriter,
        const wchar_t*    ElementName = XmlElementName) const;

    bool
    ReadXml(
              IXmlReader* pReader,
        const wchar_t*    ElementName = XmlElementName);

#if 0
    template<
        class AddFunc_t>
    static
    bool
    ReadXmlContainer(
              IXmlReader*  pReader,
        const wchar_t*     ElementName,
              AddFunc_t    AddFunc);

    template<class T>
    static
    bool
    WriteXmlContainer(
              IXmlWriter* pReader,
        const wchar_t*    ElementName,
        const T&          Container);
#endif

    struct CompareCardId
    {
        CardId_t Id;
        CompareCardId(CardId_t InitId) : Id(InitId)  {}
        bool operator()(const CardQuantity_t& CardQ) const { return CardQ.CardId == Id; }
    };

    struct TotalQuantity
    {
        size_t Total;

        TotalQuantity() : 
            Total(0)
        { } 

        void operator()(const CardQuantity_t& CardQ)
        {
            Total += CardQ.Quantity;
        }
    };

    struct IsBooster :
        std::unary_function<CardQuantity_t, bool>
    {
        bool operator()(const CardQuantity_t& CardQ) const;
    };
};

typedef std::vector<const CardQuantity_t*> CardQuantityPtrVector_t;

///////////////////////////////////////////////////////////////////////////////
//
// CardQuantityQueue_t
//
///////////////////////////////////////////////////////////////////////////////

class CardQuantityQueue_t :
    public std::deque<CardQuantity_t>
{

    static const wchar_t XmlElementName[];

public:

    void 
    Show(
        const wchar_t* pHeader = NULL,
              Flag_t   Flags = 0) const;

    bool
    WriteXml(
              IXmlWriter* pWriter,
        const wchar_t*    ElementName = XmlElementName) const;

    bool
    ReadXml(
              IXmlReader* pReader,
        const wchar_t*    ElementName = XmlElementName);

};

/////////////////////////////////////////////////////////////////////////////
//
// CardCollection_t
//
/////////////////////////////////////////////////////////////////////////////

class CardCollection_t :
    public std::set<CardQuantity_t, CardQuantity_t>
{

public:

    typedef std::pair<iterator, bool> Pair_t;

    static const wchar_t XmlElementName[];

public:

    bool
    Compare(
        const CardCollection_t& Other) const;

    bool
    HasOnly(
        const CardCollection_t& Cards,
              bool              bCompareQuantity = false) const;

    size_t
    GetTotalQuantity() const
    {
        return std::for_each(begin(), end(), CardQuantity_t::TotalQuantity()).Total;
    }

    size_t
    GetQuantity(
        const Card_t* pCard) const;

    size_t
    Remove(
        const CardCollection_t& Cards);

    size_t
    Remove(
        const CardQuantity_t& CardQ);

    size_t
    Add(
        const CardCollection_t& Cards);

    size_t
    Add(
        const CardQuantity_t& CardQ);

    void 
    Show(
              Flag_t   Flags,
              size_t   Order   = 0,
        const wchar_t* pHeader = NULL) const;

    bool
    WriteXmlFile(
        const wchar_t* pszFilename) const;

    size_t
    ReadXmlFile(
        const wchar_t* pszFilename);

    bool
    WriteXml(
              IXmlWriter* pWriter,
        const wchar_t*    ElementName = XmlElementName) const;

    bool
    ReadXml(
              IXmlReader* pReader,
        const wchar_t*    ElementName = XmlElementName);

    size_t
    Read(
        CDatabase& db,
        long       GroupId);
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_CARDCOLLECTION_T_H

/////////////////////////////////////////////////////////////////////////////
