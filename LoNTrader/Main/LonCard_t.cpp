///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// LonCard_t.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LonCard_t.h"
#include "DbCards_t.h"
#include "Log.h"
#include "XmlUtil.h"
#include "DbGroupedCards_t.h"

///////////////////////////////////////////////////////////////////////////////

#define EXTRALOG 0

//size_t LonCard_t::CompareCount = 0;

///////////////////////////////////////////////////////////////////////////////

namespace CardType
{
    bool IsNumberPrefixed(int Type)
    {
        switch (Type)
        {
        case L'C':
        case L'U':
        case L'R':
        case L'F':
        case L'M':
        case L'P':
            return true;
        default:
            return false;
        }
    }
};

///////////////////////////////////////////////////////////////////////////////

bool
LonCard_t::
CompareName::
operator()(
    const _Ty& _Left,
    const _Ty& _Right) const
{
//        ++LonCard_t::CompareCount;
#if 1
    LonCard_t::Number_t left = LonCard_t::GetCardNumber(_Left);
    LonCard_t::Number_t right = LonCard_t::GetCardNumber(_Right);
#else
    LonCard_t::Number_t left = _Left.GetNumber();
    LonCard_t::Number_t right = _Right.GetNumber();
#endif
    if ((CardType::Loot != left.type) &&
        (CardType::Loot != right.type))
    {
        if (left.set != right.set)
            return left.set < right.set;

        if (left.type == right.type)
        {
            if (left.num == right.num)
            {
                if (left.foil && right.foil)
                    return false;
                else if (left.foil)
                    return false;
                else if (right.foil)
                    return true;
                else 
                    return false;
            }
            return left.num < right.num;
        }

        // Cards of "CRU" types get sorted numerically together.
        bool bLeftBasic = false;
        bool bRightBasic = false;
        switch (left.type)
        {
        case L'C':
        case L'U':
        case L'R':
            bLeftBasic = true;
            break;
        }
        switch (right.type)
        {
        case L'C':
        case L'U':
        case L'R':
            bRightBasic = true;
            break;
        }
        if (bLeftBasic && bRightBasic)
            return left.num < right.num;
        else if (bLeftBasic)
            return true;
        else if (bRightBasic)
            return false;
        else // Cards of other types ("FP") are sort alphabetically by type
            return left.type < right.type;
    }
    else if (CardType::Loot != left.type)
        return true;
    else if (CardType::Loot != right.type)
        return false;
    else
        return 0 > wcscmp(_Left.c_str(), _Right.c_str());
}

/////////////////////////////////////////////////////////////////////////////

/* static */
LonCard_t::Number_t 
LonCard_t::
GetCardNumber(
    const std::wstring& str) // full text card name: ID TEXT (foil) 
{
    Number_t zero;
    Number_t num;
    const wchar_t* pszText = str.c_str();
    // prefixed with numeral but no space immediately following,
    // is special case for 
    if (iswdigit(pszText[0]))
    {
        if (L' ' == pszText[1])                           // "3 jim lee posters"
            goto loot;
        if ((L't' == pszText[1]) && (L'h' == pszText[2])) // 4th of July
            goto loot;

        // Set
        int set = _wtoi(pszText);
        if ((1 > set) || (9 < set))
        {
            ASSERT(0);
            return zero;
        }
        num.set = BYTE(set);

        // Type
        while (iswdigit(*pszText)) ++pszText;
        wchar_t cType = *pszText++;
        switch (cType)
        {
        case L'C': // common
        case L'U': // uncommon
        case L'R': // rare
        case L'F': // fixed
        case L'M': // trak raid card
        case L'P': // promo
            break;
        default:
            LogError(L"GetCardNumber(): Invalid type '%c' for '%ls'", cType, str.c_str());
            ASSERT(0);
            return zero;
        }
        num.type = static_cast<BYTE>(cType);

        // Num
        int n = _wtoi(pszText);
        if ((0 >= n) || (1000 <= n))
        {
            ASSERT(0);
            return zero;
        }
        num.num = n;

        // Flags
        while (iswdigit(*pszText)) ++pszText;
        if (NULL != wcsstr(pszText, L" (foil)"))
            num.foil = 1;

        return num;
    }
loot:
    num.type = CardType::Loot;
    static const wchar_t szOBooster[] = L"Oathbound Booster Pack";
    static const wchar_t szFBooster[] = L"Forsworn Booster Pack";
    static const wchar_t szIBooster[] = L"Inquisitor Booster Pack";
    static const wchar_t szOathbreakerBooster[] = L"Oathbreaker Booster Pack";
// TODO: even better, put these card ids in a database record Booster_Table
// or at least a static/global table of ptrs.
    if (0 == wcscmp(pszText, szOBooster))
        num.booster = 1;
    else if (0 == wcscmp(pszText, szFBooster))
        num.booster = 1;
    else if (0 == wcscmp(pszText, szIBooster))
        num.booster = 1;
    else if (0 == wcscmp(pszText, szOathbreakerBooster))
        num.booster = 1;
    return num;
}

