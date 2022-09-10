////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DiffDbTypes.h
//
////////////////////////////////////////////////////////////////////////////////

#ifndef Include_DIFFDBTYPES_H
#define Include_DIFFDBTYPES_H

#include "Db.h"
#include "DbItemsForSale_t.h"
#include "BaseTypes.h"
#include "math.h"
#include "GameId.h"

////////////////////////////////////////////////////////////////////////////////

namespace DiffDb
{
    class PriceSet_t;

    ////////////////////////////////////////////////////////////////////////////////

    struct SaleData_t
    {
        ItemId_t   ItemId;
        SellerId_t SellerId;
        size_t         Price;
        time_t         Time;
    };

    class SaleDataVector_t :
        public vector<SaleData_t>
    {
    public:

        void
        Validate() const
        { }

        size_t
        GetMaxPrice() const
        {
            return max_element(begin(), end(), ComparePrice())->Price;
        }

        size_t
        GetMinPrice() const
        {
            return min_element(begin(), end(), ComparePrice())->Price;
        }

        size_t
        GetAvgSale() const
        {
            return GetGrossSales() / size();
        }

        size_t
        GetSumOfSquares(size_t mean) const
        {
            return for_each(begin(), end(), SumOfSquares_t(mean)).sumOfSquares;
        }

        struct SumOfSquares_t
        {
            size_t sumOfSquares;
            size_t mean;
            SumOfSquares_t(size_t initMean) : mean(initMean), sumOfSquares(0) {}
            void operator()(const SaleData_t& data)
            {
                sumOfSquares += size_t(pow(double(data.Price - mean), 2));
            }
        };

        size_t
        GetNetSales(
            size_t salePrice) const
        {
            return for_each(begin(), end(), NetSales_t(salePrice)).netSales;
        }

        struct NetSales_t
        {
            size_t netSales;
            size_t salePrice;
            NetSales_t(size_t price) : netSales(0), salePrice(price) {}
            void operator()(const SaleData_t& Data)
            {
                if (Data.Price < salePrice)
                {
                    netSales += salePrice - Data.Price;
                }
            }
        };

        size_t
        GetGrossSales() const
        {
            return for_each(begin(), end(), GrossSales_t()).grossSales;
        }

        struct GrossSales_t
        {
            size_t grossSales;
            GrossSales_t() : grossSales(0) {}
            void operator()(const SaleData_t& Data)
            {
                grossSales += Data.Price;
            }
        };

        struct PriceEqualTo
        {
        private:
            size_t Price;
        public:
            PriceEqualTo(size_t initPrice) : Price(initPrice) {}
            bool operator()(const SaleData_t& d1)
            {
                return Price == d1.Price;
            }
        };

        struct ComparePrice
        {
            bool operator()(const SaleData_t& d1, const SaleData_t& d2)
            {
                return d1.Price < d2.Price;
            }
        };
    };

    typedef pair<ItemId_t, SaleDataVector_t>      ItemSalePair_t;
//    typedef map<Db::ItemId_t, SaleDataVector_t>       ItemSaleMap_t;

    ////////////////////////////////////////////////////////////////////////////////

    class DbFiles_t :
        public std::map<time_t, std::wstring>
    {
    public:

        DbFiles_t() {}

        void
        SetDirectory(
            const wstring& directory);

        void
        SetFiles(
            const wstring& file1,
            const wstring& file2);
    };

    ////////////////////////////////////////////////////////////////////////////////

    struct ThreadData_t
    {
        shared_ptr<DiffDb_t> spDiff;
        DbFiles_t::const_iterator itBegin;
        DbFiles_t::const_iterator itEnd;

        ThreadData_t(
            DiffDb_t* pDiff,
            DbFiles_t::const_iterator& itInitBegin,
            DbFiles_t::const_iterator& itInitEnd)
            :
            spDiff(pDiff), itBegin(itInitBegin), itEnd(itInitEnd)
        { }

        ThreadData_t()
        { }
    };

    typedef vector<ThreadData_t> ThreadDataVector_t;

    ////////////////////////////////////////////////////////////////////////////////
    //
    // TimeSaleVectorMap_t
    //
    ////////////////////////////////////////////////////////////////////////////////

