////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TxSellTabGetItems.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PipelineManager.h"
#include "TransactionManager.h"
#include "TxSellTabGetItems.h"
#include "DcrBrokerSell.h"
#include "BrokerSellTypes.h"
#include "UiEvent.h"
#include "MainWindow_t.h"
#include "TiBrokerSell.h"
#include "TxSetActiveWindow.h"
#include "Eq2Broker_t.h"
#include "Character_t.h"
#include "DbItems_t.h"
#include "Price_t.h"

namespace Broker
{
namespace Transaction
{
namespace SellTabGetItems
{

////////////////////////////////////////////////////////////////////////////////

Handler_t::
Handler_t(
    Eq2Broker_t& broker)
:
    m_broker(broker),
    m_tiTable(Sell::Table::CharColumnWidths, Sell::Table::ColumnCount)
{
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
ExecuteTransaction(
    DP::Transaction::Data_t& txData)
{
    LogInfo(L"TxSellTabGetItems::ExecuteTransaction()");
    static_cast<Data_t&>(txData).Init();
    GetTiTable().ClearText();
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
OnTransactionComplete(
    const DP::Transaction::Data_t&)
{
    LogInfo(L"TxSellTabGetItems::TransactionComplete()");
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
MessageHandler(
    const DP::Message::Data_t* pMessage)
{
    LogInfo(L"TxSellTabGetItems::MessageHandler()");
    DP::TransactionManager_t::AutoRelease_t arTxData(GetTransactionManager().Acquire());
    DP::Transaction::Data_t* pTxData = arTxData.get();
    if (nullptr != pTxData)
    {
        Data_t& txData = static_cast<Data_t&>(*pTxData);
        if (Message::Id::Sell != pMessage->Id)
        {
#if 0
            DP::Transaction::Data_t* pTxSetWindow =
                new SetActiveWindow::Data_t(Window::Id::BrokerSellTab);
            GetTransactionManager().ExecuteTransaction(pTxSetWindow, true);
#else
            txData.Complete(Error::InvalidWindow);
#endif
            return S_FALSE;
        }
        using SellTabGetItems::Data_t;
        GetSellItems(static_cast<const Sell::Translate::Data_t&>(*pMessage), txData);
        return S_OK;
    }
    throw logic_error("TxSellTabGetItems::MessageHandler(): No transaction active");
}

////////////////////////////////////////////////////////////////////////////////

void
Handler_t::
GetSellItems(
    const Sell::Translate::Data_t& Message,
          Data_t&                  TxData)
{
#if EXTRALOG
    LogAlways(L"TxSellTabGetItems::GetSellItems() ScrollPos(%d)", Message.VScrollPos);
#endif
    // First, scroll to top.
    using Sell::Interpret::Handler_t;
    if (!TxData.hasScrolledToTop && !Handler_t::IsScrolledToTop(Message))
    {
        Scroll(Ui::Scroll::Direction::Up);
    }
    else
    {
        TxData.hasScrolledToTop = true;
        CompareSellText(Message.Text, TxData);
        // Scroll to bottom
        if (!Handler_t::IsScrolledToBottom(Message))
        {
            Scroll(Ui::Scroll::Direction::Down);
        }
        else
        {
            // Done.
            GetTransactionManager().CompleteTransaction(TxData.Id);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void
Handler_t::
CompareSellText(
    const Sell::Text_t& text,
          Data_t&       txData)
{
    size_t firstNewRow;
    if (GetTiTable().IsNewText(text, firstNewRow))
    {
        ASSERT(text.GetEndRow() > firstNewRow);
        wstring dummy;
        for (size_t row = firstNewRow; text.GetEndRow() > row; ++row)
        {
#if EXTRALOG
            Text.DumpRow(row, L"New");
#endif
//            ItemData_t itemData;
//            memcpy(itemData.sellTextRow, Text.GetRow(Row), sizeof(itemData.sellTextRow));
//            TxData.itemVector.push_back(itemData);
            txData.fnAddRow(text.GetRow(row), txData.itemName, txData.param);
        }
        GetTiTable().SetText(text);
    }
    else
    {
        LogAlways(L"TxSellTabGetItems::CompareSellText() SameText");
    }
}

////////////////////////////////////////////////////////////////////////////////

void
Handler_t::
Scroll(
    Ui::Scroll::Direction_t direction) const
{
    using namespace Ui::Scroll::Direction;
#if EXTRALOG
    LogAlways(L"TxSellTabGetItems::Scroll(%s)", (Up == direction) ? L"Up" : L"Down");
#endif
    m_broker.GetWindow(Window::Id::BrokerSellTab).Scroll(direction);
}

////////////////////////////////////////////////////////////////////////////////

/*static*/
bool
Handler_t::
AddRow(
   const wchar_t* pTextRow,
   const wstring& /*itemName*/,
   ItemDataMap_t& itemDataMap)
{
    using namespace Sell::Table;
    Sell::Text_t text(CharColumnWidths, ColumnCount);
    size_t Price = 0;
    Price_t parser;
    if (parser.Parse(&pTextRow[text.GetColumnOffset(PriceColumn)]))
    {
        Price = parser.GetPrice();
    }
//    if (0 != Price)
    {
        const wstring itemName(&pTextRow[text.GetColumnOffset(ItemNameColumn)]);
        size_t Quantity = _wtol(&pTextRow[text.GetColumnOffset(QuantityColumn)]);
        if (0 == Quantity)
        {
            Quantity = 1;
        }
        auto [itMap, _] = itemDataMap.insert(make_pair(itemName, PriceCountMap_t()));
        auto [pq, pqInserted] = itMap->second.insert(make_pair(Price, Quantity));
        if (!pqInserted)
        {
            pq->second += Quantity;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

} // SellTabGetItems
} // Transaction
} // Broker

////////////////////////////////////////////////////////////////////////////////
