/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TxBuyTabGetItems.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TXBUYTABGETITEMS_H
#define Include_TXBUYTABGETITEMS_H

#include "DpHandler_t.h"
#include "BrokerId.h"
#include "TiBrokerBuy.h"
#include "BrokerBuyText.h"
#include "PageNumber_t.h"
#include "UiTypes.h"
#include "TxBaseGetItems.h"

class Eq2Broker_t;

namespace Broker
{
namespace Transaction
{

typedef map<size_t, size_t>                      PriceCountMap_t;
typedef map<wstring, PriceCountMap_t>            ItemDataMap_t;

namespace BuyTabGetItems
{

/////////////////////////////////////////////////////////////////////////////

    static const DP::MessageId_t kTransactionId  = Id::BuyTabGetItems;
    //static const DP::MessageId_t  kMessageId      = Message::Id::Buy;
    //static const Ui::WindowId_t   kTopWindowId    = Window::Id::BrokerBuyTab;

//            s_stringSet.clear();
//    struct Data_t : public 
//    struct Data_t;
    template<class T> struct Data;
    typedef Data<ItemDataMap_t> Data_t;

    class Handler_t :
        public DP::Handler_t
    {
    private:

        Eq2Broker_t&   m_broker;
        Buy::TiTable_t m_tiTable;
        //ItemDataMap_t  m_itemDataMap;

    public:

        Handler_t(
            Eq2Broker_t& broker);

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
        OnTransactionComplete(
            DP::Transaction::Data_t& Data) override;

    private:

              Buy::TiTable_t& GetTiTable()       { return m_tiTable; }
        const Buy::TiTable_t& GetTiTable() const { return m_tiTable; }


        template<class Data_t>
        bool
        AddItemPrices(
            const Buy::Translate::Data_t& Message,
                  Data_t&                 TxData) const;

        void 
        Complete(
            DP::Transaction::Data_t& TxData,
            DP::Transaction::Error_t Error = DP::Transaction::Error::None) const;

        template<class Data_t>
        bool
        GetItemPrices(
            const Buy::Translate::Data_t& Message,
                  Data_t&                 TxData) const;

        bool
        ValidatePageNumbers(
            const PageNumber_t& CurPage,
            const PageNumber_t& PrevPage) const;

        bool
        ValidateMismatch(
                  Data_t&  txData,
            const wstring& foundName) const;

    public:

        static
        bool
        AddRow(
            const wchar_t* pTextRow,
            const wstring& itemName,
            ItemDataMap_t& itemDataMap);

    private:

        Handler_t();
    };

/////////////////////////////////////////////////////////////////////////////

    template<class Param_t>
    struct Data :
        public BaseGetItems::Data_t<Buy::Text_t, Param_t>
    {
        typedef BaseGetItems::Data_t<Buy::Text_t, Param_t> Base_t;

        PageNumber_t pageNumber;

        // Constructor with item name.
        Data(
            const wstring&      itemName,
            Param_t&            param,
            const FnAddRow_t&   fnAddRow         = Base_t::FnAddRow_t(&Handler_t::AddRow),
            DP::TransactionId_t id               = kTransactionId,
            size_t              size             = sizeof(Data_t))
        :
            Base_t(
                itemName,
                fnAddRow,
                param,
                id,
                size)
        {
            Init();
        }

#if 0
        // Constructor without item name.
        Data(
            ItemDataMap_t&      itemDataMap,
            DP::TransactionId_t id               = kTransactionId,
            size_t              size             = sizeof(Data_t))
        :
            Base_t(
                wstring(),
                Base_t::FnAddRow_t(&Handler_t::AddRow),
                itemDataMap,
                id,
                size)
        {
            Init();
        }
#endif

        void
        Init()
        {
            pageNumber.Reset();
        }
    };

/////////////////////////////////////////////////////////////////////////////

} // BuyTabGetItems
} // Transaction
} // Broker

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TxBuyTABGETITEMS_H

/////////////////////////////////////////////////////////////////////////////
