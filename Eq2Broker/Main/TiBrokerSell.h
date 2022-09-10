/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TiBrokerSell.h
//
// Broker Sell window text interpreter.
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TIBROKERSELL_H
#define Include_TIBROKERSELL_H

#include "DpHandler_t.h"
#include "BrokerSellTypes.h"
#include "BrokerId.h"
#include "TiBase_t.h"
#include "AutoCs.h"

class PageNumber_t;

namespace Broker
{
namespace Sell
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

    //

    static
    bool
    IsScrolledToTop(
        const Translate::Data_t& Message);

    static
    bool
    IsScrolledToBottom(
        const Translate::Data_t& Message);

private:

    Window::ManagerBase_t& GetManager() const { return m_Manager; }

private:

    Handler_t();
    Handler_t(const Handler_t&);
    Handler_t& operator= (const Handler_t&);
};

} // Interpret
} // Sell
} // Broker

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TIBROKERSELL_H

/////////////////////////////////////////////////////////////////////////////
