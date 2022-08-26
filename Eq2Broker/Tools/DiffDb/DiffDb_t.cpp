////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DiffDb_t.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DiffDb.h"
#include "DiffDb_t.h"
#include "Db.h"
#include "Log.h"
#include "Macros.h"
#include "DiffDbUtil.h"
#include "ProgramOptions_t.h"

namespace DiffDb
{

using namespace Db;

////////////////////////////////////////////////////////////////////////////////

Base_t::
Base_t() :
    m_CountType(CountType::Default),
    m_FirstTime(0),
    m_SecondTime(0)
{
    GetCritSec();
}

////////////////////////////////////////////////////////////////////////////////

Base_t::
~Base_t()
{
}

////////////////////////////////////////////////////////////////////////////////

CAutoCritSec&
Base_t::GetCritSec()
{
    static CAutoCritSec cs;
    return cs;
}

////////////////////////////////////////////////////////////////////////////////

void
Base_t::
Diff(
    const wstring& firstDbPath,
    const wstring& secondDbPath)
{
    if (Open(firstDbPath, secondDbPath))
    {
        Init();
        if (Load(m_FirstDb, m_SecondDb))
        {
            Diff();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

bool
Base_t::
Open(
    const wstring& firstDbPath,
    const wstring& secondDbPath)
{
    wchar_t firstDbConnect[MAX_PATH];
    wchar_t secondDbConnect[MAX_PATH];

    m_FirstDb.Close();
    if (!firstDbPath.empty())
    {
        if (!m_FirstDb.OpenEx(Db::FormatConnectString(firstDbConnect, _countof(firstDbConnect), firstDbPath.c_str())))
        {
            LogError(L"DiffDb_t::Open(): Can't open first Db '%s'", firstDbPath.c_str());
            return false;
        }
    }
    m_SecondDb.Close();
    if (!m_SecondDb.OpenEx(Db::FormatConnectString(secondDbConnect, _countof(secondDbConnect), secondDbPath.c_str())))
    {
        LogError(L"DiffDb_t::Open(): Can't open second Db '%s'", secondDbPath.c_str());
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

void
Base_t::
Load(
    CDatabase&            Db,
    ItemSellerPriceMap_t& ItemSellerPriceMap,
    ItemPriceDataMap_t*   pItemPriceDataMap,
    MinMaxPrice_t&        minMaxPrice)
{
    // force serialization of reads
    CLock lock(GetCritSec());

    ItemsForSale_t rs(&Db);
    rs.AddField(ItemsForSale_t::Field::ItemId);
    rs.AddField(ItemsForSale_t::Field::SellerId);
    rs.AddField(ItemsForSale_t::Field::Quantity);
    rs.AddField(ItemsForSale_t::Field::Price);
    // TODO: if (!rs.ReadAll())
    if (0 == rs.Open(CRecordset::forwardOnly, NULL, Recordset_t::DefaultReadOnlyFlags))
    {
        throw std::logic_error("DiffDb_t::Load(): rs.Open() failed");
    }
    size_t priceCount = 0;
    minMaxPrice.minPrice = size_t(rs.m_Price);
    size_t lastPrice = 0;
    for (; !rs.IsEOF(); rs.MoveNext())
    {
        const size_t price = size_t(rs.m_Price);
        if (0 == price)
        {
            continue;
        }
        if (price > minMaxPrice.maxPrice)
        {
            minMaxPrice.maxPrice = price;
        }
        lastPrice = price;
        ItemSellerPriceMap.insert(
            make_pair(ItemSellerPair_t(rs.m_ItemId, rs.m_SellerId), price));
        if (NULL != pItemPriceDataMap)
        {
            // TODO: Hacky. rs.m_AddedDate is wrong, though.
            priceCount += pItemPriceDataMap->Add(rs, m_SecondTime);
        }
    }
    size_t ItemPriceDataMapSize = 0;
    if (NULL != pItemPriceDataMap)
    {
        ItemPriceDataMapSize = pItemPriceDataMap->size();
    }
    LogInfo(L"DiffDb_t::Load ItemSellers(%d) ItemCount(%d) priceCount(%d) lo(%d) hi(%d)",
            ItemSellerPriceMap.size(), ItemPriceDataMapSize, priceCount,
            minMaxPrice.minPrice, minMaxPrice.maxPrice);
}

////////////////////////////////////////////////////////////////////////////////
//
// NOTE: May remove items from the supplied PriceMap.
//
////////////////////////////////////////////////////////////////////////////////
void
Base_t::
ValidateRemovedItems(
          ItemPriceDataMap_t& RemovedItems,
    const ItemPriceDataMap_t& AllItems)
{
    m_MatchData.Clear();
    ItemMatchMap_t ItemMatchMap;
    // For each removed item
    ItemPriceDataMap_t::iterator itRemovedItems = RemovedItems.begin();
    for (; RemovedItems.end() != itRemovedItems; ++itRemovedItems)
    {
        const ItemId_t itemId = itRemovedItems->first;
        ItemPriceDataMap_t::const_iterator itAllItems = AllItems.find(itemId);
        if (AllItems.end() == itAllItems)
        {
            throw logic_error("ValidateRemovedItems(): Item not in AllItems");
        }
        const PriceDataMap_t& AllPrices = itAllItems->second;
        PriceDataMap_t& RemovedPrices = itRemovedItems->second;

        // Make sure the removed prices match the lowest of "all prices"
        MatchData_t MatchData;
        ValidateRemovedPrices(RemovedPrices, AllPrices, MatchData);
        if (0 < MatchData.MismatchCount)
        {
            ++MatchData.ItemMismatchCount; // = 1
            ItemMatchMap.insert(make_pair(itemId, MatchData));
        }
        else
        {
            ++MatchData.ItemMatchCount; // = 1
        }
        m_MatchData += MatchData;
    }

#if 0
    ItemMatchMap_t::const_iterator itMatchMap = ItemMatchMap.begin();
    for (; ItemMatchMap.end() != itMatchMap; ++itMatchMap)
    {
        const MatchData_t& Data = itMatchMap->second;
        LogAlways(L"Item (%d) Match (%d) Mismatch (%d)",
            itMatchMap->first, Data.Match, Data.Mismatch);
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////

void
DiffDb_t::
ValidateRemovedPrices(
          PriceDataMap_t& RemovedPrices,
    const PriceDataMap_t& AllPrices,
          MatchData_t&    MatchData)
{
    // For each removed price
    PriceDataMap_t::iterator itRemovedPrice = RemovedPrices.begin();
    PriceDataMap_t::const_iterator itAllPrice = AllPrices.begin();
    for (; (RemovedPrices.end() != itRemovedPrice) && (AllPrices.end() != itAllPrice);)
    {
        // If removed price matches all prices, increment match values
        if (itRemovedPrice->first == itAllPrice->first)
        {
            ++MatchData.MatchCount;
            MatchData.MatchValue += itRemovedPrice->first;

            ++itAllPrice;
            ++itRemovedPrice;
        }
        else
        {
            ++MatchData.MismatchCount;
            MatchData.MismatchValue += itRemovedPrice->first;

            if (!PO::GetFlags().Test(Flag::NoRemoveAllAfterMismatch))
            {
                itRemovedPrice = RemovedPrices.erase(itRemovedPrice);
                while (RemovedPrices.end() != itRemovedPrice)
                {
                    ++MatchData.RemovedAfterMismatchCount;
                    MatchData.RemovedAfterMismatchValue += itRemovedPrice->first;
                    itRemovedPrice = RemovedPrices.erase(itRemovedPrice);
                }
            }
            else
            {
                do
                {
                    ++itAllPrice;
                } while ((AllPrices.end() != itAllPrice) && (itRemovedPrice->first != itAllPrice->first));
                itRemovedPrice = RemovedPrices.erase(itRemovedPrice);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// This function logs an "Item Sold By Seller @ Price".
//
void
Base_t::
AddSalePrices(
    const ItemSellerPair_t& ItemSellerPair,
    const PriceSet_t&       Prices)
{
    SaleData_t Data;
    Data.ItemId = ItemSellerPair.first;
    Data.SellerId = ItemSellerPair.second;
    PriceSet_t::const_iterator it = Prices.begin();
    for (; Prices.end() != it; ++it)
    {
        Data.Price = *it;
        AddSaleData(Data);
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// This function logs an "Item Sold By Seller @ Price".
//
void
Base_t::
AddSalePrices(
    const ItemPriceDataMap_t& ItemPriceDataMap)
{
    ItemPriceDataMap_t::const_iterator itItem = ItemPriceDataMap.begin();
    for (; ItemPriceDataMap.end() != itItem; ++itItem)
    {
        const PriceDataMap_t& PriceDataMap = itItem->second;
        PriceDataMap_t::const_iterator itPrice = PriceDataMap.begin();
        for (; PriceDataMap.end() != itPrice; ++itPrice)
        {
            const ItemPriceData_t& Data = itPrice->second;
            SaleData_t SaleData;
            SaleData.ItemId   = itItem->first;
            SaleData.Price    = itPrice->first;
            SaleData.SellerId = Data.SellerId;
            SaleData.Time     = Data.Time;
            AddSaleData(SaleData);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void
Base_t::
AddSaleData(
    const SaleData_t& Data)
{
    const_cast<SaleData_t&>(Data).Time = m_SecondTime;
//    m_TimeSet.insert(Data.Time);
#if 0
    ItemSaleMap_t::iterator itSaleMap = m_SaleMap.find(Data.ItemId);
    if (m_SaleMap.end() == itSaleMap)
    {
        ItemSaleMap_t::_Pairib pairIB = m_SaleMap.insert(
            ItemSalePair_t(Data.ItemId, SaleDataVector_t()));
        if (!pairIB.second)
        {
            throw std::logic_error("AddSaleData(): m_SaleMap.insert() failed");
        }
        itSaleMap = pairIB.first;
    }
    itSaleMap->second.push_back(Data);
#else
    ItemSaleVectorMap_t::iterator itSaleMap = m_SaleMap.find(Data.ItemId);
    if (m_SaleMap.end() == itSaleMap)
    {
        ItemSaleVectorMap_t::_Pairib pairIB = m_SaleMap.insert(
            ItemSaleVectorMap_t::value_type(Data.ItemId, TimeSaleVectorMap_t()));
        if (!pairIB.second)
        {
            throw std::logic_error("AddSaleData(): m_SaleMap.insert() failed");
        }
        itSaleMap = pairIB.first;
    }
    TimeSaleVectorMap_t& timeMap = itSaleMap->second;
    TimeSaleVectorMap_t::iterator itTimeMap = timeMap.find(Data.Time);
    if (timeMap.end() == itTimeMap)
    {
        TimeSaleVectorMap_t::_Pairib ibTimeMap = timeMap.insert(
            TimeSaleVectorMap_t::value_type(Data.Time, SaleDataVector_t()));
        if (!ibTimeMap.second)
        {
            throw std::logic_error("AddSaleData(): timeMap.insert() failed");
        }
        itTimeMap = ibTimeMap.first;
    }
    SaleDataVector_t& saleVector = itTimeMap->second;
    saleVector.push_back(Data);
#endif
}

////////////////////////////////////////////////////////////////////////////////

void
Base_t::
AddHiLoData(
    const ItemPriceDataMap_t& RemovedItemPriceMap,
    const ItemPriceDataMap_t& NewItemPriceMap)
{
    ItemPriceDataMap_t::const_iterator itRemovedItem = RemovedItemPriceMap.begin();
    for (; RemovedItemPriceMap.end() != itRemovedItem; ++itRemovedItem)
    {
        const ItemId_t ItemId = itRemovedItem->first;

        PriceSet_t RemovedPrices(RemovedItemPriceMap, ItemId); // , false);
        //TODO: empty why? how?
        if (RemovedPrices.empty()) continue; // weird

        // item, date, volume sold, volume total, high total, high sold, low sold, low total
        HiLoData_t hiloData;

        // item
        hiloData.ItemId = ItemId;
        // date
        hiloData.Time = m_SecondTime;

        // volume sold
        hiloData.volumeSold = RemovedPrices.size();
        // volume total
        PriceSet_t NewPrices(NewItemPriceMap, ItemId);
        hiloData.volumeTotal = RemovedPrices.size() + NewPrices.size();

        // high sold
        hiloData.hiSold = *max_element(RemovedPrices.begin(), RemovedPrices.end());
        // high total
        long hiTotal = hiloData.hiSold;
        PriceSet_t::const_iterator itMaxNewPrice = max_element(NewPrices.begin(), NewPrices.end());
        if (NewPrices.end() != itMaxNewPrice)
        {
            if (*itMaxNewPrice > hiTotal)
            {
                hiTotal = *itMaxNewPrice;
            }
        }
        hiloData.hiTotal = hiTotal;

        // low sold
        hiloData.loSold = *min_element(RemovedPrices.begin(), RemovedPrices.end());
        // low total
        long loTotal = hiloData.loSold;
        PriceSet_t::const_iterator itMinNewPrice = min_element(NewPrices.begin(), NewPrices.end());
        if (NewPrices.end() != itMinNewPrice)
        {
            if (*itMinNewPrice < loTotal)
            {
                loTotal = *itMinNewPrice;
            }
        }
        hiloData.loTotal = loTotal;

        // avg sold
        hiloData.avgSold = accumulate(RemovedPrices.begin(), RemovedPrices.end(), 0) / RemovedPrices.size();

        AddHiLoData(hiloData);
    }
}

////////////////////////////////////////////////////////////////////////////////

void
Base_t::
AddHiLoData(
    const HiLoData_t& Data)
{
    ItemHiLoMap_t::iterator itHiLoMap = m_HiLoMap.find(Data.ItemId);
    if (m_HiLoMap.end() == itHiLoMap)
    {
        ItemHiLoMap_t::_Pairib pairIB = m_HiLoMap.insert(
            make_pair(Data.ItemId, HiLoDataVector_t()));
        if (!pairIB.second)
        {
            throw std::logic_error("AddHiLoData(): m_HiLoMap.insert() failed");
        }
        itHiLoMap = pairIB.first;
    }
    itHiLoMap->second.push_back(Data);
}

////////////////////////////////////////////////////////////////////////////////

} // namepsace DiffDb
