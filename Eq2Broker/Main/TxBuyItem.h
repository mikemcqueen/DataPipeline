/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TxBuyItem.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TXBUYITEM_H
#define Include_TXBUYITEM_H

#include "DpHandler_t.h"
#include "BrokerId.h"
#include "UiTypes.h"

class Eq2Broker_t;

namespace Broker
{
namespace Transaction
{
namespace BuyItem
{

/////////////////////////////////////////////////////////////////////////////

    namespace State
    {
        enum : DP::Transaction::State_t
        {
            First                                = DP::Transaction::State::User_First,
            SelectItem                           = First,
            VerifySelected,
            Buy,
            VerifyQuantity,
            GetMoreOrFinish,
        };
    }

/////////////////////////////////////////////////////////////////////////////

    struct Data_t :
        public DP::Transaction::Data_t
    {
        wstring itemName;
        size_t  price;
        size_t  qtyToBuy;
        size_t  qtyAvailable;

        Data_t(
            const wstring&      initItemName,
                  size_t        initPrice,
                  size_t        initQuantity)
        :
            DP::Transaction::Data_t(
                Id::BuyItem,
                sizeof(Data_t)),
            itemName(initItemName),
            price(initPrice),
            qtyToBuy(initQuantity),
            qtyAvailable(0)
        {
        }

    private:

        Data_t();
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
        OnTransactionComplete(
            const DP::Transaction::Data_t& Data) override;

    private:

        Handler_t();
    };

} // BuyItem
} // Transaction
} // Broker

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TXBUYITEM_H

/////////////////////////////////////////////////////////////////////////////
