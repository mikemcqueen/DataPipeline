/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// TiBrokerBuy.h
//
// Broker Buy window text interpreter.
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TIBROKERBUY_H
#define Include_TIBROKERBUY_H

#include "DpHandler_t.h"
#include "DpTransaction.h"
#include "BrokerBuyTypes.h"
#include "AutoCs.h"
#include "PageNumber_t.h"
#include "TiBase_t.h"

namespace Broker
{
namespace Buy
{

typedef TiBase_t<Table::RowCount, Table::CharsPerRow, Table::ColumnCount> TiTable_t;

namespace Interpret
{

/////////////////////////////////////////////////////////////////////////////

class Handler_t :
    public DP::Handler_t
{

private:

    Window::ManagerBase_t&  m_Manager;
    PageNumber_t            m_PageNumber;

    mutable CAutoCritSec    m_csState;

public:

    Handler_t(
        Window::ManagerBase_t& Manager);

    //
    // DP::Handler_t virtual:
    //

    virtual
    bool
    Initialize(
        const wchar_t* pszClass) override;

#if 0
    virtual
    void
    Shutdown() override;
#endif

    HRESULT
    MessageHandler(
        const DP::Message::Data_t* pData) override;

    HRESULT
    ExecuteTransaction(
        const DP::Transaction::Data_t& Data) override;

private:

    Window::ManagerBase_t& GetManager() const { return m_Manager; }

private:

    Handler_t();
    Handler_t(const Handler_t&);
    Handler_t& operator= (const Handler_t&);
};

/////////////////////////////////////////////////////////////////////////////

} // Interpret
} // Buy
} // Broker

#endif // Include_TIBROKERBUY

/////////////////////////////////////////////////////////////////////////////