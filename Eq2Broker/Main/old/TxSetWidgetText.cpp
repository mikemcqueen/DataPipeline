/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TxSetWidgetText.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PipelineManager.h"
#include "TransactionManager.h"
#include "TxSetWidgetText.h"
#include "UiEvent.h"
#include "UiInput.h"
#include "DcrBrokerBuy.h"
#include "Eq2Broker_t.h"

namespace Broker::Transaction::SetWidgetText {

/////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
ExecuteTransaction(
    DP::Transaction::Data_t& txData)
{
    LogInfo(L"TxSetWidgetText::ExecuteTransaction()");
    txData.SetState(State::First);
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
OnTransactionComplete(
    const DP::Transaction::Data_t&)
{
    LogInfo(L"TxSetWidgetText::TransactionComplete()");
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
//
// Idea:
// The "state" is defined by the input (state of the UI/contents of pMessage)
// and doesn't need to be tracked in the transaction itself.
// 
// text field:                 state:          action:
//   empty                       empty field     enter text
//   has matching contents       valid text      complete transaction
//   has non-matching contents   invalid text    clear text
//
// So we could have a getState(pMessage) method, and maybe a "state -> action"
// map. Then the code for this function is one line:
//
// stateActionMap[getState(pMessage)](pMessage);
// 
// Issues:
// This class/handler is tightly coupled to BrokerBuy window based on the
// (Dcr) message type. So it's named generically but doesn't act that way.
//
HRESULT
Handler_t::
MessageHandler(
    const DP::Message::Legacy::Data_t* pMessage)
{
    LogInfo(L"TxSetWidgetText::MessageHandler()");
    DP::TransactionManager_t::AutoRelease_t TxData(GetTransactionManager().Acquire());
    DP::Transaction::Data_t* pTxData = TxData.get();
    if (nullptr == pTxData) {
        throw logic_error("TxSetWidgetText::MessageHandler(): No transaction active");
    }
    using Broker::Transaction::SetWidgetText::Data_t;
    Data_t& txData = static_cast<Data_t&>(*pTxData);

    // hacky, but need to either shove DcrRect_t& into TxData
    // or manually extract widgetText from each supported
    // window/message type
    if (Message::Id::Buy != pMessage->Id)
    {
        txData.Complete(Error::InvalidWindow);
        return S_FALSE;
    }
    const auto buyMessage = static_cast<const Buy::Translate::Data_t&>(*pMessage);
    const wstring widgetText(buyMessage.searchText);
    switch (txData.GetState()) {
    case State::ClearText:
        if (!widgetText.empty()) {
            ClearText(txData, widgetText);
        } else {
            txData.NextState();
        }
        break;

    case State::EnterText:
        if (widgetText != txData.text) {
            Ui::Input_t::SendChars(txData.text.c_str());
        }
        txData.NextState();
        break;

    case State::ValidateText:
        if (widgetText == txData.text) {
            GetTransactionManager().CompleteTransaction(txData.Id);
        } else {
            static const size_t kTimeoutThreshold = 30;
            if (txData.GetStateTimeout() > kTimeoutThreshold)
            {
                LogError(L"TxSetWidgetText::ValidateText: Invalid Text(%s)",
                         widgetText.c_str());
                txData.SetState(State::ClearText);
            }
        }
        break;

    default:
        LogError(L"TxSetWidgetText::MessageHandler(): Invalid state (%x)",
                 txData.GetState());
        throw logic_error("TxSetWidgetText::MessageHandler(): invalid state");
    }
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
ClearText(
    Data_t&  txData,
    const wstring& currentText) const
{
    Ui::Window_t& window = m_broker.GetWindow(txData.windowId);
    Rect_t rect;
    if (window.GetWidgetRect(txData.widgetId, rect))
    {
        // click on right side so backspace characters will delete all text
        rect.left = rect.right - 1;
        window.ClickWidget(txData.widgetId, true, &rect);
        size_t wordCount = 1 + count(currentText.begin(), currentText.end(), L' ');
        Ui::Input_t::SendKey(VK_CONTROL);
        Ui::Input_t::SendChar(VK_BACK, wordCount, 0);
        Ui::Input_t::SendKey(VK_CONTROL, true);
    }
    else
    {
        LogError(L"TxSetWidgetText::ClearText() %s::GetWidgetRect(%d) failed",
                 window.GetWindowName(), txData.widgetId);
        throw logic_error("TxSetWidgetText::ClearText() GetWidgetRect() failed");
    }
}

/////////////////////////////////////////////////////////////////////////////

} // Broker::Transaction::SetWidgetText

/////////////////////////////////////////////////////////////////////////////