    class TimeSaleVectorMap_t :
        public map<time_t, SaleDataVector_t> 
    {
    public:

        TimeSaleVectorMap_t() {}

        size_t
        Count(
            CountType::Type countType,
            size_t          salePrice) const;

        size_t
        GetSaleCount() const
        {
            return for_each(begin(), end(), SaleCount_t()).saleCount;
        }

        struct SaleCount_t
        {
            size_t saleCount;
            SaleCount_t() : saleCount(0) {}
            void operator()(const TimeSaleVectorMap_t::value_type& timeMapPair)
            {
                const SaleDataVector_t& saleVector = timeMapPair.second;
                saleCount += saleVector.size();
            }
        };

        size_t
        GetNetSales(
            size_t salePrice) const
        {
            return for_each(begin(), end(), NetSales_t(salePrice)).netSales;
        }

        struct NetSales_t
        {
            size_t netSales;
            size_t salePrice;
            NetSales_t(size_t price) : netSales(0), salePrice(price) {}
            void operator()(const TimeSaleVectorMap_t::value_type& timeMapPair)
            {
                const SaleDataVector_t& saleVector = timeMapPair.second;
                netSales += saleVector.GetNetSales(salePrice);
            }
        };

        size_t
        GetGrossSales() const
        {
            return for_each(begin(), end(), GrossSales_t()).grossSales;
        }

        struct GrossSales_t
        {
            size_t grossSales;
            GrossSales_t() : grossSales(0) {}
            void operator()(const TimeSaleVectorMap_t::value_type& timeMapPair)
            {
                const SaleDataVector_t& saleVector = timeMapPair.second;
                grossSales += saleVector.GetGrossSales();
            }
        };
    };

    ////////////////////////////////////////////////////////////////////////////////
    //
    // TimeSaleVectorPtrMap_t
    //
    ////////////////////////////////////////////////////////////////////////////////

    class TimeSaleVectorPtrMap_t :
        public map<time_t, const SaleDataVector_t *> 
    {
    public:

        TimeSaleVectorPtrMap_t() {}

        size_t
        GetMaxPrice() const;

        size_t
        GetMinPrice() const;

        size_t
        Remove(
            size_t Price);

        size_t
        Count(
            CountType::Type countType,
            size_t          salePrice) const;

        size_t
        GetAvgSale() const
        {
            return GetGrossSales() / GetSaleCount();
        }

        size_t
        GetStdDev() const
        {
            size_t sumOfSquares = for_each(begin(), end(), SumOfSquares_t(GetAvgSale())).sumOfSquares;
            double meanSquares = double(sumOfSquares) / double(GetSaleCount());
            return size_t(sqrt(meanSquares));
        }

        struct SumOfSquares_t
        {
            size_t sumOfSquares;
            size_t mean;
            SumOfSquares_t(size_t initMean) : mean(initMean), sumOfSquares(0) {}
            void operator()(const value_type& timeMapPair)
            {
                const SaleDataVector_t& saleVector = *timeMapPair.second;
                sumOfSquares += saleVector.GetSumOfSquares(mean);
            }
        };

        size_t
        GetSaleCount() const
        {
            return for_each(begin(), end(), SaleCount_t()).saleCount;
        }

        struct SaleCount_t
        {
            size_t saleCount;
            SaleCount_t() : saleCount(0) {}
            void operator()(const TimeSaleVectorPtrMap_t::value_type& timeMapPair)
            {
                const SaleDataVector_t& saleVector = *timeMapPair.second;
                saleCount += saleVector.size();
            }
        };

        size_t
        GetNetSales(
            size_t salePrice) const
        {
            return for_each(begin(), end(), NetSales_t(salePrice)).netSales;
        }

        struct NetSales_t
        {
            size_t netSales;
            size_t salePrice;
            NetSales_t(size_t price) : netSales(0), salePrice(price) {}
            void operator()(const value_type& timeMapPair)
            {
                const SaleDataVector_t& saleVector = *timeMapPair.second;
                netSales += saleVector.GetNetSales(salePrice);
            }
        };

        size_t
        GetGrossSales() const
        {
            return for_each(begin(), end(), GrossSales_t()).grossSales;
        }

        struct GrossSales_t
        {
            size_t grossSales;
            GrossSales_t() : grossSales(0) {}
            void operator()(const value_type& timeMapPair)
            {
                const SaleDataVector_t& saleVector = *timeMapPair.second;
                grossSales += saleVector.GetGrossSales();
            }
        };
    };

    ////////////////////////////////////////////////////////////////////////////////

    class ItemSaleVectorMap_t : 
        public map<ItemId_t, TimeSaleVectorMap_t>
    {
    public:

        ItemSaleVectorMap_t() {}
    };

    ////////////////////////////////////////////////////////////////////////////////

    class ItemCoalesceMap_t : 
        public map<ItemId_t, TimeSaleVectorPtrMap_t>
    {
    public:

        ItemCoalesceMap_t() {}

        void
        Populate(
            const ThreadDataVector_t& threadData);

        void
        RemoveHighSales();

        void
        RemoveAboveMaxDeviation(
            const TimeSaleVectorPtrMap_t& timeMap,
                  size_t                  count,
                  size_t                  mean,
                  size_t                  stdev,
                  long                    maxDeviation,
                  size_t&                 erasedCount,
                  size_t&                 erasedValue);
    };

