////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TiBrokerSell.cpp
//
// Broker Sell window text interpreter.
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BrokerSell.h"
#include "PipelineManager.h"
#include "BrokerId.h"
#include "TransactionManager.h"
#include "SsWindow.h"
//#include "TxRepriceItems.h"

////////////////////////////////////////////////////////////////////////////////

namespace Broker
{
namespace Sell
{
namespace Interpret
{

////////////////////////////////////////////////////////////////////////////////

Handler_t::
Handler_t(
    Window::ManagerBase_t& Manager)
:
    m_Manager(Manager)
{
}

////////////////////////////////////////////////////////////////////////////////

/*
bool
Handler_t::
Initialize(
        const wchar_t* pClass)
{
    LogInfo(L"TiBrokerSell::Initialize()");
    if (!DP::Handler_t::Initialize(pClass))
        return false;
    return true;
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
Shutdown()
{
    LogAlways(L"TiBrokerSell::Shutdown()");
    m_MessageThread.Shutdown();
}
*/

/////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
ExecuteTransaction(
    DP::Transaction::Data_t& /*Data*/)
{
/*
    using namespace Transaction;
    switch (Data.Id)
    {
    case Id::RepriceItems:
        RepriceItems::Handler_t::Start(static_cast<RepriceItems::Data_t&>(Data));
        return S_OK;
    default:
        break;
    }
*/
    return S_FALSE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
MessageHandler(
    const DP::Message::Data_t* pMessage)
{
    if (Message::Id::Sell != pMessage->Id)
    {
        return S_FALSE;
    }
#if 0

    const Translate::Data_t&
        Message = *static_cast<const Translate::Data_t*>(pMessage);
Message;
//    if (Window::PostedTradeDetailWindow != pDetailData->WindowType)
//        return S_FALSE;

    DP::Transaction::Data_t* pData = GetTransactionManager().Acquire();
    DP::TransactionManager_t::AutoRelease_t ar(pData);
    if (NULL == pData)
    {
        return S_FALSE;
    }

    using namespace Transaction;
    switch (pData->Id)
    {
    case Id::RepriceItems:
        return RepriceItems::Handler_t::Message(
                   *pMessage, // Message,
                   static_cast<RepriceItems::Data_t&>(*pData),
                   GetManager().GetWindow());
    default:
        break;
    }
#endif
    return S_FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// Returns true if we're currently scrolled to top (or can't scroll)
/* static */
bool
Handler_t::
IsScrolledToTop(
    const Sell::Translate::Data_t& Message) 
{
    using namespace Ui::Scroll;
    return ((Position::Top == Message.VScrollPos) ||
           (Position::Unknown == Message.VScrollPos));
}

////////////////////////////////////////////////////////////////////////////////
// Returns true if we're currently scrolled to bottom (or can't scroll)
/* static */
bool
Handler_t::
IsScrolledToBottom(
    const Sell::Translate::Data_t& Message)
{
    using namespace Ui::Scroll;
    return (Position::Bottom == Message.VScrollPos) ||
           (Position::Unknown == Message.VScrollPos);
}

////////////////////////////////////////////////////////////////////////////////

} // Interpret
} // Sell
} // Broker

////////////////////////////////////////////////////////////////////////////////
