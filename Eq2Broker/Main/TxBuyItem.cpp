////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TxBuyItem.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PipelineManager.h"
#include "TransactionManager.h"
#include "TxBuyItem.h"
#include "UiEvent.h"
#include "MainWindow_t.h"
#include "TxSetActiveWindow.h"
#include "Eq2Broker_t.h"
#include "Character_t.h"
#include "DbItems_t.h"
#include "TxBaseGetItems.h"
#include "DcrBrokerBuy.h"

namespace Broker
{
namespace Transaction
{
namespace BuyItem
{

////////////////////////////////////////////////////////////////////////////////

Handler_t::
Handler_t(
    Eq2Broker_t& broker)
:
    m_broker(broker)
{
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
ExecuteTransaction(
    DP::Transaction::Data_t& txData)
{
    LogInfo(L"TxBuyItem::ExecuteTransaction()");
    txData.SetState(State::First);
    //static_cast<Data_t&>(txData).Init();
    //GetTiTable().ClearText();
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
OnTransactionComplete(
    const DP::Transaction::Data_t& txData)
{
    LogInfo(L"TxBuyItem::TransactionComplete()");
    if (DP::Transaction::Error::None != txData.Error)
    {
        // we should call GetCharacter().CancelBuyPending()
        // let character decide what to do (at least log something - maybe
        //  not actually clear from pending as that could result in too muhc
        // stuff being bought.. need to log all of these "errors that fuck
        // with the state of the world" to a central place
        // - need to put qtyRemaining in txData so we can see how many
        // have been bought and not modify qtyToBuy (const?)
    }
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
MessageHandler(
    const DP::Message::Data_t* pMessage)
{
    static const Ui::WindowId_t kTopWindowId = Window::Id::BrokerBuyTab;
    LogInfo(L"TxBuyItem::MessageHandler()");
    DP::TransactionManager_t::AutoRelease_t TxData(GetTransactionManager().Acquire());
    DP::Transaction::Data_t* pTxData = TxData.get();
    if (nullptr == pTxData)
    {
        throw logic_error("TxBuyItem::MessageHandler(): No transaction active");
    }
    using BuyItem::Data_t;
    Data_t& txData = static_cast<Data_t&>(*pTxData);
    if (Message::Id::Buy != pMessage->Id)
    {
        txData.Complete(Error::InvalidWindow);
        return S_FALSE;
    }
    TableWindow_t& tableWindow = static_cast<TableWindow_t&>(
        m_broker.GetWindow(kTopWindowId));
    const Buy::Translate::Data_t&
        buyMessage = static_cast<const Buy::Translate::Data_t&>(*pMessage);

    switch (txData.GetState())
    {
    case State::SelectItem:
        {
            if (BaseGetItems::SelectItem(buyMessage, txData.itemName,
                                         txData.price, tableWindow))
            {
                txData.NextState();
            }
            else
            {
                LogWarning(L"TxBuyItem::SelectItem() failed, itemName(%s) price(%d)",
                           txData.itemName.c_str(), txData.price);
                txData.Complete(Error::ItemNotFound);
            }
        }
        break;
    case State::VerifySelected:
        {
            size_t row;
            if (BaseGetItems::GetItemRow(buyMessage, txData.itemName, txData.price, row))
            {
                if (buyMessage.selectedRow == row + 1)
                {
                    const Buy::Text_t& text = buyMessage.tableText;
                    size_t quantity = text.GetQuantity(row);
                    if (0 < quantity)
                    {
                        txData.qtyAvailable = quantity;
                        txData.NextState();
                    }
                    else
                    {
                        // zero available, try to select another price match
                        txData.PrevState();
                    }
                }
            }
            else
            {
                // can't find it, try to select another price match
                txData.PrevState();
            }
        }
        break;
    case State::Buy:
        {
            m_broker.GetWindow(kTopWindowId).ClickWidget(Buy::Widget::Id::BuyButton);
            txData.NextState();
        }
        break;
    case State::VerifyQuantity:
        if (0 < buyMessage.selectedRow)
        {
            const Buy::Text_t& text = buyMessage.tableText;
            const size_t row = buyMessage.selectedRow - 1;
            const size_t qtyRemaining = text.GetQuantity(row);
            if (qtyRemaining < txData.qtyAvailable)
            {
                txData.qtyAvailable = qtyRemaining;
                txData.NextState();
            }
        }
        break;
    case State::GetMoreOrFinish:
        {
            GetCharacter().BuyItem(txData.itemName, txData.price);
            if (0 < --txData.qtyToBuy)
            {
                if (0 < txData.qtyAvailable)
                {
                    txData.SetState(State::VerifySelected);
                }
                else
                {
                    txData.SetState(State::SelectItem);
                }
            }
            else
            {
                // move the mouse off the buy button for safey
                m_broker.GetWindow(kTopWindowId).ClickWidget(Buy::Widget::Id::PageNumber);
                txData.Complete();
            }
        }
        break;
    default:
        LogError(L"TxBuyItem::Message(): Invalid state (%x)", txData.GetState());
        throw logic_error("TxBuyItem::Message() invalid state");
    }
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

} // BuyItem
} // Transaction
} // Broker

////////////////////////////////////////////////////////////////////////////////
