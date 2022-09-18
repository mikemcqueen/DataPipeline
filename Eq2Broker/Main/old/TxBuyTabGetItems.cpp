////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TxBuyTabGetItems.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PipelineManager.h"
#include "TransactionManager.h"
#include "TxBuyTabGetItems.h"
#include "BrokerBuyTypes.h"
#include "DcrBrokerBuy.h"
#include "UiEvent.h"
#include "MainWindow_t.h"
#include "TiBrokerBuy.h"
#include "TxSetActiveWindow.h"
#include "Eq2Broker_t.h"
#include "Character_t.h"
#include "DbItems_t.h"

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>

namespace Broker
{
namespace Transaction
{
namespace BuyTabGetItems
{

////////////////////////////////////////////////////////////////////////////////

Handler_t::
Handler_t(
    Eq2Broker_t& broker)
:
    m_broker(broker),
    m_tiTable(Buy::Table::CharColumnWidths, Buy::Table::ColumnCount)
{
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
ExecuteTransaction(
    DP::Transaction::Data_t& txData)
{
    LogInfo(L"TxBuyTabGetItems::ExecuteTransaction()");
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
    LogInfo(L"TxBuyTabGetItems::TransactionComplete()");
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
MessageHandler(
    const DP::Message::Data_t* pMessage)
{
    LogInfo(L"TxBuyTabGetItems::MessageHandler()");
    DP::TransactionManager_t::AutoRelease_t arTxData(GetTransactionManager().Acquire());
    DP::Transaction::Data_t* pTxData = arTxData.get();
    if (nullptr != pTxData)
    {
        Data_t& txData = static_cast<Data_t&>(*pTxData);
        if (Message::Id::Buy != pMessage->Id)
        {
            txData.Complete(Error::InvalidWindow);
            return S_FALSE;
        }
        if (GetItemPrices(static_cast<const Buy::Translate::Data_t&>(*pMessage), txData))
        {
            Complete(txData);
        }
        return S_OK;
    }
    throw logic_error("TxBuyTabGetItems::MessageHandler(): No transaction active");
}

////////////////////////////////////////////////////////////////////////////////

bool 
HasSameWords(
    const wstring& s,
    const wstring& d)
{
    wstring src(s);
    transform(src.begin(), src.end(), src.begin(), [](wchar_t c) { return std::towlower(c); });
    wstring dst(d);
    transform(dst.begin(), dst.end(), dst.begin(), [](wchar_t c) { return std::towlower(c); });
    using namespace boost;
    size_t count = 0;
    char_separator<wchar_t> sep(L" ");
    tokenizer<char_separator<wchar_t>, wstring::const_iterator, wstring> tokens(src, sep);
    BOOST_FOREACH(wstring str, tokens)
    {
        if (dst.npos == dst.find(str))
        {
            return false;
        }
        ++count;
    }
    if (0 == count)
    {
        throw logic_error("HasSameWords() Empty vector");
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

template<class Data_t>
bool
Handler_t::
GetItemPrices(
    const Buy::Translate::Data_t& Message,
          Data_t&                 txData) const
{
    static size_t emptyCount = 0;
    static const size_t kEmptyThreshold = 20;
    const Buy::Text_t& Text = Message.tableText;
    if (Text.IsEmpty())
    {
        bool doneWaiting = ++emptyCount > kEmptyThreshold;
        if (doneWaiting)
        {
            emptyCount = 0;
            LogAlways(L"TxBuyTabGetItems::GetItemPrices() No items - done waiting");
        }
        return doneWaiting;
    }
    emptyCount = 0;
    const wstring firstItemName(Text.GetItemName(size_t(0))); // 0 == first row
    const PageNumber_t& CurPage  = Message.pageNumber;
    PageNumber_t&       PrevPage = txData.pageNumber;
    if (ValidatePageNumbers(CurPage, PrevPage))
    {
        if (1 == CurPage.GetPage() && !txData.itemName.empty() &&
            !ValidateMismatch(txData, firstItemName))
        {
            return false;
        }
        bool buying = AddItemPrices(Message, txData);
        txData.pageNumber = CurPage;
        // if AddItemPrices returns true, it means a buy transaction has
        // been executed; don't complete transaction or do any clicking
        if (!buying)
        {
            if (CurPage.GetPage() == CurPage.GetLastPage())
            {
                LogAlways(L"TxBuyTabGetItems::GetItemPrices() done Page(%d)",
                          CurPage.GetPage());
                return true;
            }
            else
            {
                m_broker.GetWindow(Window::Id::BrokerBuyTab).ClickWidget(
                    Buy::Widget::Id::NextButton);
            }
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
ValidateMismatch(
          Data_t&  txData,
    const wstring& foundName) const
{
    static size_t mismatchCount = 0;
    static const size_t kMismatchCheckWordsThreshold = 5;
    static const size_t kMismatchAbortThreshold = 20;
    if (foundName != txData.itemName)
    {
        if (kMismatchCheckWordsThreshold > ++mismatchCount)
        {
            return false;
        }
        else if (!HasSameWords(txData.itemName, foundName))
        {
            if (kMismatchAbortThreshold <= mismatchCount)
            {
                LogError(L"TxBuyTabGetItems::ValidateMismatch() txData.itemName(%s) foundName(%s)",
                         txData.itemName.c_str(), foundName.c_str());
                Complete(txData, Error::InvalidItemName);
                mismatchCount = 0;
            }
            return false;
        }
    }
    mismatchCount = 0;
    return true;
}

////////////////////////////////////////////////////////////////////////////////

template<class Data_t>
bool
Handler_t::
AddItemPrices(
    const Buy::Translate::Data_t& Message,
          Data_t&                 txData) const
{
    bool buying = false;
    using namespace Transaction;
    const Buy::Text_t& Text = Message.tableText;
    for (size_t Row = 0; Row < Text.GetEndRow(); ++Row)
    {
        //TODO: GetCharacter().BuyItem() here
        if (txData.fnAddRow(Text.GetRow(Row), txData.itemName, txData.param))
        {
            buying = true;
        }
    }
    return buying;
}

////////////////////////////////////////////////////////////////////////////////

void 
Handler_t::
Complete(
    DP::Transaction::Data_t& txData,
    DP::Transaction::Error_t Error /*= Error::None*/) const
{
    GetTransactionManager().CompleteTransaction(txData.Id, Error);
}

////////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
ValidatePageNumbers(
    const PageNumber_t& CurPage,
    const PageNumber_t& PrevPage) const
{
    if (!CurPage.IsValid())
    {
        return false;
    }
    if (!PrevPage.IsValid())
    {
        if (1 != CurPage.GetPage())
        {
            LogInfo(L"Waiting for first page..");
            return false;
        }
    }
#if 0
    else if (PrevPage.GetLastPage() != CurPage.GetLastPage())
    {
        if (1 == CurPage.GetPage())
        {
            LogAlways(L"Last page changed - resetting to first page.");
        }
        else
        {
            LogError(L"Last page changed (%d -> %d) at page (%d) - Ignoring...",
                     PrevPage.GetLastPage(), CurPage.GetLastPage(), CurPage.GetPage());
            return false;
        }
    }
#endif
    else
    {
        if (PrevPage.GetPage() == CurPage.GetPage())
        {
            // TODO:  click failure, same page timeout, reclick
            LogInfo(L"Same page - Ignoring...");
            return false;
        }
        else if (PrevPage.GetPage() + 1 != CurPage.GetPage())
        {
            //  allow change to page 1
            if (1 != CurPage.GetPage())
            {
                // TODO: ?
                LogError(L"Skipped a page (%d -> %d) - Ignoring...",
                    PrevPage.GetPage(), CurPage.GetPage());
                return false;
            }
        }
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////

/*static*/
bool
Handler_t::
AddRow(
    const wchar_t* pTextRow,
    const wstring& txItemName,
    ItemDataMap_t& itemDataMap)
{
    using namespace Buy::Table;
    Buy::Text_t text(CharColumnWidths, ColumnCount);
    const wstring textItemName(text.GetItemName(pTextRow));
    if (!textItemName.empty())
    {
        if (!txItemName.empty() && (textItemName != txItemName))
        {
            LogError(L"TxBuyTabGetItems::AddRow(): Desired (%s) On Screen (%s)",
                     txItemName.c_str(), textItemName.c_str());
            return false;
        }
        else
        {
            size_t Price = text.GetPrice(pTextRow);
            if (0 != Price)
            {
                size_t Quantity = text.GetQuantity(pTextRow);
                auto [itMap, _] = itemDataMap.insert(make_pair(textItemName, PriceCountMap_t()));
                auto [pq, pqInserted] = itMap->second.insert(make_pair(Price, Quantity));
                if (!pqInserted)
                {
                    pq->second += Quantity;
                }
            }
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

} // BuyTabGetItems
} // Transaction
} // Broker

////////////////////////////////////////////////////////////////////////////////