/////////////////////////////////////////////////////////////////////////////
// accessed by TradePoster::Data_t

LonCard_t::
LonCard_t(
    Card_t::Token_t Token)
:
    Card_t(Token),
    m_Flags(0)
{
}

/////////////////////////////////////////////////////////////////////////////

LonCard_t::
LonCard_t(
    CardId_t       id,
    const wchar_t* pszName,
    size_t         Value)
:
    Card_t(id, pszName, Value),
    m_Number(GetCardNumber(pszName)),
    m_Flags(0)
{
}

/////////////////////////////////////////////////////////////////////////////

bool
LonCard_t::
IsLootCard() const
{
    return CardType::Loot == m_Number.type;
}

/////////////////////////////////////////////////////////////////////////////

void
LonCard_t::
ShowValue(
    Flag_t Flags,
    size_t Order,
    size_t Quantity) const
{
    wchar_t szQuantity[16] = L"";
    if (0 < Quantity)
        wsprintf(szQuantity, L"%3d x ", Quantity);
    LogAlways(L"%5d,\"%ls%-45ls\", Bids(%3d) Asks(%3d) - HiBid(%6d) LoAsk(%6d) Value(%d,%d)",
        GetId(), szQuantity, GetName(),
        m_Bids.TradeValues.size(), m_Asks.TradeValues.size(),
        GetHighBid(), GetLowAsk(),
        Quantity * GetHighBid(), Quantity * GetLowAsk());

    const bool bDetail = 0 != (Flag::ShowDetail & Flags);
    if (bDetail)
    {
        // TODO: ValueData_t::Show
        m_Bids.TradeValues.Show(L"Bids", Order);
        m_Asks.TradeValues.Show(L"Asks", Order);
        m_Bids.DependentTradeIds.Show(L"Bids Dependent Trades");
        m_Asks.DependentTradeIds.Show(L"Asks Dependent Trades");
        LogAlways(L"Bids TradeId (%d) Value (%d)", m_Bids.TradeId, m_Bids.Value);
        LogAlways(L"Asks TradeId (%d) Value (%d)", m_Asks.TradeId, m_Asks.Value);
    }
}

/////////////////////////////////////////////////////////////////////////////

void
LonCard_t::
ShowTransactions(
    Flag_t Flags,
    size_t Income) const
{
    if (0 == GetTransactionCount())
    {
        LogAlways(L"%ls: No transactions", GetName());
        return;
    }
    const bool bDetail = (0 != (Flag::ShowDetail & Flags));

    size_t TotalBought = m_Bought.TotalQuantity();
    size_t TotalSold   = m_Sold.TotalQuantity();
    size_t MeanBought  = m_Bought.MeanValue(bDetail);
    size_t MeanSold    = m_Sold.MeanValue(bDetail);
    size_t BuyPrice    = GetHighBid();
    size_t SellPrice   = GetLowAsk();
    size_t Margin      = SellPrice - BuyPrice;

    size_t MeanMargin  = MeanSold - MeanBought;
    size_t MeanIncome  = MeanMargin * TotalSold;

    if (0 == Income)
        Income = Margin * TotalSold;

    LogAlways(L"%4d,\"%-40s\", B(%3d,%4d) @(%5d,%5d) - S(%3d,%4d) @(%5d,%5d) M(%5d,%5d) I(%6d,%6d)",
              GetId(),         GetName(),
              m_Bought.size(), TotalBought,
              MeanBought,      BuyPrice, 
              m_Sold.size(),   TotalSold,
              MeanSold,        SellPrice,
              MeanMargin,      Margin,
              MeanIncome,      Income);

    if (bDetail)
    {
        // TODO: ValueData_t::Show
        m_Bought.Show(L"Bought");
        m_Sold.Show(L"Sold");
//  LogAlways(L"Bids TradeId (%d) Value (%d)", m_Bids.TradeId, m_Bids.Value);
//  LogAlways(L"Asks TradeId (%d) Value (%d)", m_Asks.TradeId, m_Asks.Value);
    }
}

/////////////////////////////////////////////////////////////////////////////

bool
LonCard_t::
IsBoosterPack() const
{
    return 1 == m_Number.booster;
}

/////////////////////////////////////////////////////////////////////////////

bool
LonCard_t::
IsFoilCard() const
{
    return 1 == m_Number.foil;
}

/////////////////////////////////////////////////////////////////////////////

