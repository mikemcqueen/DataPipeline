/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TxSetActiveWindow.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PipelineManager.h"
#include "TransactionManager.h"
#include "TxSetActiveWindow.h"
#include "UiEvent.h"
#include "MainWindow_t.h"
#include "BrokerWindow.h"
#include "UiInput.h"

namespace Broker
{
namespace Transaction
{
namespace SetActiveWindow
{

/////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
ExecuteTransaction(
    DP::Transaction::Data_t&)
{
    LogInfo(L"TxSetActiveWindow::ExecuteTransaction()");
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
OnTransactionComplete(
    DP::Transaction::Data_t&)
{
    LogInfo(L"TxSetActiveWindow::TransactionComplete()");
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
MessageHandler(
    const DP::Message::Data_t* pData)
{
    LogInfo(L"TxSetActiveWindow::MessageHandler()");
    DP::TransactionManager_t::AutoRelease_t TxData(GetTransactionManager().Acquire());
    DP::Transaction::Data_t* pTxData = TxData.get();
    if (NULL == pTxData)
    {
        throw logic_error("TxSetActiveWindow::MessageHandler(): No transaction active");
    }
    Data_t& txData = static_cast<Data_t&>(*pTxData);
    SetActiveWindow(pData->Id, txData);
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
SetActiveWindow(
    DP::MessageId_t messageId,
    Data_t&         txData) const
{
    static const wchar_t kEsc = L'\027';

    using namespace Broker::Message;
    const Ui::WindowId_t curWindowId = m_mainWindow.GetMessageWindowId(messageId);
    Ui::Window_t& curWindow = m_mainWindow.GetWindow(curWindowId);
//    const Ui::WindowID_t
    if (curWindowId == txData.windowId)
    {
        Complete(txData);
        return;
    }
    if (Window_t::IsTabWindow(curWindowId))
    {
        if (Window_t::IsTabWindow(txData.windowId))
        {
            curWindow.ClickWidget(Window_t::GetTabWidgetId(txData.windowId));
        }
        else if ((Window::Id::BrokerSetPricePopup == txData.windowId) &&
                 (Window::Id::BrokerSellTab == curWindowId))
        {
            curWindow.ClickWidget(Sell::Widget::Id::SetPriceButton);
        }
    }
    else
    {
        switch (messageId)
        {
        case Message::Id::SetPrice:
            Ui::Input_t::SendChar(kEsc);
            break;
        default:
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void 
Handler_t::
Complete(
    Data_t& txData) const
{
//    DumpItems(TxData.itemDataMap);
    GetTransactionManager().CompleteTransaction(txData.Id);
}

////////////////////////////////////////////////////////////////////////////////

} // SetActiveWindow
} // Transaction
} // Broker

/////////////////////////////////////////////////////////////////////////////
