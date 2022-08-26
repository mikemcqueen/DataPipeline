////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DiffDbFaster_t.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DiffDb_t.h"

////////////////////////////////////////////////////////////////////////////////

namespace DiffDb
{

////////////////////////////////////////////////////////////////////////////////

    class Faster_t :
        public Base_t
    {

    private:

        SellerCountMap_t       m_OldSellers;
        SellerCountMap_t       m_NewSellers;
        RemovedSellers_t       m_RemovedSellers;
        ItemPriceDataMap_t     m_RemovedItemPriceMap;

        ItemPriceDataMap_t*    m_pOldItemPriceMap;
        ItemSellerPriceMap_t*  m_pOldPriceMap;
        ItemPriceDataMap_t*    m_pNewItemPriceMap;
        ItemSellerPriceMap_t*  m_pNewPriceMap;

    public:

        Faster_t();
        ~Faster_t();

    private:

        using Base_t::Load;

        virtual void Init() override;
        virtual bool Load(CDatabase& oldDb, CDatabase& newDb) override; // , MinMaxPrice_t& prices) override;
        virtual void Diff() override;

        void
        ProcessPrices(
            const ItemSellerPair_t&   ItemSellerPair,
                  DiffData_t&         Data,
                  ItemPriceDataMap_t& OldItemPriceMap);

        void
        EliminateCommonPrices(
            const ItemSellerPair_t&   ItemSellerPair,
                  PriceSet_t&         OldPrices,
                  PriceSet_t&         NewPrices,
                  DiffData_t&         Data,
                  ItemPriceDataMap_t& OldItemPriceMap);

        void
        EliminateExactPriceMatches(
            PriceSet_t& OldPrices,
            PriceSet_t& NewPrices,
            DiffData_t& Data);

        void
        EliminateRaisedLoweredPrices(
            const ItemSellerPair_t&   ItemSellerPair,
                  PriceSet_t&         OldPrices,
                  PriceSet_t&         NewPrices,
                  DiffData_t&         Data,
                  ItemPriceDataMap_t& OldItemPriceMap);

        void
        ProcessRemainingPrices(
            const ItemSellerPair_t& ItemSellerPair,
                  PriceSet_t&       OldPrices,
                  DiffData_t&       Data);

        void
        ValidateSalePrices(
            const ItemSellerPair_t&     ItemSellerPair,
            const PriceSet_t&           Prices,
                  ItemSellerPriceMap_t& PriceMap);

        void
        Dump(
            const ItemDataMap_t& Map) const;
    };

////////////////////////////////////////////////////////////////////////////////

} // namespace DiffDb

////////////////////////////////////////////////////////////////////////////////