    ////////////////////////////////////////////////////////////////////////////////

    class CountItemMap_t :
        public multimap<size_t, ItemId_t>
    {
        typedef std::function<size_t(const TimeSaleVectorMap_t&)> FnCount_t;

    public:

        CountItemMap_t() {}

        size_t
        Populate(
            const ThreadDataVector_t&  threadData,
            const ItemCoalesceMap_t&   itemMap);
    };

    ////////////////////////////////////////////////////////////////////////////////

    struct HiLoData_t
    {
        ItemId_t ItemId;
        time_t       Time;
        size_t       volumeSold;
        long         hiSold;
        long         loSold;
        long         avgSold;
        size_t       volumeTotal;
        long         hiTotal;
        long         loTotal;
        long         avgTotal;
    };

    typedef vector<HiLoData_t>                   HiLoDataVector_t;
    typedef pair<ItemId_t, HiLoDataVector_t> ItemHiLoPair_t;
    typedef map<ItemId_t, HiLoDataVector_t>  ItemHiLoMap_t;

    class TimeHiLoMap_t :
        public map<time_t, const HiLoData_t*>
    {
    public:

        TimeHiLoMap_t() {}

        void
        Populate(
                  ItemId_t        ItemId,
            const ThreadDataVector_t& threadData);

        void
        Populate(
                  ItemId_t         ItemId,
            const ItemSaleVectorMap_t& itemMap);

        struct MeanAvgSales
        {
            size_t sumAvgSales;
            size_t saleCount;
            MeanAvgSales() : saleCount(0), sumAvgSales(0) { }
            void operator()(const value_type& timeDataPair)
            {
                const HiLoData_t* pHiloData = timeDataPair.second;
                sumAvgSales += pHiloData->avgSold;
                saleCount += pHiloData->volumeSold;
            }
            size_t get() const
            {
                return sumAvgSales / saleCount;
            }
        };
    };

    ////////////////////////////////////////////////////////////////////////////////

    class MatchData_t
    {
    public:

        size_t ItemMatchCount;
        size_t ItemMismatchCount;
        size_t MatchCount;
        size_t MatchValue;
        size_t MismatchCount;
        size_t MismatchValue;
        size_t RemovedAfterMismatchCount;
        size_t RemovedAfterMismatchValue;

        MatchData_t()
        {
            Clear();
        }

        void
        Clear()
        {
            SecureZeroMemory(this, sizeof(MatchData_t));
        }

        void
        operator +=(const MatchData_t Data)
        {
            ItemMatchCount += Data.ItemMatchCount;
            ItemMismatchCount += Data.ItemMismatchCount;
            // price counts
            MatchCount += Data.MatchCount;
            MatchValue += Data.MatchValue;
            MismatchCount += Data.MismatchCount;
            MismatchValue += Data.MismatchValue;
            RemovedAfterMismatchCount += Data.RemovedAfterMismatchCount;
            RemovedAfterMismatchValue += Data.RemovedAfterMismatchValue;
        }

        void
        Dump() const;
    };

    typedef pair<ItemId_t, MatchData_t> ItemMatchPair_t;
    typedef map<ItemId_t, MatchData_t>  ItemMatchMap_t;

    ////////////////////////////////////////////////////////////////////////////////

    struct DiffData_t
    {
        size_t AddedCount;
        size_t AddedValue;
        size_t RemovedCount;
        size_t RemovedValue;
        size_t RaisedCount;
        size_t RaisedValueDelta;
        size_t Lowered;
        size_t UnchangedCount;
        size_t UnchangedValue;

        DiffData_t()
        {
            SecureZeroMemory(this, sizeof(DiffData_t));
        }

