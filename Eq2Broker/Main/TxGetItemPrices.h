/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TxGetItemPrices.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TXGETITEMPRICES_H
#define Include_TXGETITEMPRICES_H

#include "DpHandler_t.h"
#include "BrokerId.h"
#include "TxBuyTabGetItems.h"

namespace Broker
{
namespace Transaction
{
namespace GetItemPrices
{

/////////////////////////////////////////////////////////////////////////////

    namespace State
    {
        enum E : DP::Transaction::State_t
        {
            First                      = DP::Transaction::State::User_First,
            SetItemText                = First,
            Search,
            GetItems,
            Done,
        };
    }

    class Handler_t :
        public DP::Handler_t
    {
    private:

        Eq2Broker_t& m_broker;

    public:

        Handler_t(
            Eq2Broker_t& m_broker);

        //
        // DP::Handler_t virtual
        //
        virtual
        HRESULT
        MessageHandler(
            const DP::Message::Data_t* pData) override;

        virtual
        HRESULT
        ExecuteTransaction(
            DP::Transaction::Data_t& Data) override;

        virtual
        HRESULT
        ResumeTransaction(
                  DP::Transaction::Data_t& txData,
            const DP::Transaction::Data_t* pPrevTxData) override;

        virtual
        HRESULT
        OnTransactionComplete(
            DP::Transaction::Data_t& Data) override;

    public:

        static
        bool
        AddRow(
            const wchar_t* pTextRow,
            const wstring& itemName,
            PriceCountMap_t& priceMap);

    private:

        Handler_t();
    };

/////////////////////////////////////////////////////////////////////////////

    struct Data_t :
        public BuyTabGetItems::Data<PriceCountMap_t>
    {
        typedef BuyTabGetItems::Data<PriceCountMap_t> Base_t;

        Data_t(
            const wstring&   itemName,
            PriceCountMap_t& priceMap)
        :
            Base_t(
                itemName,
                priceMap,
                FnAddRow_t(&Handler_t::AddRow),
                Id::GetItemPrices,
                sizeof(Data_t))
        {
            Init();
        }

        void Init()
        {
            Base_t::Init();
            param.clear(); // priceMap
        }

    private:

        Data_t();
    };

} // GetItemPrices
} // Transaction
} // Broker

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TXGETITEMPRICES_H

/////////////////////////////////////////////////////////////////////////////
