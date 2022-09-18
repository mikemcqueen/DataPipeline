////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TxBaseGetItems.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PipelineManager.h"
#include "TransactionManager.h"
#include "TxBaseGetItems.h"
#include "UiEvent.h"
#include "MainWindow_t.h"
#include "TxSetActiveWindow.h"
#include "Eq2Broker_t.h"
#include "Character_t.h"
#include "DbItems_t.h"
#include "BrokerBuyText.h"
#include "TabWindow.h"
#include "DcrBrokerBuy.h"

namespace Broker
{
namespace Transaction
{
namespace BaseGetItems
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

#if 0
HRESULT
Handler_t::
ExecuteTransaction(
    DP::Transaction::Data_t& /*txData*/)
{
    LogInfo(L"TxBaseGetItems::ExecuteTransaction()");
    //static_cast<Data_t&>(txData).Init();
    //GetTiTable().ClearText();
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
OnTransactionComplete(
    DP::Transaction::Data_t& /*txData*/)
{
    LogInfo(L"TxBaseGetItems::TransactionComplete()");
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
MessageHandler(
    const DP::Message::Data_t* /*pMessage*/)
{
    throw logic_error("TxBaseGetItems::MessageHandler()");
}
#endif

////////////////////////////////////////////////////////////////////////////////

template<class Data_t>
bool
SelectItem(
    const Data_t&  message,
    const wstring& itemName,
    TableWindow_t& window)
{
    size_t row;
    if (GetItemRow(message, itemName, row))
    {
        return window.ClickRow(row);
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

bool
SelectItem(
    const Buy::Translate::Data_t& message,
    const wstring& itemName,
          size_t   price,
    TableWindow_t& window)
{
    using namespace Broker::Buy;
    size_t row;
    if (GetItemRow(message, itemName, price, row))
    {
        return window.ClickRow(row);
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

template<class Data_t>
bool
GetItemRow(
    const Data_t&  message,
    const wstring& itemName,
          size_t&  outRow)
{
    const Data_t::Text_t& text = message.Text;
    const size_t itemNameOffset = text.GetColumnOffset(itemNameColumn);
    for (size_t Row = 0; Row < text.GetEndRow(); ++Row)
    {
        const wchar_t* pRow = &text.GetRow(Row);
        if (itemName == &pRow[itemNameOffset])
        {
            outRow = Row;
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

bool
GetItemRow(
    const Buy::Translate::Data_t&  message,
    const wstring& itemName,
          size_t   price,
          size_t&  outRow)
{
    const Buy::Text_t& text = message.tableText;
    for (size_t Row = 0; Row < text.GetEndRow(); ++Row)
    {
        if ((itemName == text.GetItemName(Row)) &&
            (0 < price) && (long(price) == text.GetPrice(Row)) &&
            (0 < text.GetQuantity(Row)))
        {
            outRow = Row;
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

} // BaseGetItems
} // Transaction
} // Broker

////////////////////////////////////////////////////////////////////////////////
