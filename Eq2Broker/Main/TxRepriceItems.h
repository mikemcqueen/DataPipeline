/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// TxRepriceItems.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TXREPRICEITEMS_H
#define Include_TXREPRICEITEMS_H

#include "DpHandler_t.h"
#include "BrokerBuyTypes.h"
#include "BrokerSellTypes.h"
#include "SetPriceTypes.h"
#include "PageNumber_t.h"
#include "BrokerId.h"
#include "UiTypes.h"
#include "TxSellTabGetItems.h"
#include "StringSet_t.h"

class TableWindow_t;

namespace Broker
{
namespace Transaction
{
namespace RepriceItems
{

/////////////////////////////////////////////////////////////////////////////

    namespace State
    {
        enum E : DP::Transaction::State_t
        {
            First                    = DP::Transaction::State::User_First,
/*
            SearchSelectItem         = First,
            SearchValidateSelected,
            SearchClickSearch,
            SearchValidateBuyTab,
            SearchGetItems,
            SearchDoneClickSellTab,
            SearchDoneValidateSellTab,
*/
            SelectItem               = First,
            ValidateSelected,
            ClickSetPrice,
            ValidatePopup,
            EnterPrice,
            ValidatePopupPrice,
            ClosePopup,
            ValidateSellTab,
            ListItem,
        };
    }

/////////////////////////////////////////////////////////////////////////////

    struct Data_t :
        public DP::Transaction::Data_t
    {
        const ItemDataMap_t& brokerItemData;
        StringSet_t   sellItemNames;
        wstring       sellItemName;
        size_t        price;
        bool          hasScrolledToTop;

        Data_t(
            const ItemDataMap_t& itemData)
        :
            DP::Transaction::Data_t(
                Id::RepriceItems,
                sizeof(Data_t)),
            brokerItemData(itemData)
        {
            Init();
        }

        void
        Init();
    };

/////////////////////////////////////////////////////////////////////////////

    class Handler_t :
        public DP::Handler_t
    {
    private:

        MainWindow_t& m_mainWindow;

    public:

        Handler_t(
            MainWindow_t& mainWindow);

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

    private:

        bool
        ValidateMessage(
            const DP::Message::Data_t& Message,
                  Data_t&              TxData) const;

        Ui::Window_t& 
        GetMessageWindow(
            const DP::Message::Data_t& Message) const;

        void
        InitItemNames(
                  Data_t&                  txData,
            const SellTabGetItems::Data_t& sellItemsData);

/*
        void
        InitItemPrices(
                  Data_t&                 txData,
            const BuyTabGetItems::Data_t& buyItemsData);
*/

        bool
        GetSellItemRow(
            const Sell::Translate::Data_t& Message,
                  Data_t&                  TxData,
                  size_t&                  OutRow) const;

        bool
        GetAnySellItemRow(
            const Sell::Translate::Data_t& Message,
                  Data_t&                  txData,
                  size_t&                  OutRow) const;

        void
        ScrollSell(
            Ui::Scroll::Direction_t direction) const;

        bool
        SelectSellItem(
            const Sell::Translate::Data_t& Message,
            Data_t&                  TxData,
            const TableWindow_t&           Window) const;

        size_t
        CalcPrice(
            const Data_t& txData) const;

        bool
        SetItemPrice(
            const SetPrice::Translate::Data_t& Message,
            Data_t&                      TxData,
            const Ui::Window_t&                Window) const;

        void
        EnterNumber(
            const Ui::Window_t& Window,
            size_t        Number) const;

        bool 
        ValidatePrice(
            const SetPrice::Translate::Data_t& Message,
                  Data_t&                      TxData) const;

        void
        ListItem(
            const Sell::Translate::Data_t& Message,
                  Data_t&                      TxData,
            const Ui::Window_t&                Window) const;

        bool
        NextItem(
            const Sell::Translate::Data_t& Message,
                  Data_t&                  TxData) const;

        void
        DumpItems(
            const wchar_t*       pTitle,
            const ItemDataMap_t& items) const;

    private:

        Handler_t();
    };

} // RepriceItems
} // Transaction
} // Broker

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TXREPRICEITEMS_H

/////////////////////////////////////////////////////////////////////////////
