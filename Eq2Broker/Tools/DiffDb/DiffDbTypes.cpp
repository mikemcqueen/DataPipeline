////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DiffDbTypes.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DiffDbTypes.h"
#include "DiffDbUtil.h"
//#include "Db.h"
#include "DbItemsForSale_t.h"
#include "Log.h"
#include "Macros.h"
#include "DiffDb_t.h"
#include "ProgramOptions_t.h"
#include "DbItems_t.h"

#if _MSC_VER > 1000
#pragma warning(push)
#pragma warning(disable:4245) // signed/unsigned mismatch in boost
#pragma warning(disable:4701) // uninitialized local variable used

#endif

#include "boost/filesystem.hpp"
#include "boost/regex/v4/fileiter.hpp"

#if _MSC_VER > 1000
#pragma warning(pop)
#endif

using namespace Db;

namespace DiffDb
{

////////////////////////////////////////////////////////////////////////////////
//
// DbFiles_t
//
////////////////////////////////////////////////////////////////////////////////

void
DbFiles_t::
SetDirectory(
    const wstring& directory)
{
    namespace fs = boost::filesystem;
    fs::wpath Directory(directory.c_str());
    const fs::wdirectory_iterator itEnd;
    // loop through each file in the directory
    for (fs::wdirectory_iterator it(Directory); itEnd != it; ++it)
    {
        // skip directories
        if (is_directory(it->status()))
        {
            LogInfo(L"  Skipping directory: %ls", it->path().string().c_str());
            continue;
        }
        // skip non-mdb files
        if (extension(it->path()) != L".mdb")
        {
            LogInfo(L"  Skipping non-mdb file: %ls", it->path().string().c_str());
            continue;
        }
        const boost::uintmax_t size = file_size(it->path());
        const boost::uintmax_t minsize = 6000000;
        if (minsize > size)
        {
            LogInfo(L"  Skipping small mdb file (%d bytes): %ls", size, it->path().string().c_str());
            continue;
        }
        const time_t time = last_write_time(it->path());
        DbFiles_t::_Pairib ibPair = insert(value_type(time, it->path().string()));
        if (!ibPair.second)
        {
            throw logic_error("DbFiles_t(): insert it->path failed");
        }
    }
    LogInfo(L"Directory: %ls", Directory.string().c_str());
    for (const_iterator it = begin(); end() != it; ++it)
    {
        LogInfo(L"  File: %ls", it->second.c_str());
    }
}

////////////////////////////////////////////////////////////////////////////////

void
DbFiles_t::
SetFiles(
     const wstring& file1,
     const wstring& file2)
{
    const int time1 = 100;
    const int time2 = 10000;

    DbFiles_t::_Pairib PairIb = insert(value_type(time1, file1));
    if (!PairIb.second)
    {
        throw logic_error("DbFiles_t(): insert file1 failed");
    }
    PairIb = insert(value_type(time2, file2));
    if (!PairIb.second)
    {
        throw logic_error("DbFiles_t(): insert file2 failed");
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// HiLoMap_t
//
////////////////////////////////////////////////////////////////////////////////

void
TimeHiLoMap_t::
Populate(
          ItemId_t        ItemId,
    const ThreadDataVector_t& threadData)
{
    ThreadDataVector_t::const_iterator itThreadData = threadData.begin();
    for (; threadData.end() != itThreadData; ++itThreadData)
    {
        const ItemHiLoMap_t& map = itThreadData->spDiff->GetHiLoMap();
        ItemHiLoMap_t::const_iterator itMap = map.find(ItemId);
        if (map.end() != itMap)
        {
            const HiLoDataVector_t& hiloVector = itMap->second;
            HiLoDataVector_t::const_iterator itHilo = hiloVector.begin();
            for (; hiloVector.end() != itHilo; ++itHilo)
            {
                insert(value_type(itHilo->Time, &*itHilo));
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// ItemCoalesceMap_t
//
////////////////////////////////////////////////////////////////////////////////

void
ItemCoalesceMap_t::
Populate(
    const ThreadDataVector_t& threadData)
{
    // for each thread data (i.e. each thread diff result)
    ThreadDataVector_t::const_iterator itThreadData = threadData.begin();
    for (; threadData.end() != itThreadData; ++itThreadData)
    {
        // for each item sold
        if (!itThreadData->spDiff)
        {
            throw logic_error("ItemCoalesceMap_t::Popuplate() !itThreadData->spDiff");
        }
        const ItemSaleVectorMap_t& itemMap = itThreadData->spDiff->GetSaleMap();
        ItemSaleVectorMap_t::const_iterator itItemMap = itemMap.begin();
        for(; itemMap.end() != itItemMap; ++itItemMap)
        {
            const ItemId_t itemId = itItemMap->first;
            const TimeSaleVectorMap_t& timeMap = itItemMap->second;
            if (timeMap.empty())
            {
                throw logic_error("ItemCoalesceMap_t::Populate(): timeMap.empty()");
            }
            // ensure a TimeSaleVectorPtrMap_t exists for this item
            ItemCoalesceMap_t::iterator itThis = find(itemId);
            if (end() == itThis)
            {
                ItemCoalesceMap_t::_Pairib ibCoalesce = insert(
                    value_type(itemId, TimeSaleVectorPtrMap_t()));
                if (!ibCoalesce.second)
                {
                    throw logic_error("ItemCoalesceMap_t::Populate(): insert() failed");
                }
                itThis = ibCoalesce.first;
            }
            TimeSaleVectorPtrMap_t& thisTimeMap = itThis->second;
            // add an entry to thisTimeMap for each sale vector in timeMap
            TimeSaleVectorMap_t::const_iterator itTimeMap = timeMap.begin();
            for (; timeMap.end() != itTimeMap; ++itTimeMap)
            {
                const SaleDataVector_t& saleVector = itTimeMap->second;
                TimeSaleVectorPtrMap_t::_Pairib ibThisTime = thisTimeMap.insert(
                    TimeSaleVectorPtrMap_t::value_type(itTimeMap->first, &saleVector));
                if (!ibThisTime.second)
                {
                    throw logic_error("ItemCoalesceMap_t::Populate(): thisTimeMap.insert() failed");
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void
ItemCoalesceMap_t::
RemoveHighSales()
{
    const size_t minCount = PO::GetOption<size_t>("countmin");
    const size_t avgSaleMin = PO::GetOption<size_t>("avgsalemin");
    const size_t sigma = PO::GetOption<size_t>("sigma");

    size_t erasedCount = 0;
    size_t erasedValue = 0;
    ItemCoalesceMap_t::iterator itThis = begin();
    for (; end() != itThis; ++itThis)
    {
        TimeSaleVectorPtrMap_t& timeMap = itThis->second;
goto again;
again:
        const size_t mean  = timeMap.GetAvgSale();
        // don't bother processing if we don't meet the low average sale price
        // before trimming potential high prices.
        // more checks could be done here
        if (mean < avgSaleMin)
        {
            continue;
        }

        const size_t count = timeMap.GetSaleCount();
        const size_t stdev = timeMap.GetStdDev();
#if 1 // debug
        size_t max = 0;

        TimeSaleVectorPtrMap_t::const_iterator itTime = timeMap.begin();
        for (; timeMap.end() != itTime; ++itTime)
        {
            const SaleDataVector_t& saleVector = *itTime->second;
            SaleDataVector_t::const_iterator itSaleVector = saleVector.begin();
            for (; saleVector.end() != itSaleVector; ++itSaleVector)
            {
                if (max < itSaleVector->Price)
                {
                    max = itSaleVector->Price;
                }
            }
        }
        using namespace Accounts::Db;
        wstring itemName(Items_t::GetItemName(itThis->first));
        if (minCount <= count)
        {
            LogInfo(L"%ls: count(%d) mean(%d) stdev(%d) max(%d)",
                    itemName.c_str(), count, mean, stdev, max);
        }
#endif
        if (count < minCount)
        {
            continue;
        }
#if 0
        const long maxDeviation = stdev * sigma;
        RemoveAboveMaxDeviation(timeMap, count, mean, stdev, maxDeviation, erasedCount, erasedValue);
#else
        sigma;
        if (stdev < mean)
        {
            continue;
        }
        size_t maxPrice = timeMap.GetMaxPrice();
        size_t removedCount = timeMap.Remove(maxPrice);
        if (0 == removedCount)
        {
            throw logic_error("ItemCoalesceMap_t::RemoveHighSales(): 0 == removedCount");
        }
        erasedCount += removedCount;
        erasedValue += (removedCount * maxPrice);
        goto again;
#endif
    }
    LogInfo(L"Removed (%d) @ (%ls)", erasedCount, GetCoinString(erasedValue));
}

////////////////////////////////////////////////////////////////////////////////

void
ItemCoalesceMap_t::
RemoveAboveMaxDeviation(
    const TimeSaleVectorPtrMap_t& timeMap,
    size_t count,
    size_t mean,
    size_t stdev,
    long maxDeviation,
    size_t& erasedCount,
    size_t& erasedValue)
{
    TimeSaleVectorPtrMap_t::const_iterator itTimeMap = timeMap.begin();
    for (; timeMap.end() != itTimeMap; ++itTimeMap)
    {
        SaleDataVector_t& saleVector = const_cast<SaleDataVector_t&>(*itTimeMap->second);
        SaleDataVector_t::iterator itSaleVector = saleVector.begin();
        for (; saleVector.end() != itSaleVector;)
        {
            const long dev = long(itSaleVector->Price) - long(mean);
            if (dev > maxDeviation)
            {
                ++erasedCount;
                erasedValue += itSaleVector->Price;
                using namespace Accounts::Db;
                wstring itemName(Items_t::GetItemName(itSaleVector->ItemId));
                LogInfo(L"Removed %ls @ (%d) count(%d) mean(%d) stdev(%d) maxDev(%d)",
                        itemName.c_str(), itSaleVector->Price,
                        count, mean, stdev, maxDeviation);
                itSaleVector = saleVector.erase(itSaleVector);
            }
            else
            {
                ++itSaleVector;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// TimeSaleVectorMap_t
//
////////////////////////////////////////////////////////////////////////////////

size_t
TimeSaleVectorMap_t::
Count(
    CountType::Type countType,
    size_t          salePrice) const
{
    switch(countType)
    {
    case CountType::Volume:     return GetSaleCount();
    case CountType::NetSales:   return GetNetSales(salePrice);
    case CountType::GrossSales: return GetGrossSales();
    default:
        throw logic_error("TimeSaleVectorMap_t::Count(): Invalid sort order");
    }
}

////////////////////////////////////////////////////////////////////////////////

size_t
TimeSaleVectorPtrMap_t::
Count(
    CountType::Type countType,
    size_t          salePrice) const
{
    switch(countType)
    {
    case CountType::Volume:     return GetSaleCount();
    case CountType::NetSales:   return GetNetSales(salePrice);
    case CountType::GrossSales: return GetGrossSales();
    default:
        throw logic_error("TimeSaleVectorPtrMap_t::Count(): Invalid sort order");
    }
}

////////////////////////////////////////////////////////////////////////////////

size_t
TimeSaleVectorPtrMap_t::
GetMaxPrice() const
{
    size_t maxPrice = 0;
    TimeSaleVectorPtrMap_t::const_iterator itThis = begin();
    for (; end() != itThis; ++itThis)
    {
        SaleDataVector_t& saleVector = const_cast<SaleDataVector_t&>(*itThis->second);
        if (!saleVector.empty())
        {
            SaleDataVector_t::const_iterator itMaxSale =
                max_element(saleVector.begin(), saleVector.end(), SaleDataVector_t::ComparePrice());
            if (itMaxSale->Price > maxPrice)
            {
                maxPrice = itMaxSale->Price;
            }
        }
    }
    return maxPrice;
}

////////////////////////////////////////////////////////////////////////////////

size_t
TimeSaleVectorPtrMap_t::
GetMinPrice() const
{
    size_t minPrice = 0;
    TimeSaleVectorPtrMap_t::const_iterator itThis = begin();
    for (; end() != itThis; ++itThis)
    {
        SaleDataVector_t& saleVector = const_cast<SaleDataVector_t&>(*itThis->second);
        if (!saleVector.empty())
        {
            SaleDataVector_t::const_iterator itMinSale =
                min_element(saleVector.begin(), saleVector.end(), SaleDataVector_t::ComparePrice());
            if ((0 == minPrice) || (itMinSale->Price < minPrice))
            {
                minPrice = itMinSale->Price;
            }
        }
    }
    return minPrice;
}

////////////////////////////////////////////////////////////////////////////////

size_t
TimeSaleVectorPtrMap_t::
Remove(
    size_t Price)
{
    size_t count = 0;
    TimeSaleVectorPtrMap_t::const_iterator itThis = begin();
    for (; end() != itThis; ++itThis)
    {
        SaleDataVector_t& saleVector = const_cast<SaleDataVector_t&>(*itThis->second);

        SaleDataVector_t::const_iterator itRemove = 
            remove_if(saleVector.begin(), saleVector.end(), SaleDataVector_t::PriceEqualTo(Price));
        if (saleVector.end() != itRemove)
        {
            count += saleVector.end() - itRemove;
        }
        saleVector.erase(itRemove, saleVector.end());
    }
    return count;
}

////////////////////////////////////////////////////////////////////////////////
//
// CountItemMap_t
//
////////////////////////////////////////////////////////////////////////////////

size_t
CountItemMap_t::
Populate(
    const ThreadDataVector_t&  threadData,
    const ItemCoalesceMap_t&   itemMap)
{
    CountType::Type countType = CountType::Type(PO::GetOption<int>("sortorder"));
    size_t highestCount = 0;
    ItemCoalesceMap_t::const_iterator itItemMap = itemMap.begin();
    for(; itemMap.end() != itItemMap; ++itItemMap)
    {
        const ItemId_t itemId = itItemMap->first;
        const TimeSaleVectorPtrMap_t& timeMap = itItemMap->second;
        size_t salePrice = 0;
        // for NetSales sorting, we must first calculate a "salePrice" from which 
        // the 'net' is determined. Using mean of means for now.
        if (CountType::NetSales == countType)
        {
#if 0 
           TimeHiLoMap_t hiloMap;
            hiloMap.Populate(itemId, threadData);
            if (!hiloMap.empty())
            {
                // calculate the overall average sale - the average of all average sales
                const size_t avgSaleOverall = for_each(hiloMap.begin(), hiloMap.end(),
                    TimeHiLoMap_t::MeanAvgSales()).get();

                salePrice = avgSaleOverall;
            }
            else
            {
                throw logic_error("CountItemMap_t::Populate(): HiloMap empty() - wasn't happening before");
            }
#else
            threadData;
            salePrice = timeMap.GetAvgSale();
#endif
        }

        const size_t count = timeMap.Count(countType, salePrice);
        insert(value_type(count, itemId));
        if (count > highestCount)
        {
            highestCount = count;
        }
    }
    return highestCount;
}

////////////////////////////////////////////////////////////////////////////////
//
// MatchData_t
//
////////////////////////////////////////////////////////////////////////////////

void
MatchData_t::
Dump() const
{
    LogInfo(L"    Matched Items (%4d) Prices (%4d) Total Value (%ls)",
        ItemMatchCount, MatchCount, GetCoinString(MatchValue));
    LogInfo(L" Mismatched Items (%4d) Prices (%4d) Total Value (%ls)",
        ItemMismatchCount, MismatchCount, GetCoinString(MismatchValue));
    if (0 < RemovedAfterMismatchCount)
    {
        LogInfo(L"Removed after Mismatched Prices (%4d) Total Value (%ls)",
            RemovedAfterMismatchCount, GetCoinString(RemovedAfterMismatchValue));
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// ItemDataMap_t
//
////////////////////////////////////////////////////////////////////////////////

DiffData_t&
ItemDataMap_t::
Add(
    ItemId_t ItemId)
{
    iterator itFind = find(ItemId);
    if (end() != itFind)
    {
        return itFind->second;
    }
    IbPair_t IbPair = insert(ItemDataPair_t(ItemId, DiffData_t()));
    if (!IbPair.second)
    {
        throw std::logic_error("ItemDataMap_t::Add(): insertion failed");
    }
    return IbPair.first->second;
}

////////////////////////////////////////////////////////////////////////////////
//
// ItemSellerPriceMap_t
//
////////////////////////////////////////////////////////////////////////////////

ItemSellerPriceMap_t::
ItemSellerPriceMap_t()
{
}

////////////////////////////////////////////////////////////////////////////////
// This function iterates through the supplied PriceMap and adds each
// itmem/seller pair to this ItemSellerSet.
// If a SellerCountMap is supplied, an entry for each seller in the PriceMap
// is added to it, with the count set to the number of items sold by that seller.
//

void
ItemSellerSet_t::
AddSellers(
    const ItemSellerPriceMap_t& PriceMap,
          SellerCountMap_t*     pSellerCountMap)
{
    ItemSellerPriceMap_t::const_iterator itPriceMap = PriceMap.begin();
    for (; PriceMap.end() != itPriceMap; ++itPriceMap)
    {
        const ItemSellerPair_t& ItemSellerPair = itPriceMap->first;
        insert(ItemSellerPair);
        if (NULL != pSellerCountMap)
        {
            static const size_t FirstSaleCount = 1;
            SellerCountMap_t::_Pairib
                PairIB = pSellerCountMap->insert(SellerCountPair_t(ItemSellerPair.second, FirstSaleCount));
            if (!PairIB.second)
            {
                ++PairIB.first->second;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// PriceSet_t
//
////////////////////////////////////////////////////////////////////////////////

PriceSet_t::
PriceSet_t()
{
}

////////////////////////////////////////////////////////////////////////////////

PriceSet_t::
PriceSet_t(
    const ItemSellerPriceMap_t& PriceMap,
    const ItemSellerPair_t&     ItemSellerPair,
          bool                  bAllowEmpty /* = true */)
{
    GetPrices(PriceMap, ItemSellerPair, bAllowEmpty);
}

////////////////////////////////////////////////////////////////////////////////

PriceSet_t::
PriceSet_t(
    const ItemPriceDataMap_t& ItemPriceMap,
          ItemId_t        ItemId,
          bool                bAllowEmpty /* = true */)
{
    GetPrices(ItemPriceMap, ItemId, bAllowEmpty);
}

////////////////////////////////////////////////////////////////////////////////

void
PriceSet_t::
GetPrices(
    const ItemSellerPriceMap_t& PriceMap,
    const ItemSellerPair_t&     ItemSellerPair,
          bool                  bAllowEmpty)
{
    pair<ItemSellerPriceMap_t::const_iterator, ItemSellerPriceMap_t::const_iterator>
        RangePair = PriceMap.equal_range(ItemSellerPair);

    if (RangePair.first != RangePair.second)
    {
        for (; RangePair.first != RangePair.second; ++RangePair.first)
        {
            insert(RangePair.first->second);
        }
    }
    else if (!bAllowEmpty)
    {
        throw logic_error("PriceSet_t::GetPrices(): empty set");
    }
}

////////////////////////////////////////////////////////////////////////////////

void
PriceSet_t::
GetPrices(
    const ItemPriceDataMap_t& ItemPriceMap,
          ItemId_t        ItemId,
          bool                bAllowEmpty)
{
    bool bInserted = false;
    ItemPriceDataMap_t::const_iterator itItem = ItemPriceMap.find(ItemId);
    if (ItemPriceMap.end() != itItem)
    {
        const PriceDataMap_t& PriceMap = itItem->second;
        PriceDataMap_t::const_iterator itPrice = PriceMap.begin();
        for (; PriceMap.end() != itPrice; ++itPrice)
        {
            insert(itPrice->first);
            bInserted = true;
        }
    }
    if (!bAllowEmpty && !bInserted)
    {
        throw logic_error("PriceSet_t::GetPrices(): empty set");
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// PriceDataMap_t
//
////////////////////////////////////////////////////////////////////////////////

PriceDataMap_t::
PriceDataMap_t()
{
}

////////////////////////////////////////////////////////////////////////////////

void
PriceDataMap_t::
Add(
    const PriceDataMap_t& Map)
{
    insert(Map.begin(), Map.end());
}

////////////////////////////////////////////////////////////////////////////////
//
// ItemPriceDataMap_t
//
////////////////////////////////////////////////////////////////////////////////

ItemPriceDataMap_t::
ItemPriceDataMap_t()
{
}

////////////////////////////////////////////////////////////////////////////////
//
// Add items from another ItemPriceDataMap to this map.
//

void
ItemPriceDataMap_t::
Add(
    const ItemPriceDataMap_t& Map)
{
    ItemPriceDataMap_t::const_iterator it = Map.begin();
    for (; Map.end() != it; ++it)
    {
        ItemPriceDataMap_t::iterator itFind = find(it->first);
        if (end() == itFind)
        {
            throw logic_error("ItemPriceDataMap_t::Add() Item not found");
        }
        itFind->second.Add(it->second);
    }
}

////////////////////////////////////////////////////////////////////////////////

size_t
ItemPriceDataMap_t::
Add(
    const Db::ItemsForSale_t& rs,
    const time_t&             time)
{
    ItemPriceDataMap_t::iterator it = find(rs.m_ItemId);
    if (end() == it)
    {
        ItemPriceDataMap_t::_Pairib
            PairIB = insert(value_type(rs.m_ItemId, PriceDataMap_t()));
        if (!PairIB.second)
        {
            throw logic_error("ItemPriceDataMap_t::Add(): insert empty failed");
        }
        it = PairIB.first;
    }
    PriceDataMap_t& priceDataMap = it->second;
    ItemPriceData_t Data;
    Data.SellerId = rs.m_SellerId;
    Data.Time = time;
    // NOTE: multi-map so insert always works.
    PriceDataMap_t::value_type priceDataPair(rs.m_Price, Data);
    for (long item = 0; item < rs.m_Quantity; ++item)
    {
        priceDataMap.insert(priceDataPair);
    }
    return size_t(rs.m_Quantity);
}

////////////////////////////////////////////////////////////////////////////////
//
// Add entries for the specified item and prices into this map.
//
void
ItemPriceDataMap_t::
Add(
    const ItemSellerPair_t& ItemSellerPair,
    const PriceSet_t&       PriceSet,
          time_t            Time)
{
    const ItemId_t   ItemId   = ItemSellerPair.first;
    const SellerId_t SellerId = ItemSellerPair.second;
    ItemPriceDataMap_t::iterator itMap = find(ItemId);
    if (end() == itMap)
    {
        // If the supplied item doesn't have a price-data map, add an empty one.
        ItemPriceDataMap_t::_Pairib IbPair = insert(
            ItemPriceMapPair_t(ItemId, PriceDataMap_t()));
        if (!IbPair.second)
        {
            throw logic_error("ItemPriceDataMap_t::Add(): insert() failed");
        }
        itMap = IbPair.first;
    }
    ItemPriceData_t Data;
    Data.Time = Time;
    PriceDataMap_t& PriceDataMap = itMap->second;
    // For each price in supplied pricemap, add an entry into the item's price-data map.
    PriceSet_t::const_iterator itPrice = PriceSet.begin();
    for (; PriceSet.end() != itPrice; ++itPrice)
    {
        Data.SellerId = SellerId;
        PriceDataMap.insert(make_pair(*itPrice, Data));
    }
}

////////////////////////////////////////////////////////////////////////////////

time_t
ItemPriceDataMap_t::
Remove(
    const ItemSellerPair_t& ItemSellerPair,
          long              Price)
{
    ItemPriceDataMap_t::iterator itMap = find(ItemSellerPair.first);
    if (end() == itMap)
    {
        throw logic_error("ItemPriceDataMap_t::Remove(): Item not found");
    }
    PriceDataMap_t& PriceDataMap = itMap->second;
    pair<PriceDataMap_t::iterator, PriceDataMap_t::iterator>
        RangePair = PriceDataMap.equal_range(Price);
    if (RangePair.first == RangePair.second)
    {
        throw logic_error("ItemPriceDataMap_t::Remove(): Price not found or not equal");
    }
    for (; RangePair.first != RangePair.second; ++RangePair.first)
    {
        const ItemPriceData_t& Data = RangePair.first->second;
        if (ItemSellerPair.second == Data.SellerId)
        {
            time_t time = Data.Time;
            PriceDataMap.erase(RangePair.first);
            return time;
        }
    }
    throw logic_error("ItemPriceDataMap_t::Remove(): No price removed");
}

////////////////////////////////////////////////////////////////////////////////

void
ItemPriceDataMap_t::
Reprice(
    const ItemSellerPair_t& ItemSellerPair,
          long              OldPrice,
          long              NewPrice)
{
    time_t Time = Remove(ItemSellerPair, OldPrice);
    PriceSet_t Prices;
    Prices.insert(NewPrice);
    Add(ItemSellerPair, Prices, Time);
}

////////////////////////////////////////////////////////////////////////////////
//
// RemovedSellers_t
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// Do some messy work which involves keeping track of "Removed Sellers",
// and the items and values associated with those Removed Sellers.
// Removed Sellers are sellers which are present in OldSellers but
// not in NewSellers - as such, any potential "Sold Items" from these
// sellers could be bogus - because it's possible all of the items for
// that seller were removed due to inactivity.
//
// TODO: Add new heuristics for determining if a potential sold item is bogus.
//

void
RemovedSellers_t::
Add(
          SellerId_t        SellerId,
          size_t            ItemCount,
          size_t            ItemValue,
    const SellerCountMap_t& OldSellers,
    const SellerCountMap_t& NewSellers)
{
NewSellers;
    SellerData_t NewData;
    NewData.ItemCount = ItemCount;
    NewData.ItemValue = ItemValue;
    // TODO: better heuristics
    // 1) if seller had only 1 item, maybe it was legitimately sold
    // 2) better: was it the cheapest of all items being sold?
    SellerCountMap_t::const_iterator itOldSeller = OldSellers.find(SellerId);
    if (1 == itOldSeller->second)
    {
        NewData.OneItemCount = ItemCount;
        NewData.OneItemValue = ItemValue;
    }
    SellerDataMap_t::_Pairib
        PairIB = insert(SellerDataMap_t::value_type(SellerId, SellerData_t()));
    PairIB.first->second += NewData;
}

////////////////////////////////////////////////////////////////////////////////

size_t
RemovedSellers_t::
Dump() const
{
    SumValues Total = for_each(begin(), end(), SumValues());
    SellerData_t& Data = Total.Data;
    LogInfo(L"RemovedSellers.size(%d) ItemCount(%d) Value(%ls)",
              size(), Data.ItemCount, GetCoinString(Data.ItemValue));
    LogInfo(L"RemovedOneItemSellers(%d) ItemCount(%d) Value(%ls)",
              Data.OneItemCount, Data.OneItemCount, GetCoinString(Data.OneItemValue));
    return Data.ItemValue;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace DiffDb

////////////////////////////////////////////////////////////////////////////////
