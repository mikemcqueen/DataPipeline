/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TxSetActiveWindow.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TXSETACTIVEWINDOW_H
#define Include_TXSETACTIVEWINDOW_H

#include "DpHandler_t.h"
#include "BrokerId.h"
#include "UiWindowId.h"
#include "BrokerUi.h"

namespace Broker
{
namespace Transaction
{
namespace SetActiveWindow
{

/////////////////////////////////////////////////////////////////////////////

    struct Data_t :
        public DP::Transaction::Data_t
    {
        Ui::WindowId_t windowId;

        Data_t(
            Ui::WindowId_t Id)
        :
            DP::Transaction::Data_t(
                Id::SetActiveWindow,
                sizeof(Data_t)),
            windowId(Id)
        { }

    private:

        Data_t();
    };

/////////////////////////////////////////////////////////////////////////////

    class Handler_t :
        public DP::Handler_t
    {
    private:

        MainWindow_t& m_mainWindow;

    public:

        Handler_t(
            MainWindow_t& mainWindow)
        :
            m_mainWindow(mainWindow)
        {}

        //
        // DP::Handler_t virtual
        //

        HRESULT
        MessageHandler(
            const DP::Message::Data_t* pData) override;

        HRESULT
        ExecuteTransaction(
            const DP::Transaction::Data_t& Data) override;

        HRESULT
        OnTransactionComplete(
            const DP::Transaction::Data_t& Data) override;

        //
        
        void
        SetActiveWindow(
            DP::MessageId_t messageId,
            Data_t&         txData) const;

        void
        GetMessageWindow(
            DP::MessageId_t messageId) const;

    private:

        void 
        Complete(
            Data_t& txData) const;

    private:

        Handler_t();
    };

} // SetActiveWindow
} // Transaction
} // Broker

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TXSETACTIVEWINDOW_H

/////////////////////////////////////////////////////////////////////////////