/*static*/
bool
LonCard_t::
TruncateName(
    std::wstring& strName)
{
    const size_t StartLength = strName.length();
    const size_t Pos = strName.find(L"...");
    if (strName.npos != Pos)
        strName.erase(Pos);
    return strName.length() < StartLength;
}

/////////////////////////////////////////////////////////////////////////////

size_t
LonCard_t::
GetHighBid() const
{
    return (0 < GetValue()) ? GetValue() : m_Bids.Value; //m_Bids.HighestValue();
}

/////////////////////////////////////////////////////////////////////////////

size_t
LonCard_t::
GetLowAsk() const
{
    return (0 < GetValue()) ? GetValue() : m_Asks.Value ; // m_Asks.LowestValue();
}

/////////////////////////////////////////////////////////////////////////////

void
LonCard_t::
AddAsk(
    const TradeValue_t& tv)
{
#if EXTRALOG
    LogInfo(L"AddAsk: %ls (%d) (%d)", GetName(), tv.TradeId, tv.Value);
#endif
    ASSERT(0 != tv.Value);
    LonCard_t::ValueData_t& Data =  GetAsks();
    if ((0 == Data.Value) || (tv.Value < Data.Value))
        Data.SetTradeValue(tv);
    TradeValueSet_t::Pair_t pr = m_Asks.TradeValues.insert(tv);
    // If an Asks TradeValue for this TradeId already exists
    if (!pr.second)
    {
        // Update the TradeValue.Value to the new value.
		// NOTE set elements are apparently immutable now, so just commenting out all mutations to get it comipiling
		//pr.first->Value = tv.Value;

        // If this was the TradeId the for the LowestAsk value, and the new
        // value is higher
        if ((tv.TradeId == Data.TradeId) && (tv.Value > Data.Value))
        {
            // We need to find and set a new LowestAsk.
            Data.SetTradeValue(*Data.TradeValues.Lowest());
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void
LonCard_t::
AddBid(
    const TradeValue_t& tv)
{
#if EXTRALOG
    LogInfo(L"AddBid: %ls (%d) (%d)", GetName(), tv.TradeId, tv.Value);
#endif
    ASSERT(0 != tv.Value);
    LonCard_t::ValueData_t& Data =  GetBids();
    if ((0 == Data.Value) || (tv.Value > Data.Value))
        Data.SetTradeValue(tv);
    TradeValueSet_t::Pair_t pr = Data.TradeValues.insert(tv);

    // If a Bids TradeValue for this TradeId already exists
    if (!pr.second)
    {
        // Update the TradeValue.Value to the new value.
		// NOTE set elements are apparently immutable now, so just commenting out all mutations to get it comipiling
		//pr.first->Value = tv.Value;

        // If this was the TradeId the for the HighestBid value, and the new
        // value is lower
        if ((tv.TradeId == Data.TradeId) && (tv.Value < Data.Value))
        {
            // We need to find and set a new HighestBid.
            Data.SetTradeValue(*Data.TradeValues.Highest());
        }
    }

}

/////////////////////////////////////////////////////////////////////////////

bool
LonCard_t::
UiNameCleanup(
    std::wstring& strName)
{
    bool bTruncated = TruncateName(strName);

    // Convert leading lowercase 'L' to uppercase 'I'.
    bool bTweaked = false;
    size_t Pos = 0;
    do
    {
        if (L'\0' == strName[Pos])
        {
            LogWarning(L"UiNameCleanup(): Advanced to end of string (%s).", strName.c_str());
            break;
        }
        if (L'l' == strName[Pos])
        {
            strName[Pos] = L'I';
            bTweaked = true;
        }
        Pos = strName.find(L' ', ++Pos);
        if (strName.npos != Pos)
            ++Pos;
    } while ((strName.npos != Pos) && ('\0' != strName[Pos]));
    return bTweaked || bTruncated;
}

/////////////////////////////////////////////////////////////////////////////

bool
LonCard_t::
RemoveDependentTradeId(
    bool       bBids,
    TradeId_t  TradeId)
{
    LonCard_t::ValueData_t& Data = (bBids) ? GetBids() : GetAsks();
    Data.DependentTradeIds.erase(TradeId);
    if (0 == Data.TradeValues.erase(TradeValue_t(TradeId)))
        return false;

    TradeValueSet_t::const_iterator it = (bBids) ? Data.TradeValues.Highest()
                                                 : Data.TradeValues.Lowest();
    if (Data.TradeValues.end() != it)
    {
        Data.TradeId = it->TradeId;
        Data.Value   = it->Value;
    }
    else
    {
        ASSERT(Data.TradeValues.empty());
        Data.TradeId = 0;
        Data.Value   = 0;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////
