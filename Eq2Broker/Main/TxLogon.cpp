////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TxLogon.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PipelineManager.h"
#include "TransactionManager.h"
#include "TxLogon.h"
#include "UiEvent.h"
#include "Eq2Broker_t.h"
#include "UiWindow.h"
#include "BrokerUi.h"
#include "DcrEq2Login.h"

namespace Broker
{
namespace Transaction
{
namespace Logon
{

/////////////////////////////////////////////////////////////////////////////

    namespace State
    {
        enum : DP::Transaction::State_t
        {
            First                  = DP::Transaction::State::User_First,
            ClickButton            = First,
            VerifyHandle,
            ClickConnect,
            VerifyConnecting,
        };
    }

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
ExecuteTransaction(
    DP::Transaction::Data_t& data)
{
    LogInfo(L"TxLogon::ExecuteTransaction()");
    using Broker::Transaction::Logon::Data_t;
    Data_t& txData = static_cast<Data_t&>(data);
    switch (txData.method)
    {
    case Method::Eq2Login:
        txData.SetState(State::First);
        break;
    default:
        break;
    }
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
OnTransactionComplete(
    DP::Transaction::Data_t&)
{
    LogInfo(L"TxLogon::TransactionComplete()");
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
MessageHandler(
    const DP::Message::Data_t* pMessage)
{
    LogInfo(L"TxLogon::MessageHandler()");
    DP::TransactionManager_t::AutoRelease_t ar(GetTransactionManager().Acquire());
    DP::Transaction::Data_t* pTxData = ar.get();
    if (NULL == pTxData)
    {
        throw logic_error("TxLogon::MessageHandler() No transaction active");
    }
    using Broker::Transaction::Logon::Data_t;
    Data_t& txData = static_cast<Data_t&>(*pTxData);
    switch (txData.method)
    {
    case Method::Camp:
        {
            Ui::Event::SendChars::Data_t sendChars(L"/camp\n");
            GetPipelineManager().SendEvent(sendChars);
            txData.Complete();
        }
        return S_OK;
    case Method::Eq2Login:
        return Eq2Login(static_cast<const Eq2Login::Translate::Data_t&>(*pMessage), txData);
            
    default:
        break;
    }
    throw logic_error("TxLogon::MessageHandler() Unknown method");
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
Eq2Login(
    const Eq2Login::Translate::Data_t& message,
          Data_t&                              txData) const
{
    if (Message::Id::Eq2Login != message.Id)
    {
        txData.Complete(Error::InvalidWindow);
        return S_FALSE;
    }
    switch (txData.GetState())
    {
    case State::ClickButton:
        {
            Ui::WidgetId_t widgetId = GetCharacterButton(message, txData.loginHandle);
            if (Ui::Widget::Id::Unknown != widgetId)
            {
                m_broker.GetWindow(Window::Id::Eq2Login).ClickWidget(widgetId);
                txData.NextState();
            }
            else
            {
                txData.Complete(Error::InvalidLoginHandle);
            }
        }
        break;
    case State::VerifyHandle:
        {
            if (message.characterName == txData.loginHandle)
            {
                txData.NextState();
            }
            else
            {
                txData.PrevState();
            }
        }
        break;
    case State::ClickConnect:
        {
            m_broker.GetWindow(Window::Id::Eq2Login).ClickWidget(Eq2Login::Widget::Id::ConnectButton);
            txData.NextState();
        }
        break;
    case State::VerifyConnecting:
        txData.Complete();
        break;
    default:
        throw logic_error("TxLogon::Eq2Login() Invalid state");
    }
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

Ui::WidgetId_t
Handler_t::
GetCharacterButton(
    const Eq2Login::Translate::Data_t& message,
    const wstring&                     characterName) const
{
    using namespace Broker::Eq2Login;
    for (size_t button = 0; button < kCharacterButtonCount; ++button)
    {
        if (characterName == message.characterButtons[button].text)
        {
            return Eq2Login::Widget::Id::FirstCharacterButton + button;
        }
    }
    return Ui::Widget::Id::Unknown;
}

////////////////////////////////////////////////////////////////////////////////

} // Logon
} // Transaction
} // Broker

/////////////////////////////////////////////////////////////////////////////
