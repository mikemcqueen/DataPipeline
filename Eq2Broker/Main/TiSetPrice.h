/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TiSetPrice.h
//
// Set Price popup window text interpreter.
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TISETPRICE_H
#define Include_TISETPRICE_H

/////////////////////////////////////////////////////////////////////////////

#include "DpHandler_t.h"
#include "SetPriceTypes.h"
#include "BrokerId.h"
#include "AutoCs.h"

namespace Broker
{
namespace SetPrice
{
namespace Interpret
{

/////////////////////////////////////////////////////////////////////////////

class Handler_t :
    public DP::Handler_t
{

private:

    Window::ManagerBase_t&  m_Manager;

public:

    Handler_t(
        Window::ManagerBase_t& Manager);

    //
    // DP::Handler_t virtual:
    //

    HRESULT
    MessageHandler(
        const DP::Message::Data_t* pData) override;

    HRESULT
    ExecuteTransaction(
        DP::Transaction::Data_t& Data) override;

private:

    Window::ManagerBase_t& GetManager() const { return m_Manager; }

private:

    Handler_t();
    Handler_t(const Handler_t&);
    Handler_t& operator= (const Handler_t&);
};

} // Interpret
} // SetPrice
} // Broker

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TISetPrice

/////////////////////////////////////////////////////////////////////////////
