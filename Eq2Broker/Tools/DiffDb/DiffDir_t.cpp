////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DiffDir_t.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DiffDb.h"
#include "DiffDbFaster_t.h"
#include "Log_t.h"
#include "Log.h"
//#include "Db.h"
#include "Timer_t.h"
#include "DbItems_t.h"
#include "DiffDbTypes.h"
#include "DiffDbUtil.h"
#include "ProgramOptions_t.h"
#include "DiffDir_t.h"
#include "DbItems_t.h"

////////////////////////////////////////////////////////////////////////////////

namespace DiffDb
{

////////////////////////////////////////////////////////////////////////////////

void
DiffDir_t::
Diff(
    const DbFiles_t& dbFiles,
          int        threadCount)
{
    Timer_t Timer(L"DiffDir_t::Diff()");
    Result_t result;
    result.threadData.resize(threadCount);
    DiffFiles(dbFiles, result);
    WaitForAllThreads(result);
    Timer.Show();
    ItemCoalesceMap_t itemMap;
    itemMap.Populate(result.threadData);
    if (PO::GetOption<bool>(Flag::g_szRemoveHighSales))
    {
        itemMap.RemoveHighSales();
    }        
    DumpSaleData(result.threadData, itemMap);
/*
    for(int thread = 0; thread < threadCount; ++thread)
    {
        delete result.threadData[thread].pDiff;
    }
*/
}

////////////////////////////////////////////////////////////////////////////////

void
DiffDir_t::
WaitForAllThreads(
    Result_t& result)
{
    if (!result.threadVector.empty())
    {
        WaitForMultipleObjects(result.threadVector.size(), &result.threadVector[0], TRUE, INFINITE);
        while (!result.threadVector.empty())
        {
            DWORD exitCode = 0;
            GetExitCodeThread(result.threadVector.back(), &exitCode);
            result.threadVector.pop_back();
            if (0 != exitCode)
            {
                LogError(L"Thread returned exitCode (%d)", exitCode);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void
DiffDir_t::
DiffFiles(
    const DbFiles_t& dbFiles,
          Result_t&  result)
{
    const size_t diffCount = dbFiles.size() - 1;
    size_t threadCount = result.threadData.size();
    size_t chunkSize = diffCount / threadCount;
    size_t chunkExtra = diffCount % threadCount;
    if (chunkExtra > 0)
    {
        ++chunkSize;
    }
    size_t startPos = 0;
    size_t curSize = 0;
    size_t curThread = 0;
    DbFiles_t::const_iterator itBegin = dbFiles.begin();
    DbFiles_t::const_iterator itEnd = itBegin;
    for (++itEnd; dbFiles.end() != itEnd; ++itEnd)
    {
        if (++curSize == chunkSize)
        {
            LogInfo(L"Chunk %d:%d", startPos, startPos + curSize);
            ThreadData_t& td = result.threadData[curThread++];
            td.itBegin = itBegin;
            td.itEnd = itEnd;
            ++td.itEnd;
            if (1 == threadCount)
            {
                DoDiff(td, false);
                // i.e.: enforce 'we're ending now'
                if (dbFiles.end() != td.itEnd)
                {
                    throw logic_error("DiffDbDir(): DbFiles.end() != td.itEnd");
                }
            }
            else
            {
                result.threadVector.push_back(DoDiff(td, true));
                startPos = curSize;
                curSize = 0;
                itBegin = itEnd;
                if ((0 < chunkExtra) && (0 == --chunkExtra))
                {
                    --chunkSize;
                }
                --threadCount;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

HANDLE
DiffDir_t::
DoDiff(
    ThreadData_t& data,
    bool   bCreateThread)
{
    data.spDiff.reset(new DiffDb::Faster_t);
    if (bCreateThread)
    {
        LogInfo(L"********** CreateThread *************");
        DWORD Id;
        return util::CreateThread(0, 0, DiffThreadProc, &data, 0, &Id);
    }
    LogInfo(L"********** DiffExecute *************");
    DiffExecute(&data);
    LogInfo(L"********** DiffExecute Done *************");
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

DWORD
WINAPI
DiffDir_t::
DiffThreadProc(
    LPVOID pvParam)
{
    ThreadData_t* pData = reinterpret_cast<ThreadData_t*>(pvParam);
    DWORD retVal = DiffDir_t::DiffExecute(pData);
    LogInfo(L"********** Thread Done *************");
    return retVal;
}

////////////////////////////////////////////////////////////////////////////////

DWORD
DiffDir_t::
DiffExecute(
    ThreadData_t* pData)
{
    DiffDb_t* pDiff = pData->spDiff.get();
    DbFiles_t::const_iterator itFirst = pData->itBegin;
    wstring firstFilename(itFirst->second);
    DbFiles_t::const_iterator itSecond = itFirst;
    for (++itSecond; pData->itEnd != itSecond; ++itSecond)
    {
        pDiff->SetTimes(itFirst->first, itSecond->first);
        pDiff->Diff(firstFilename, itSecond->second);
        firstFilename.clear();
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

void
DiffDir_t::
DumpSaleData(
    const ThreadDataVector_t& threadData,
    const ItemCoalesceMap_t&  itemMap)
{
    CountItemMap_t countMap;
    const size_t highestCount = countMap.Populate(threadData, itemMap);
    const size_t avgSaleMin   = PO::GetOption<size_t>("avgsalemin");
    const size_t avgSaleMax   = PO::GetOption<size_t>("avgsalemax");
    const size_t minSaleCount = PO::GetOption<size_t>("salecountmin");
    const size_t minCount     = PO::GetOption<size_t>("countmin");

    size_t itemCount = 0;
    size_t totalValue = 0;
    size_t totalSaleCount = 0;
    size_t hiloItemCount = 0;
    size_t hiloSaleCount = 0;
    size_t hiloValue = 0;
    size_t netSales = 0;
    DumpHiloHeader();
    // iterate over countmap starting at highest count
    for (size_t count = highestCount; minCount <= count; --count)
    {
        // get range of SaleDataVectors that have a 'count' equal to the current count
        pair<CountItemMap_t::iterator, CountItemMap_t::iterator>
            rangePair = countMap.equal_range(count);
        for (; rangePair.first != rangePair.second; ++rangePair.first)
        {
            const ItemId_t itemId = rangePair.first->second;
            ItemCoalesceMap_t::const_iterator itItem = itemMap.find(itemId);
            if (itemMap.end() == itItem)
            {
                throw logic_error("DumpSaleData(): itemMap.end() == itItem");
            }
            const TimeSaleVectorPtrMap_t& saleVectorMap = itItem->second;
            const size_t saleCount = saleVectorMap.GetSaleCount();
            const size_t grossSales = saleVectorMap.GetGrossSales();
            const size_t avgSale = grossSales / saleCount;
            totalSaleCount += saleCount;
            totalValue += grossSales;
            ++itemCount;

            // determine if hilo data should be displayed
            // item may need to have been sold a certain number of times
            if ((0 < minSaleCount) && (minSaleCount > saleCount))
            {
                continue;
            }
            // the average sale may need to be within a certain range
            if ((avgSale < avgSaleMin) || ((avgSaleMax > 0) && (avgSale > avgSaleMax)))
            {
                continue;
            }

            netSales += DumpHiLoData(itemId, threadData, itemMap);

            ++hiloItemCount;
            hiloSaleCount += saleCount; 
            hiloValue += grossSales;
        }
    }
    LogInfo(L"ItemCount(%d) SaleCount(%d) TotalValue(%ls)",
        itemCount, totalSaleCount, GetCoinString(totalValue));
    LogInfo(L"HiloItemCount(%d) HiLoSaleCount(%d) HiloTotalValue(%ls)",
        hiloItemCount, hiloSaleCount, GetCoinString(hiloValue));
    LogInfo(L"Net sales(%ls)", GetCoinString(netSales));
}

////////////////////////////////////////////////////////////////////////////////

/* static */
void
DiffDir_t::
DumpHiloHeader()
{
    if (PO::GetFlags().Test(Flag::DumpHiLo))
    {
        // name, date, volume sold, high sold, high total, low total, low sold,
        // avg sale, volume total
        wprintf(L"Item,Date,Volume Sold,High Sale,High Total,Low Total,Low Sale,"
            L"Avg Sale,Volume Total,Avg Sale Overall,Net Sales\n");
    }
}

////////////////////////////////////////////////////////////////////////////////

size_t
DiffDir_t::
DumpHiLoData(
          ItemId_t         itemId,
    const ThreadDataVector_t&  threadData,
    const ItemCoalesceMap_t&   itemMap) const
{
    ItemCoalesceMap_t::const_iterator itMap = itemMap.find(itemId);
    if (itemMap.end() == itMap)
    {
        throw logic_error("DumpHiLoData(): itemMap.end() == itMap");
    }
    const TimeSaleVectorPtrMap_t& saleVectorMap = itMap->second;

    // populate the hilo map
    TimeHiLoMap_t hiloMap;
    hiloMap.Populate(itemId, threadData);

    // calculate the overall average sale - the average of all average sales
#if 1
//    TimeHiLoMap_t::MeanAvgSales mas = for_each(hiloMap.begin(), hiloMap.end(), TimeHiLoMap_t::MeanAvgSales());
//    const size_t avgSaleOverall = mas.saleCount;
//    const size_t saleCount = mas.saleCount;
    const size_t avgSaleOverall = saleVectorMap.GetAvgSale();
#else
    const size_t avgSaleOverall = for_each(hiloMap.begin(), hiloMap.end(),
        const size_t saleCount = 0;
#endif

    using namespace Accounts::Db;
    // get item name
    wstring itemName(Items_t::GetItemName(itemId));

#if 0 // DEBUG
    // TODO: all 
    LogInfo(L"%ls - itemMapAvg(%d) count (%d) hiloMapAvg(%d) count(%d)",
        itemName.c_str(),
        saleVectorMap.GetAvgSale(), saleVectorMap.GetSaleCount(),
        avgSaleOverall, saleCount);
#endif

#if 0
    // debug display
    LogInfo(L"DumpHiLoData(): Item(%d) saleVectorMap.size(%d)", itemId, saleVectorMap.size());
    TimeSaleVectorPtrMap_t::const_iterator itSaleVector = saleVectorMap.begin();
    for (; saleVectorMap.end() != itSaleVector; ++itSaleVector)
    {
        struct tm tmTime;
        localtime_s(&tmTime, &itSaleVector->first);
        wchar_t szTime[100];
        wcsftime(szTime, _countof(szTime), L"%d-%b-%Y %H:%M:%S", &tmTime);
        LogInfo(L"  Time(%ls)", szTime);
    }
#endif

    // dump the hi/lo data map
    size_t totalNetSales = 0;
    TimeHiLoMap_t::const_iterator it = hiloMap.begin();
    for (; hiloMap.end() != it; ++it)
    {
        const HiLoData_t& hilo = *it->second;
        if (hilo.Time != it->first)
        {
            LogError(L"DumpHiLoData(): hilo.Time != it->first");
        }
        // find the sale data vector for the current hiloData time
        size_t netSales = 0;
        TimeSaleVectorPtrMap_t::const_iterator itSaleVector = saleVectorMap.find(it->first);
        if (saleVectorMap.end() == itSaleVector)
        {
            throw logic_error("DumpHiLoData(): saleVectorMap.find() failed");
        }
        // calculate net sales using the sale data vector
        const SaleDataVector_t& saleVector = *itSaleVector->second;
        if (saleVector.empty())
        {
            continue;
        }
        netSales = saleVector.GetNetSales(avgSaleOverall);
        totalNetSales += netSales;
        if (PO::GetFlags().Test(Flag::DumpHiLo))
        {
            // name, date, volume sold, high sold, high total, low total, low sold,
            // avg sale, volume total
            // avg sale overall, net sales 
            struct tm tmTime;
            localtime_s(&tmTime, &hilo.Time);
            wchar_t szSellTime[100];
            wcsftime(szSellTime, _countof(szSellTime), L"%d-%b-%Y %H:%M:%S", &tmTime);
            wprintf(L"%ls,%ls,%d"
                L",%d,%d,%d,%d"
                L",%d,%d,%d,%d\n",
                itemName.c_str(), szSellTime, saleVector.size(),
                saleVector.GetMinPrice(), hilo.hiTotal, hilo.loTotal, saleVector.GetMaxPrice(),
                //hilo.avgSold,
                saleVector.GetAvgSale(), hilo.volumeTotal, avgSaleOverall, netSales);
        }
    }
    return totalNetSales;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace DiffDb
