/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TxBuySellItems.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TXBUYSELLITEMS_H
#define Include_TXBUYSELLITEMS_H

#include "DpHandler_t.h"
#include "BrokerId.h"
#include "UiTypes.h"
#include "BrokerBuyTypes.h"
#include "StringSet_t.h"
#include "TxSellTabGetItems.h"
#include "Character_t.h"

class Eq2Broker_t;
//class TableWindow_t;

namespace Broker
{
namespace Transaction
{
namespace BuySellItems
{

/////////////////////////////////////////////////////////////////////////////

    namespace State
    {
        enum : DP::Transaction::State_t
        {
            First                      = DP::Transaction::State::User_First,
            FirstGotoSellTab           = First,
            SellTabGetItems,
            GotoBuyTab,
            GetItemPrices,
            SecondGotoSellTab,
            RepriceItems,
            Done,
        };
    }

    struct Data_t :
        public DP::Transaction::Data_t
    {
        StringSet_t   itemNames;
        wstring       itemName;
        ItemDataMap_t myItemMap;            // my items for sale via TxSellTabGetItems
        ItemDataMap_t buySellItemMap;       // combo of my items + Char.itemsToBuySell via TxGetItemPrices
        bool          hasScrolledToTop;

        Data_t():
            DP::Transaction::Data_t(
                Id::BuySellItems,
                sizeof(Data_t)) {}
    };

/////////////////////////////////////////////////////////////////////////////

    class Handler_t final :
        public DP::Handler_t
    {
    private:

        Eq2Broker_t& m_broker;

    public:

        Handler_t(
            Eq2Broker_t& broker);

        //
        // DP::Handler_t virtual
        //

        HRESULT
            MessageHandler(
                const DP::Message::Data_t* pData) override;

        HRESULT
        ExecuteTransaction(
            DP::Transaction::Data_t& Data) override;

        HRESULT
            ResumeTransaction(
                DP::Transaction::Data_t& txData,
                const DP::Transaction::Data_t* pPrevTxData) override;

        HRESULT
        OnTransactionComplete(
            const DP::Transaction::Data_t& Data) override;

        // Implementation:
 
        void
        InitItemNames(
            StringSet_t&            itemNames,
            const ItemDataMap_t&    itemMap,
            const ItemBuySellMap_t& buySellMap) const;

    private:

        Handler_t();
    };

/////////////////////////////////////////////////////////////////////////////

} // BuySellItems
} // Transaction
} // Broker

#endif // Include_TXBUYSELLITEMS_H

/////////////////////////////////////////////////////////////////////////////
