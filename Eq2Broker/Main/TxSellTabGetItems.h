/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TxSellTabGetItems.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TXSELLTABGETITEMS_H
#define Include_TXSELLTABGETITEMS_H

#include "DpHandler_t.h"
#include "BrokerId.h"
#include "TiBrokerSell.h"
#include "BrokerSellTypes.h"
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

namespace SellTabGetItems
{

/////////////////////////////////////////////////////////////////////////////

    static const DP::MessageId_t  kTransactionId  = Id::SellTabGetItems;
    static const DP::MessageId_t  kMessageId      = Message::Id::Sell;

#if 0
    struct ItemData_t
    {
        Sell::Text_t::Row_t sellTextRow;
    };
    typedef std::vector<ItemData_t> ItemVector_t;
#endif

/////////////////////////////////////////////////////////////////////////////

    struct Data_t;

    class Handler_t :
        public DP::Handler_t
    {
    private:

        Eq2Broker_t&    m_broker;
        Sell::TiTable_t m_tiTable;

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
            const DP::Transaction::Data_t& Data) override;

        HRESULT
        OnTransactionComplete(
            const DP::Transaction::Data_t& Data) override;

    private:

              Sell::TiTable_t& GetTiTable()       { return m_tiTable; }
        const Sell::TiTable_t& GetTiTable() const { return m_tiTable; }

        void
        GetSellItems(
            const Sell::Translate::Data_t& Message,
                  Data_t&                  TxData);

        void
        CompareSellText(
            const Sell::Text_t& Text,
                  Data_t&       TxData);

        void
        Scroll(
            Ui::Scroll::Direction_t direction) const;

        void
        SetItemsForSale(
            const Sell::Text_t&  text,
                  ItemDataMap_t& itemDataMap);

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

    typedef BaseGetItems::Data_t<Sell::Text_t, ItemDataMap_t> Base_t;

//#pragma warning(push)
//#pragma warning(disable:4239)

    struct Data_t :
        public Base_t
    {
        bool           hasScrolledToTop;

        Data_t(
            ItemDataMap_t&      itemDataMap,
            DP::TransactionId_t id     = kTransactionId,
            size_t              size   = sizeof(Data_t))
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

        void
        Init()
        {
            hasScrolledToTop = false;
        }
    };

//#pragma warning(pop)

/////////////////////////////////////////////////////////////////////////////

} // SellTabGetItems
} // Transaction
} // Broker

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TXSELLTABGETITEMS_H

/////////////////////////////////////////////////////////////////////////////
