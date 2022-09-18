////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// TxGetItemsForSale.h
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TXGETITEMSFORSALE_H
#define Include_TXGETITEMSFORSALE_H

#include "BrokerBuyTypes.h"
#include "BrokerBuyText.h"
#include "BrokerId.h"
#include "ThreadQueue_t.h"
#include "UiTypes.h"
#include "PageNumber_t.h"
#include "StringSet_t.h"

namespace Broker
{
namespace Transaction
{
namespace GetItemsForSale
{

////////////////////////////////////////////////////////////////////////////////

    struct Data_t :
        public DP::Transaction::Data_t
    {
        PageNumber_t PageNumber;

        Data_t() :
            DP::Transaction::Data_t(
                Id::GetItemsForSale,
                sizeof(Data_t))
        { }
    };

////////////////////////////////////////////////////////////////////////////////

class Handler_t
{
    typedef Buy::Text_t Message_t;

    struct ThreadProcessor_t
    {
        bool
        operator()(
            ThreadQueue::State_t State,
            const Message_t&     Message,
                  int*           Dummy);
    };

    typedef ThreadQueue_t<Message_t, int, ThreadProcessor_t> MessageThread_t;

public:

    static
    bool
    Initialize();

    static
    void
    Shutdown();

    static
    void
    Start(
        Data_t& TxData);

    static
    HRESULT
    Message(
        const Buy::Translate::Data_t& Message,
              Data_t&                 TxData,
        const Ui::Window_t&           Window);

    static 
    void 
    Complete();

    static
    bool
    ValidateMessage(
        const PageNumber_t& CurPage,
        const PageNumber_t& PrevPage);

private:

    //  
    // Message processing thread functions:
    //

    static
    MessageThread_t&
    GetThread();

    static
    bool
    ThreadStartup();

    static
    void
    ThreadShutdown();

    static
    void
    ThreadMessage(
        const Message_t& Message);

};

} // GetItemsForSale
} // Transaction
} // Broker

////////////////////////////////////////////////////////////////////////////////

#endif // Include_TXGETITEMSFORSALE_H

////////////////////////////////////////////////////////////////////////////////
