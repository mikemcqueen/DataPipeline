////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// DiffDbFaster_t.cpp
//
// Does a diff of tables in two databases, reading everything into memory first
// for faster performance.
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DiffDbFaster_t.h"
#include "DiffDbUtil.h"
#include "Log.h"
#include "Macros.h"
#include "Timer_t.h"
#include "ProgramOptions_t.h"

using namespace Db;

namespace DiffDb
{

////////////////////////////////////////////////////////////////////////////////

Faster_t::
Faster_t() :
    m_pOldPriceMap(0),
    m_pOldItemPriceMap(0),
    m_pNewPriceMap(0),
    m_pNewItemPriceMap(0)
{
}

////////////////////////////////////////////////////////////////////////////////

Faster_t::
~Faster_t()
{
    delete m_pOldPriceMap;
    delete m_pOldItemPriceMap;
    delete m_pNewPriceMap;
    delete m_pNewItemPriceMap;
}

////////////////////////////////////////////////////////////////////////////////

void
Faster_t::
Init()
{
    m_OldSellers.clear();
    m_NewSellers.clear();
    m_RemovedSellers.clear();
    m_RemovedItemPriceMap.clear();
}

////////////////////////////////////////////////////////////////////////////////

bool
Faster_t::
Load(
    CDatabase& oldDb,
    CDatabase& newDb)
{
    static struct MinMaxPrice_t oldPrices;

    if (!oldDb.IsOpen())
    {
        delete m_pOldPriceMap;
        delete m_pOldItemPriceMap;
        m_pOldPriceMap = m_pNewPriceMap;
        m_pOldItemPriceMap = m_pNewItemPriceMap;
    }
    else
    {
        m_pOldPriceMap = new ItemSellerPriceMap_t;
        m_pOldItemPriceMap = new ItemPriceDataMap_t;
        Load(m_FirstDb, *m_pOldPriceMap, m_pOldItemPriceMap, oldPrices);
        oldDb.Close();
    }

    m_pNewPriceMap = new ItemSellerPriceMap_t;
    m_pNewItemPriceMap = new ItemPriceDataMap_t;
    MinMaxPrice_t newPrices;
    Load(newDb, *m_pNewPriceMap, m_pNewItemPriceMap, newPrices);
    newDb.Close();
    bool match = newPrices.Compare(oldPrices);
    if (!match)
    {
        LogError(L"Load(): OldPrices(%d,%d) NewPrices(%d,%d)",
            oldPrices.minPrice, oldPrices.maxPrice,
            newPrices.minPrice, newPrices.maxPrice);
        delete m_pNewPriceMap;
        delete m_pNewItemPriceMap;
        m_pNewPriceMap = m_pOldPriceMap;
        m_pNewItemPriceMap = m_pOldItemPriceMap;
        m_pOldPriceMap = 0;
        m_pOldItemPriceMap = 0;
    }
    else
    {
        oldPrices = newPrices;
    }
    return match;
}

////////////////////////////////////////////////////////////////////////////////

void
Faster_t::
Diff()
{
    Timer_t Timer(L"Faster_t::Diff()");

    ItemSellerSet_t ItemSellerSet;
    ItemSellerSet.AddSellers(*m_pOldPriceMap, &m_OldSellers);
    ItemSellerSet.AddSellers(*m_pNewPriceMap, &m_NewSellers);

    const bool showDots = PO::GetOption<bool>("showdots");

    size_t SaleCount = 0;
    ItemDataMap_t DiffMap;
    ItemSellerSet_t::const_iterator itItemSeller = ItemSellerSet.begin();
    for (; ItemSellerSet.end() != itItemSeller; ++itItemSeller)
    {
        if ((0 == ++SaleCount % 1000) && showDots) cout << '.';

        const ItemSellerPair_t& ItemSellerPair = *itItemSeller;
        const ItemId_t ItemId = ItemSellerPair.first;

        DiffData_t& Data = DiffMap.Add(ItemId);
        ProcessPrices(ItemSellerPair, Data, *m_pOldItemPriceMap);
    }
    if (showDots) cout << endl;
    LogInfo(L"SaleCount(%d) DiffMap.size(%d)", SaleCount, DiffMap.size());
    Dump(DiffMap);

    // Validate removed item prices
    ValidateRemovedItems(m_RemovedItemPriceMap, *m_pOldItemPriceMap);
    AddSalePrices(m_RemovedItemPriceMap);

    m_MatchData.Dump();
    size_t Removed = m_RemovedSellers.Dump();
    Removed += m_MatchData.MismatchValue + m_MatchData.RemovedAfterMismatchValue;
    LogInfo(L"Total Removed + Matched(%ls)", GetCoinString(Removed + m_MatchData.MatchValue));
    //TODO: Log.Flush();
    AddHiLoData(m_RemovedItemPriceMap, *m_pNewItemPriceMap);
}

////////////////////////////////////////////////////////////////////////////////

void
Faster_t::
ProcessPrices(
    const ItemSellerPair_t&   ItemSellerPair,
          DiffData_t&         Data,
          ItemPriceDataMap_t& OldItemPriceMap)
{
    PriceSet_t NewPrices(*m_pNewPriceMap, ItemSellerPair);
    PriceSet_t OldPrices(*m_pOldPriceMap, ItemSellerPair);

    // Eliminate item prices that are common to both old & new price set.
    EliminateCommonPrices(ItemSellerPair, OldPrices, NewPrices, Data, OldItemPriceMap);
    // One or both price sets is be empty
    // Process any remaining item prices in old set
    ProcessRemainingPrices(ItemSellerPair, OldPrices, Data);
}

////////////////////////////////////////////////////////////////////////////////
// Eliminate item prices that are common to both old & new price set.
void
Faster_t::
EliminateCommonPrices(
    const ItemSellerPair_t&   ItemSellerPair,
          PriceSet_t&         OldPrices,
          PriceSet_t&         NewPrices,
          DiffData_t&         Data,
          ItemPriceDataMap_t& OldItemPriceMap)
{
    EliminateExactPriceMatches(OldPrices, NewPrices, Data);
    EliminateRaisedLoweredPrices(ItemSellerPair, OldPrices, NewPrices, Data, OldItemPriceMap);
    // Any values remaining in NewPrices are "new prices"
    Data.AddedCount += NewPrices.size();
    Data.AddedValue = accumulate(NewPrices.begin(), NewPrices.end(), 0);
}

////////////////////////////////////////////////////////////////////////////////
// Walk new prices, remove matches from old & new, adjust unchanged.
void
Faster_t::
EliminateExactPriceMatches(
    PriceSet_t& OldPrices,
    PriceSet_t& NewPrices,
    DiffData_t& Data)
{
    if (!OldPrices.empty() && !NewPrices.empty())
    {
        PriceSet_t::iterator itNew = NewPrices.begin();
        while (NewPrices.end() != itNew)
        {
            PriceSet_t::iterator itOld = OldPrices.find(*itNew);
            if (OldPrices.end() != itOld)
            {
                Data.UnchangedCount++;
                Data.UnchangedValue += *itNew;
                OldPrices.erase(itOld);
                itNew = NewPrices.erase(itNew);
            }
            else
            {
                ++itNew;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// While there is still more than one old AND new price, determine whether
// the cheapest was raised or lowered; increment the corresponding DiffData
// variable, and remove the cheapest price from both sets.
void
Faster_t::
EliminateRaisedLoweredPrices(
    const ItemSellerPair_t&   ItemSellerPair,
          PriceSet_t& OldPrices,
          PriceSet_t& NewPrices,
          DiffData_t& Data,
          ItemPriceDataMap_t& OldItemPriceMap)
{
    while (!OldPrices.empty() && !NewPrices.empty())
    {
        //
        // TODO: min_element here is dumb - std:set is already sorted, just use begin()
        //
        PriceSet_t::iterator itNew = min_element(NewPrices.begin(), NewPrices.end());
        PriceSet_t::iterator itOld = min_element(OldPrices.begin(), OldPrices.end());
        if (*itNew > *itOld)
        {
            ++Data.RaisedCount;
            Data.RaisedValueDelta += (*itNew - *itOld);
            if (PO::GetFlags().Test(Flag::RevalueRaisedPrices))
            {
                OldItemPriceMap.Reprice(ItemSellerPair, *itOld, *itNew);
            }
        }
        else if (*itNew < *itOld)
        {
            ++Data.Lowered;
        }
        else
        {
            throw std::logic_error("Faster_t::ProcessCommonPrices(): I don't think this can happen?!");
        }
        OldPrices.erase(itOld);
        NewPrices.erase(itNew);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Add remaining prices to removeditems price map.
void
Faster_t::
ProcessRemainingPrices(
    const ItemSellerPair_t& ItemSellerPair,
          PriceSet_t&       OldPrices,
          DiffData_t&       Data)
{
    if (!OldPrices.empty())
    {
        // All values in OldPrices which remain after removing all common prices
        // from NewPrices are candidates for "Sold Items".
        const size_t RemovedCount = OldPrices.size();
        const size_t RemovedValue = accumulate(OldPrices.begin(), OldPrices.end(), 0);
        Data.RemovedCount += RemovedCount;
        Data.RemovedValue += RemovedValue;

        // Special consideration for sellers which have been removed entirely
        bool bAddRemovedItems = true;
        const SellerId_t SellerId = ItemSellerPair.second;
        if (m_NewSellers.end() == m_NewSellers.find(SellerId))
        {
            m_RemovedSellers.Add(SellerId, RemovedCount, RemovedValue, m_OldSellers, m_NewSellers);
            if (PO::GetFlags().Test(Flag::IgnoreRemovedSellers))
            {
                bAddRemovedItems = false;
            }
        }
        if (bAddRemovedItems)
        {
            // TODO: 1) make param, 2) declare local to Diff(), not member
            m_RemovedItemPriceMap.Add(ItemSellerPair, OldPrices, GetSecondTime());
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void
Faster_t::
Dump(
    const ItemDataMap_t& Map) const
{
    ItemDataMap_t::const_iterator it = Map.begin();
    DiffData_t Total;
//    LogInfo(L"\nName,Added,Removed,Raised,Lowered,Unchanged");
    for (; Map.end() != it; ++it)
    {
        const DiffData_t& Data = it->second;
        size_t Changed = Data.AddedCount + Data.RemovedCount + Data.RaisedCount + Data.Lowered;
        if (0 != Changed)
        {
#if 0
            wchar_t Name[100];
            LogInfo(L"%ls,%d,%d,%d,%d,%d",
                    Items_t::GetItemName(GetDb(Items), it->first, Name, _countof(Name)),
                    Data.AddedCount,
                    Data.RemovedCount,
                    Data.RaisedCount,
                    Data.Lowered,
                    Data.UnchangedCount);
#endif
        }
        Total += Data;
    }
    LogInfo(L"Added:    %-6d @ %s",      Total.AddedCount, GetCoinString(Total.AddedValue));
    LogInfo(L"Removed   %-6d @ %s",   Total.RemovedCount, GetCoinString(Total.RemovedValue));
    LogInfo(L"Raised    %-6d @ %s",   Total.RaisedCount, GetCoinString(Total.RaisedValueDelta));
    LogInfo(L"Lowered   %-6d @ ",      Total.Lowered);
    LogInfo(L"Unchanged %-6d @ %s",   Total.UnchangedCount, GetCoinString(Total.UnchangedValue));
}

////////////////////////////////////////////////////////////////////////////////

} // namespace DiffDb

////////////////////////////////////////////////////////////////////////////////
