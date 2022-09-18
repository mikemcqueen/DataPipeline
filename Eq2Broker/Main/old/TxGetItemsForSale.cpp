////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TxGetItemsForSale.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TxGetItemsForSale.h"
#include "BrokerBuy.h"
#include "BrokerBuyText.h"
#include "PipelineManager.h"
#include "BrokerId.h"
#include "TransactionManager.h"
#include "DbItemsForSale_t.h"
#include "Db.h"

//extern wchar_t g_szDbName[];
//extern wchar_t g_szServerName[];

////////////////////////////////////////////////////////////////////////////////

namespace Broker
{
namespace Transaction
{
namespace GetItemsForSale
{ 

////////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
ThreadProcessor_t::
operator()(
    ThreadQueue::State_t State,
    const Message_t&     Message,
          int*           /*Dummy*/)
{
    using namespace ThreadQueue;
    switch (State)
    {
    case State::Startup:
        return Handler_t::ThreadStartup();

    case State::Execute:
        Handler_t::ThreadMessage(Message);
        break;

    case State::Shutdown:
        Handler_t::ThreadShutdown();
        break;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
Initialize()
{
    return GetThread().Initialize(nullptr, L"TxGetItemsForSale_t::");
}

////////////////////////////////////////////////////////////////////////////////

void
Handler_t::
Shutdown()
{
    GetThread().Shutdown();
}

////////////////////////////////////////////////////////////////////////////////

Handler_t::MessageThread_t&
Handler_t::
GetThread()
{
    static MessageThread_t MessageThread;
    return MessageThread;
}

////////////////////////////////////////////////////////////////////////////////

void
Handler_t::
Start(
    Data_t& TxData)
{
    TxData.PageNumber.Reset();
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
Message(
    const Buy::Translate::Data_t& Message,
          Data_t&                 TxData,
    const Ui::Window_t&           Window)
{
    const PageNumber_t& CurPage  = Message.pageNumber;
    PageNumber_t&       PrevPage = TxData.PageNumber;

    bool ExtraLog = false;
    if (ExtraLog)
    {
        LogAlways(L"TxGetItemsForSale_t::Message(%08x, %08x)"
                  L" Curr: Page %d of %d"
                  L" Prev: Page %d of %d",
                  Message.Id, TxData.Id,
                  CurPage.GetPage(),  CurPage.GetLastPage(),
                  PrevPage.GetPage(), PrevPage.GetLastPage());
    }
    if (ValidateMessage(CurPage, PrevPage))
    {
        GetThread().QueueData(Message.tableText);
        PrevPage = CurPage;
        if (CurPage.GetPage() == CurPage.GetLastPage())
        {
            LogAlways(L"TxGetItemsForSale_t completing...");
            GetTransactionManager().CompleteTransaction(TxData.Id);
        }
        else
        {
            Window.ClickWidget(Buy::Widget::Id::NextButton);
        }
        return S_OK;
    }
    return S_FALSE;
}

////////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
ValidateMessage(
    const PageNumber_t& CurPage,
    const PageNumber_t& PrevPage)
{
    if (!CurPage.IsValid())
    {
        return false;
    }
    if (!PrevPage.IsValid())
    {
        LogAlways(L"First Page");
    }
/*
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
    else
*/
    {
        if (PrevPage.GetPage() == CurPage.GetPage())
        {
            // TODO:  click failure, same page timeout, reclick
            LogAlways(L"Same page - Ignoring...");
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

////////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
ThreadStartup()
{
    //TODO threadqueue_t::onstartup look at return value
    throw runtime_error("implementation removed");
#if 0
    if (!Db::Initialize(g_szDbName, g_szServerName, true, GetCurrentThreadId()))
    {
        LogError(L"TxGetItemsForSale_t::ThreadStartup(): Db::Initialize() failed");
        return false;
    }
    return true;
#endif
}

////////////////////////////////////////////////////////////////////////////////

void
Handler_t::
ThreadShutdown()
{
    Db::Cleanup(GetCurrentThreadId());
}

////////////////////////////////////////////////////////////////////////////////

void
Handler_t::
ThreadMessage(
    const Message_t& Text)
{
    for (size_t Row = 0; Row < Text.GetEndRow(); ++Row)
    {
        size_t Quantity = Text.GetQuantity(Row);
        const wchar_t* pItemName = Text.GetItemName(Row);
        if (L'\0' == pItemName[0])
        {
            continue;
        }
        size_t Price = Text.GetPrice(Row);
        if (0 == Price)
        {
            continue;
        }
        wstring strSellerName;
        Text.GetSellerName(Row, strSellerName);
        Db::ItemsForSale_t::AddItem(pItemName, Price, Quantity, strSellerName.c_str());
    }
}

////////////////////////////////////////////////////////////////////////////////

} // GetItemsForSale
} // Transaction
} // Broker

////////////////////////////////////////////////////////////////////////////////

