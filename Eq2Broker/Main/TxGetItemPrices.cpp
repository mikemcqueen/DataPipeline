/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TxGetItemPrices.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TxGetItemPrices.h"
#include "Eq2Broker_t.h"
#include "PipelineManager.h"
#include "TransactionManager.h"
#include "UiEvent.h"
#include "TxSetActiveWindow.h"
#include "TxSetWidgetText.h"
#include "UiWindow.h"
#include "BrokerUi.h"
#include "DcrBrokerBuy.h"
#include "Character_t.h"
#include "TxBuyItem.h"

namespace Broker
{
namespace Transaction
{
namespace GetItemPrices
{

/////////////////////////////////////////////////////////////////////////////

Handler_t::
Handler_t(
    Eq2Broker_t& broker)
:
    m_broker(broker)
{
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
ExecuteTransaction(
    DP::Transaction::Data_t& txData)
{
    LogInfo(L"TxGetItemPrices::ExecuteTransaction()");
    txData.SetState(State::First);
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
ResumeTransaction(
          DP::Transaction::Data_t& txData,
    const DP::Transaction::Data_t* pPrevTxData)
{
    const DP::Transaction::Error_t error = pPrevTxData->Error;
    if (DP::Transaction::Error::None == error)
    {
        txData.NextState();
    }
    else if ((State::GetItems == txData.GetState()) &&
             (Error::InvalidItemName == error))
    {
        // TODO: this failure loop never exits
        txData.SetState(State::Search);
    }
    else
    {
        txData.Complete(error);
    }
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
OnTransactionComplete(
    const DP::Transaction::Data_t&)
{
    LogInfo(L"TxGetItemPrices::TransactionComplete()");
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
MessageHandler(
    const DP::Message::Data_t* pMessage)
{
    static const Ui::WindowId_t kTopWindowId = Window::Id::BrokerBuyTab;
    LogInfo(L"TxGetItemPrices::MessageHandler()");
    DP::TransactionManager_t::AutoRelease_t TxData(GetTransactionManager().Acquire());
    DP::Transaction::Data_t* pTxData = TxData.get();
    if (nullptr == pTxData)
    {
        throw logic_error("TxGetItemPrices::MessageHandler(): No transaction active");
    }
    using Broker::Transaction::GetItemPrices::Data_t;
    Data_t& txData = static_cast<Data_t&>(*pTxData);
    if (Message::Id::Buy != pMessage->Id)
    {
        txData.Complete(Error::InvalidWindow);
        return S_FALSE;
    }
    switch (txData.GetState())
    {
#if 0
    case State::GotoBuyTab:
        {
            DP::Transaction::Data_t* pTxSetWindow =
                new SetActiveWindow::Data_t(kTopWindowId);
            GetTransactionManager().ExecuteTransaction(pTxSetWindow, true);
            txData.NextState();
        }
        break;

    case State::ValidateBuyTab:
        if (Message::Id::Buy != pMessage->Id)
        {
            // TODO txData.SetError(DP::Transaction::Error::Aborted); - sets state to error and error to aborted
            GetTransactionManager().CompleteTransaction(txData.Id, DP::Transaction::Error::Aborted);
        }
        else
        {
            txData.NextState();
        }
        break;
#endif

    case State::SetItemText:
        {
            wstring text(txData.itemName);
            transform(text.begin(), text.end(), text.begin(), [](wchar_t c) { return std::towlower(c); });
            DP::Transaction::Data_t* pTxSetText =
                new SetWidgetText::Data_t(
                    kTopWindowId,
                    Buy::Widget::Id::SearchEdit,
                    text,
                    Buy::Widget::Id::SearchLabel);
            GetTransactionManager().ExecuteTransaction(pTxSetText, true);
        }
        break;

    case State::Search:
        {
            m_broker.GetWindow(kTopWindowId).ClickWidget(Buy::Widget::Id::FindButton);
            txData.NextState();
        }
        break;

    case State::GetItems:
        {
            DP::Transaction::Data_t* pTxGetItems =
                new Data_t::Base_t(
                    txData.itemName,
                    txData.param,
                    Data_t::FnAddRow_t(&Handler_t::AddRow));
            GetTransactionManager().ExecuteTransaction(pTxGetItems, true);
        }
        break;

    case State::Done:
        {
            GetTransactionManager().CompleteTransaction(txData.Id);
        }
        break;

    default:
        LogError(L"TxGetItemPrices::Message(): Invalid state (%x)", txData.GetState());
        throw logic_error("TxGetItemPrices::Message() invalid state");
    }
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

/*static*/
bool
Handler_t::
AddRow(
       const wchar_t*   pTextRow,
       const wstring&   searchItemName,
       PriceCountMap_t& priceMap)
{
    bool buying = false;
    using namespace Buy::Table;
    Buy::Text_t foundText(CharColumnWidths, ColumnCount);
    const wstring foundItemName(foundText.GetItemName(pTextRow));
    if (!foundItemName.empty())
    {
        if (searchItemName.empty() || (foundItemName == searchItemName))
        {
            wstring sellerName;
            foundText.GetSellerName(pTextRow, sellerName);
            if (GetCharacter().GetName() != sellerName)
            {
                size_t price = foundText.GetPrice(pTextRow);
                size_t quantity = foundText.GetQuantity(pTextRow);
                if ((0 < price) && (0 < quantity))
                {
                    const size_t quantityToBuy = GetCharacter().WantToBuyHowManyAt(foundItemName, price);
                    if (0 < quantityToBuy)
                    {
                        DP::Transaction::Data_t* pTxBuyItem =
                            new BuyItem::Data_t(foundItemName, price, quantityToBuy);
                        GetTransactionManager().ExecuteTransaction(pTxBuyItem, true);
                        GetCharacter().BuyItem(foundItemName, price, quantityToBuy, true);
                        quantity = (quantityToBuy < quantity) ? quantity - quantityToBuy : 0;
                        buying = true;
                    }
                    if (0 < quantity)
                    {
                        auto [pq, pqInserted] = priceMap.insert(make_pair(price, quantity));
                        if (!pqInserted)
                        {
                            pq->second += quantity;
                        }
                    }
                }
            }
        }
        else
        {
            //LogWarning(L"TxGetItemPrices::AddRow(): Desired(%s) On Screen(%s)",
            //txItemName.c_str(), textItemName.c_str());
        }
    }
    return buying;
}

////////////////////////////////////////////////////////////////////////////////

} // GetItemPrices
} // Transaction
} // Broker

/////////////////////////////////////////////////////////////////////////////
