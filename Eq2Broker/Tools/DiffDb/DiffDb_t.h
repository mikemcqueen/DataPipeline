////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DiffDb_t.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DiffDbTypes.h"
#include "autocs.h"

namespace DiffDb
{
    typedef set<time_t> TimeSet_t;

    struct MinMaxPrice_t
    {
        size_t minPrice;
        size_t maxPrice;

        MinMaxPrice_t() : minPrice(0), maxPrice(0)
        {}

        bool Compare(const MinMaxPrice_t& rhs) const
        {
            return minPrice == rhs.minPrice && maxPrice == rhs.maxPrice;
        }
    };

    class Base_t
    {
    protected:

        CDatabase        m_FirstDb;
        CDatabase        m_SecondDb;
        time_t           m_FirstTime;
        time_t           m_SecondTime;

        ItemSaleVectorMap_t  m_SaleMap;
        ItemHiLoMap_t        m_HiLoMap;
        MatchData_t          m_MatchData;

        CountType_t      m_CountType;

    public:

        static
        CAutoCritSec&
        Base_t::GetCritSec();

        Base_t();
        virtual ~Base_t();

        virtual void Init() = 0;
        virtual bool Load(CDatabase& oldDb, CDatabase& newDb) = 0;
        virtual void Diff() = 0;

        void
        Diff(
            const wstring& firstDbPath,
            const wstring& secondDbPath);

        bool
        Open(
            const wstring& firstDbPath,
            const wstring& secondDbPath);

        void
        Load(
            CDatabase&            Db,
            ItemSellerPriceMap_t& ItemSellerPriceMap,
            ItemPriceDataMap_t*   pItemPriceDataMap,
            MinMaxPrice_t&        minMaxPrice);

        void
        SetTimes(time_t FirstTime, time_t SecondTime)
        {
            m_FirstTime = FirstTime;
            m_SecondTime = SecondTime;
        }

        const time_t& GetSecondTime() const { return m_SecondTime; }

        void
        ValidateRemovedItems(
                  ItemPriceDataMap_t& RemovedItems,
            const ItemPriceDataMap_t& AllItems);

        void
        ValidateRemovedPrices(
                  PriceDataMap_t& RemovedItems,
            const PriceDataMap_t& AllItems,
                  MatchData_t&    MatchData);

        void
        AddSalePrices(
            const ItemSellerPair_t& ItemSellerPair,
            const PriceSet_t&       Prices);

        void
        AddSalePrices(
            const ItemPriceDataMap_t& ItemPriceDataMap);

        void
        AddSaleData(
            const SaleData_t& Data);

        void
        AddHiLoData(
            const ItemPriceDataMap_t& RemovedItemMap,
            const ItemPriceDataMap_t& NewItemMap);

        void
        AddHiLoData(
            const HiLoData_t& Data);

        const ItemSaleVectorMap_t& GetSaleMap() const { return m_SaleMap; }
        const ItemHiLoMap_t& GetHiLoMap() const       { return m_HiLoMap; }
    };


} // namespace DiffDb

////////////////////////////////////////////////////////////////////////////////
