////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TiSetPrice.h
//
// Broker Set-Price popup window text interpreter.
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TiSetPrice.h"
//#include "SetPrice.h"
//#include "PipelineManager.h"
//#include "BrokerId.h"
//#include "TransactionManager.h"
//#include "TxRepriceItems.h"

////////////////////////////////////////////////////////////////////////////////

namespace Broker
{
namespace SetPrice
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

HRESULT
Handler_t::
ExecuteTransaction(
    DP::Transaction::Data_t& /*Data*/)

{
#if 0
    using namespace Transaction;
    switch (Data.Id)
    {
    case Id::RepriceItems:
        // RepriceItems::Handler_t::Start(static_cast<RepriceItems::Data_t&>(Data));
        break; // return S_OK;
    default:
        break;
    }
#endif
    return S_FALSE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
MessageHandler(
    const DP::Message::Data_t* /*pMessage*/)
{
#if 0
    if (Message::Id::SetPrice != pMessage->Id)
    {
        return S_FALSE;
    }
#endif
    return S_FALSE;
}

////////////////////////////////////////////////////////////////////////////////

} // Interpret
} // SetPrice
} // Broker

////////////////////////////////////////////////////////////////////////////////