        void
        operator +=(
            const DiffData_t& Data)
        {
            AddedCount     += Data.AddedCount;
            AddedValue     += Data.AddedValue;
            RemovedCount   += Data.RemovedCount;
            RemovedValue   += Data.RemovedValue;
            RaisedCount    += Data.RaisedCount;
            RaisedValueDelta += Data.RaisedValueDelta;
            Lowered        += Data.Lowered;
            UnchangedCount += Data.UnchangedCount;
            UnchangedValue += Data.UnchangedValue;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////

    typedef std::pair<ItemId_t, DiffData_t>    ItemDataPair_t;

    class ItemDataMap_t :
        public std::map<ItemId_t, DiffData_t>
    {
    public:

        DiffData_t&
        Add(
            ItemId_t ItemId);
    };

    ////////////////////////////////////////////////////////////////////////////////

    typedef std::pair<SellerId_t, size_t>       SellerCountPair_t;
    typedef std::map<SellerId_t, size_t>        SellerCountMap_t;

    typedef std::pair<ItemId_t, SellerId_t> ItemSellerPair_t;

    ////////////////////////////////////////////////////////////////////////////////

    typedef std::pair<ItemSellerPair_t, long>       ItemSellerPricePair_t;

    class ItemSellerPriceMap_t :
        public std::multimap<ItemSellerPair_t, long>
    {
    public:

        ItemSellerPriceMap_t();
    };

    ////////////////////////////////////////////////////////////////////////////////

    class ItemSellerSet_t :
        public std::set<ItemSellerPair_t>
    {
    public:

        void
        AddSellers(
            const ItemSellerPriceMap_t& PriceMap,
                  SellerCountMap_t*     pSellerCountMap);
    };

    ////////////////////////////////////////////////////////////////////////////////
    //
    // Data associated with an item and a price:
    //
    ////////////////////////////////////////////////////////////////////////////////

    struct ItemPriceData_t
    {
        SellerId_t SellerId;
        time_t         Time;
    };

    ////////////////////////////////////////////////////////////////////////////////

    typedef std::pair<long, ItemPriceData_t>         PriceDataPair_t;

    class PriceDataMap_t :
        public std::multimap<long, ItemPriceData_t>
    {
    public:

        PriceDataMap_t();

        void
        Add(
            const PriceDataMap_t& Map);
    };

    ////////////////////////////////////////////////////////////////////////////////

    typedef std::pair<ItemId_t, PriceDataMap_t> ItemPriceMapPair_t;

    class ItemPriceDataMap_t :
        public std::map<ItemId_t, PriceDataMap_t>
    {
    public:

        ItemPriceDataMap_t();

        void
        Add(
            const ItemPriceDataMap_t& Map);

        size_t
        Add(
            const Db::ItemsForSale_t& rs,
            const time_t&             time);

        void
        Add(
            const ItemSellerPair_t& ItemSellerPair,
            const PriceSet_t&       PriceSet,
                  time_t            Time);

        time_t
        Remove(
            const ItemSellerPair_t& ItemSellerPair,
                  long              Price);

        void
        Reprice(
            const ItemSellerPair_t& ItemSellerPair,
                  long              OldPrice,
                  long              NewPrice);
    };

    ////////////////////////////////////////////////////////////////////////////////

    class PriceSet_t :
        public std::multiset<long>
    {
    public:

        PriceSet_t();

        PriceSet_t(
            const ItemSellerPriceMap_t& PriceMap,
            const ItemSellerPair_t&     ItemSellerPair,
                  bool                  bAllowEmpty = true);

        PriceSet_t(
            const ItemPriceDataMap_t& ItemPriceMap,
                  ItemId_t        ItemId,
                  bool                bAllowEmpty = true);

        void
        GetPrices(
            const ItemSellerPriceMap_t& PriceMap,
            const ItemSellerPair_t&     ItemSellerPair,
                  bool                  bAllowEmpty);

        void
        GetPrices(
            const ItemPriceDataMap_t& ItemPriceMap,
                  ItemId_t        ItemId,
                  bool                bAllowEmpty);
    };

    ////////////////////////////////////////////////////////////////////////////////

    struct SellerData_t
    {
        size_t ItemCount;
        size_t ItemValue;
        size_t OneItemCount;
        size_t OneItemValue;

        SellerData_t()
        {
            SecureZeroMemory(this, sizeof(SellerData_t));
        }

        void
        operator +=(const SellerData_t& Data)
        {
            ItemCount    += Data.ItemCount;
            ItemValue    += Data.ItemValue;
            OneItemCount += Data.OneItemCount;
            OneItemValue += Data.OneItemValue;
        }
    };
    typedef std::map<SellerId_t, SellerData_t> SellerDataMap_t;

    class RemovedSellers_t :
        public SellerDataMap_t
    {
    public:

        void
        Add(
                  SellerId_t    SellerId,
                  size_t            ItemCount,
                  size_t            ItemValue,
            const SellerCountMap_t& OldSellers,
            const SellerCountMap_t& NewSellers);

        size_t
        Dump() const;

        struct SumValues
        {
            SellerData_t Data;
            void operator()(const SellerDataMap_t::value_type& vt)
            {
                Data += vt.second;
            }
        };
    };

} // namespace DiffDb

#endif // Include_DIFFDBTYPES_H

////////////////////////////////////////////////////////////////////////////////
