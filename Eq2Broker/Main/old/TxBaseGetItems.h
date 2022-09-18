/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TxBaseGetItems.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TXBASEGETITEMS_H
#define Include_TXBASEGETITEMS_H

#include "DpHandler_t.h"
#include "BrokerId.h"
#include "UiTypes.h"
#include "BrokerBuyTypes.h"

class Eq2Broker_t;
class TableWindow_t;

namespace Broker
{
namespace Transaction
{
namespace BaseGetItems
{

/////////////////////////////////////////////////////////////////////////////

    template<class Text_t>
    struct TextRow_t final
    {
        typename Text_t::Row_t textRow;
    };

#if 1
//    typedef function<bool (const wchar_t*, const wstring&)> FnAddRow_t;
#else
    typedef function<bool (const wstring&, const TextRow_t&)> FnAddRow_t;
#endif

    template<class Text_t, class Param_t>
    struct Data_t :
        public DP::Transaction::Data_t
    {
        typedef TextRow_t<Text_t> TextRow_t;
//        typedef std::function<bool (const wchar_t*, const wstring&, Param_t&)> FnAddRow_t;
        using FnAddRow_t = std::function<bool(const wchar_t*, const wstring&, Param_t&)>;

        const wstring  itemName;
        FnAddRow_t     fnAddRow;
        Param_t&       param;

        Data_t(
            const wstring&      initItemName,
            const FnAddRow_t&   initFnAddRow,
                  Param_t&      initParam,
            DP::TransactionId_t id,
            size_t              size)
        :
            DP::Transaction::Data_t(id, size),
            itemName(initItemName),
            fnAddRow(initFnAddRow),
            param(initParam)
        {
        }

    private:

        Data_t();
    };

/////////////////////////////////////////////////////////////////////////////

    // template<class TiTable_t, class ItemDataMap_t>
    class Handler_t :
        public DP::Handler_t
    {
    private:

        Eq2Broker_t& m_broker;

    public:

        Handler_t(
            Eq2Broker_t& broker);

#if 0
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
#endif

    private:

        Handler_t();
    };

template<class Data_t>
bool
SelectItem(
    const Data_t&  message,
    const wstring& itemName,
    TableWindow_t& window);

template<class Data_t>
bool
GetItemRow(
    const Data_t&  message,
    const wstring& itemName,
          size_t&  outRow);

bool
SelectItem(
    const Buy::Translate::Data_t&  message,
    const wstring& itemName,
          size_t   price,
    TableWindow_t&  window);

bool
GetItemRow(
    const Buy::Translate::Data_t&  message,
    const wstring& itemName,
          size_t   price,
          size_t&  outRow);

} // BaseGetItems
} // Transaction
} // Broker

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TXBASEGETITEMS_H

/////////////////////////////////////////////////////////////////////////////
