////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TxBuySellItems.cpp
//
// States:
//     FirstGotoSellTab
//     SellTabGetItems
//     GotoBuyTab
//     GetItemPrices
//     SecondGotoSellTab
//     RepriceItems
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PipelineManager.h"
#include "TransactionManager.h"
#include "TxBuySellItems.h"
#include "UiEvent.h"
#include "MainWindow_t.h"
#include "TxSetActiveWindow.h"
#include "Eq2Broker_t.h"
#include "Character_t.h"
#include "DbItems_t.h"
#include "BrokerBuyText.h"
#include "TabWindow.h"
#include "DcrBrokerBuy.h"
#include "DcrBrokerSell.h"
#include "TxGetItemPrices.h"
#include "TxRepriceItems.h"

namespace Broker
{
namespace Transaction
{
namespace BuySellItems
{

////////////////////////////////////////////////////////////////////////////////

Handler_t::
Handler_t(
    Eq2Broker_t& broker)
:
    m_broker(broker)
    //,m_tiTable(Buy::Table::CharColumnWidths, Buy::Table::ColumnCount)
{
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
ExecuteTransaction(
    DP::Transaction::Data_t& txData)
{
    LogInfo(L"TxBuySellItems::ExecuteTransaction()");
    txData.SetState(State::First);
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
OnTransactionComplete(
    DP::Transaction::Data_t& data)
{
    LogInfo(L"TxBuySellItems::TransactionComplete()");
    //txData.buySellMap.Dump();
    LogAlways(L"BuySellItems:");
    Data_t& txData = static_cast<Data_t&>(data);
    ItemDataMap_t::const_iterator it = txData.buySellItemMap.begin();
    for (; txData.buySellItemMap.end() != it; ++it)
    {
        LogAlways(L"  (%s)", it->first.c_str());
        size_t total = 0;
        const PriceCountMap_t& priceMap = it->second;
        PriceCountMap_t::const_iterator itPrice = priceMap.begin();
        for (; priceMap.end() != itPrice; ++itPrice)
        {
            LogAlways(L"    (%3d) @ (%s)", itPrice->second, GetCoinString(itPrice->first));
            total += itPrice->second;
        }
        LogAlways(L"    Prices(%d) Items(%d)", priceMap.size(), total);
    }
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
ResumeTransaction(
          DP::Transaction::Data_t& dpData,
    const DP::Transaction::Data_t* pPrevTxData)
{
    using namespace DP::Transaction::Error;

    Data_t& txData = static_cast<Data_t&>(dpData);
    DP::Transaction::Error_t error = pPrevTxData->Error;
    LogAlways(L"TxBuySellItems::ResumeTransaction() Prev(%s) Error(%x)",
              GetPipelineManager().GetTransactionName(pPrevTxData->Id), error);
    if (None == error)
    {
        if ((Id::SellTabGetItems == pPrevTxData->Id) &&
            (State::SellTabGetItems == txData.GetState()))
        {
            Data_t& data = static_cast<Data_t&>(txData);
            InitItemNames(data.itemNames, data.myItemMap,
                          GetCharacter().GetItemsToBuySell());
            if (data.itemNames.empty())
            {
                error = Error::NoItems;
            }
        }
        else if ((Id::GetItemPrices == pPrevTxData->Id) &&
                 (State::GetItemPrices == txData.GetState()))
        {
            if (0 == txData.itemNames.erase(txData.itemName))
            {
                LogError(L"TxBuySellItems::ResumeTransaction() itemNames.erase(%s) failed size(%d)",
                         txData.itemName.c_str(), txData.itemNames.size());
            }
            txData.itemName.clear();
            if (!txData.itemNames.empty())
            {
                // If there are more item names in set, don't call NextState();
                // stay in this state as long as there item names to process
                LogAlways(L"TxBuySellItems::ResumeTransaction() Items remaining(%d)",
                          txData.itemNames.size());
                return S_OK;
            }
        }
    }
#if 0
    // what, if anything, is recoverable from this transaction
    else if ((State::BuyTabGetItems == txData.GetState()) &&
             (Error::InvalidItemName == error))
    {
        txData.SetState(State::BuyTabSearch);
    }
#endif
    // sell tab transaction failed - so what? keep going
    else if (State::SellTabGetItems == txData.GetState())
    {
        error = None;
    }
    if (None == error)
    {
        txData.NextState();
    }
    else
    {
        txData.Complete(error);
    }
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

void
Handler_t::
InitItemNames(
    StringSet_t&            itemNames,
    const ItemDataMap_t&    itemMap,
    const ItemBuySellMap_t& buySellMap) const
{
    itemNames.clear();
    LogAlways(L"TxBuySellItems::InitItemNames() itemMap.size(%d) buySellMap.size(%d)",
              itemMap.size(), buySellMap.size());
    
    for (ItemDataMap_t::const_iterator it = itemMap.begin();
         itemMap.end() != it; ++it)
    {
        LogAlways(L"  itemNames.insert(%s)", it->first.c_str());  
        itemNames.insert(it->first);
    }
    for (ItemBuySellMap_t::const_iterator it = buySellMap.begin();
         buySellMap.end() != it; ++it)
    {
        const wchar_t* pName = Accounts::Db::Items_t::GetItemName(it->first);
        if (NULL != pName)
        {
            itemNames.insert(pName);
            LogAlways(L"  itemNames.insert(%s)", pName);
        }
        else
        {
            LogError(L"TxBuySellItems::InitItemNames() Item(%d) Name is NULL",
                     it->first);
        }
    }
    LogAlways(L"itemNames.size(%d)", itemNames.size());
    if (itemNames.empty())
    {
        LogWarning(L"TxBuySellItems::InitMyItemNames() myItemNames is empty");
    }
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
MessageHandler(
    const DP::Message::Data_t* pMessage)
{
    LogInfo(L"TxBuySellItems::MessageHandler()");
    DP::TransactionManager_t::AutoRelease_t tm(GetTransactionManager().Acquire());
    if (NULL == tm.get())
    {
        throw logic_error("TxBuySellItems::MessageHandler(): No transaction active");
    }
    Data_t& txData = static_cast<Data_t&>(*tm.get());
/*
    if (!ValidateMessage(*pMessage, txData))
    {
        return S_FALSE;
    }
*/
#if 0
    static const size_t kStateTimeoutThreshold = 30;
    if (txData.IncStateTimeout() > kStateTimeoutThreshold)
    {
        if (State::First == txData.GetState())
        {
            GetTransactionManager().CompleteTransaction(txData.Id, DP::Transaction::Error::Timeout);
        }
        else
        {
            LogWarning(L"TxBuySellItems Timeout - going to previous state");
            txData.PrevState();
        }
        return S_OK;
    }
#endif
//    Ui::Window_t& Window = broker.GetWindow(m_mainWindow.GetMessageWindowId(pMessage->Id));
    const Sell::Translate::Data_t&
        sellMessage = static_cast<const Sell::Translate::Data_t&>(*pMessage);
    sellMessage;

    switch (txData.GetState())
    {
    case State::FirstGotoSellTab:
        if (Message::Id::Sell != pMessage->Id)
        {
            DP::Transaction::Data_t* pTxSetWindow =
                new SetActiveWindow::Data_t(Window::Id::BrokerSellTab);
            pTxSetWindow->Execute(true);
        }
        else
        {
            txData.NextState();
        }
        break;
    case State::SellTabGetItems:
        {
            DP::Transaction::Data_t* pTxGetItems =
                new SellTabGetItems::Data_t(txData.myItemMap);
            pTxGetItems->Execute(true);
        }
        break;
    case State::GotoBuyTab:
        if (Message::Id::Buy != pMessage->Id)
        {
            DP::Transaction::Data_t* pTxSetWindow =
                new SetActiveWindow::Data_t(Window::Id::BrokerBuyTab);
            pTxSetWindow->Execute(true);
        }
        else
        {
            txData.NextState();
        }
        break;
    case State::GetItemPrices:
        {
            txData.itemName = *txData.itemNames.begin();
            ItemDataMap_t::_Pairib ibPair = txData.buySellItemMap.insert(
                make_pair(txData.itemName, PriceCountMap_t()));
            DP::Transaction::Data_t* pTxGetItems =
                new GetItemPrices::Data_t(txData.itemName, ibPair.first->second);
            pTxGetItems->Execute(true);
        }
        break;
    case State::SecondGotoSellTab:
        if (Message::Id::Sell != pMessage->Id)
        {
            DP::Transaction::Data_t* pTxSetWindow =
                new SetActiveWindow::Data_t(Window::Id::BrokerSellTab);
            pTxSetWindow->Execute(true);
        }
        else
        {
            txData.NextState();
        }
        break;
    case State::RepriceItems:
        {
            DP::Transaction::Data_t* pTxReprice =
                new RepriceItems::Data_t(txData.buySellItemMap);
            pTxReprice->Execute(true);
        }
        break;
    case State::Done:
        {
            txData.Complete();
        }
        break;
    default:
        throw logic_error("TxBuySellItems::MessageHandler()");
    }
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

} // BuySellItems
} // Transaction
} // Broker

////////////////////////////////////////////////////////////////////////////////
