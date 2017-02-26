/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// LonCard_t.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_LONCARD_T_H
#define Include_LONCARD_T_H

#include "Card_t.h"
#include "CommonTypes.h"
#include "TradeValue_t.h"

// HACK
#include "TradeMap_t.h"

namespace ShowFlags
{
    static const Flag_t ValuedOnly  = 0x00000001;
    static const Flag_t Detail      = 0x00000002;
    static const Flag_t SortByValue = 0x00000004;
};

namespace CardType
{
    static const int Loot = L'#';

    bool IsNumberPrefixed(int Type);
};

extern const wchar_t XmlAttributeCount[];

/////////////////////////////////////////////////////////////////////////////

class LonCard_t :
    public Card_t
{

friend class LonCardSet_t;

public:

    typedef std::pair<std::wstring, LonCard_t> Pair_t;

    static const wchar_t XmlElementName[];

    static const wchar_t XmlAttributeName[];
    static const wchar_t XmlAttributeSet[];
    static const wchar_t XmlAttributeRarity[];
    static const wchar_t XmlAttributeType[];
    static const wchar_t XmlAttributeFoil[];
    static const wchar_t XmlAttributeBooster[];

    /////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma warning(push)
#pragma warning(disable:4201) // nameless struct/union
#endif

    struct Number_t
    {
        union
        {
            struct
            {
                BYTE set;
                BYTE type;
                struct
                {
                    WORD num    :14;
                    WORD foil   :1;
                    WORD booster:1;
                };
            };
            DWORD value;
        };

        Number_t() :
            value(0)
        { }

        bool operator==(const Number_t& cn) const
        {
            return cn.value == value;
        }
    };

#if _MSC_VER > 1000
#pragma warning(pop)
#endif

    /////////////////////////////////////////////////////////////////////////////

    struct Flag
    {
        static const Flag_t Reprice          = 0x00000001;

        static const Flag_t ShowValues       = 0x00000001;
        static const Flag_t ShowTransactions = 0x00000002;
        static const Flag_t ShowDetail       = 0x00000004;
    };

    /////////////////////////////////////////////////////////////////////////////

    struct ValueData_t // TradeData_t ?
    {
        TradeValueSet_t TradeValues;       // set of trades that include this card.
        TradeId_t       TradeId;           // trade id on which this card is directly priced.
        size_t          Value;             // value of that trade (questionable optimization - it's just TradeValues[TradeId].Value).
        TradeIdSet_t    DependentTradeIds; // set of tradeids in pricing dependency chain, starting with TradeId.

        ValueData_t() :
            TradeId(0),
            Value(0)
        {}

        void
        SetTradeValue(
            const TradeValue_t& tv)
        {
            TradeId = tv.TradeId;
            Value   = tv.Value;
        }

        void
        Clear()
        {
            DependentTradeIds.clear();
            TradeValues.clear();
            TradeId = 0;
            Value   = 0;
        }

        void
        SetDependentTrades(
            const TradeIdSet_t& TradeIds)
        {
            DependentTradeIds = TradeIds;
        }

    private:

//        ValueData_t(const ValueData_t&);
        ValueData_t& operator= (const ValueData_t&);

    };

    /////////////////////////////////////////////////////////////////////////////

private:

    Number_t    m_Number;
    ValueData_t m_Bids;
    ValueData_t m_Asks;
    Flag_t      m_Flags;

    TradeValueSet_t m_Bought;
    TradeValueSet_t m_Sold;

public:

    static
    Number_t
    GetCardNumber(
        const std::wstring& str);

    static
    bool
    UiNameCleanup(
        std::wstring& Name);

    static
    bool
    TruncateName(
        std::wstring& Name);

    struct CompareName
    {
        typedef std::wstring _Ty;
        bool operator()(const _Ty& _Left, const _Ty& _Right) const;
    };

	struct CompareId
	{
        bool operator()(const CardId_t& lhs, const CardId_t& rhs) const
		{
			return lhs < rhs;
		}
	};

public:

    LonCard_t(
        Card_t::Token_t Token);

    explicit
    LonCard_t(
              CardId_t Id,
        const wchar_t* pszName,
              size_t   Value = 0);

    bool
    IsFoilCard() const;

    bool
    IsLootCard() const;

    bool
    IsBoosterPack() const; 

    // Accessors:

    Number_t             GetNumber() const  { return m_Number; }

    const ValueData_t&   GetBids() const    { return m_Bids; }
    ValueData_t&         GetBids()          { return m_Bids; }

    const ValueData_t&   GetAsks() const    { return m_Asks; }
    ValueData_t&         GetAsks()          { return m_Asks; }

    // Helper functions:

    size_t
    GetHighBid() const;

    size_t
    GetLowAsk() const;

    bool
    IsValued() const
    {
        // TODO: rename or change caller to hardcode this. 
        // "Valued" is too ambiguous.
        // TODO: different than bid/ask sets being empty?
        return (0 != GetHighBid()) || (0 != GetLowAsk());
    }

    void
    AddAsk(
        const TradeValue_t& tv);

    void
    AddBid(
        const TradeValue_t& tv);

    void
    ShowValue(
        Flag_t Flags    = Flag::ShowValues,
        size_t Order    = 0,
        size_t Quantity = 0) const;

    size_t
    GetTransactionCount() const
    {
        return m_Bought.size() + m_Sold.size();
    }

    void
    ShowTransactions(
        Flag_t Flags = Flag::ShowTransactions,
        size_t Income = 0) const;

    bool
    RemoveDependentTradeId(
        bool      bBids,
        TradeId_t TradeId);

    void
    SetFlag(
        Flag_t Flag)
    {
        m_Flags |= Flag;
    }

    bool
    CheckFlag(
        Flag_t Flag) const
    {
        return 0 != (m_Flags & Flag);
    }

    void
    ClearFlag(
        Flag_t Flag)
    {
        m_Flags &= ~Flag;
    }

    bool
    WriteXml(
        IXmlWriter* pWriter) const;

    bool
    ReadXml(
        IXmlReader* pReader);

private:

    bool
    Init(
        const wchar_t* pszText);

    bool
    Init(
              int      id,
        const wchar_t* pszName,
              size_t   Value);

    void
    Add();

private:

    LonCard_t();
    LonCard_t& operator=(const LonCard_t&);
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_LONCARD_T_H

/////////////////////////////////////////////////////////////////////////////
