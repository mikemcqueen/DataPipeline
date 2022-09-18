/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TxOpenBroker.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TXOPENBROKER_H
#define Include_TXOPENBROKER_H

#include "DpHandler_t.h"
#include "BrokerId.h"
#include "UiWindowId.h"
#include "SsWindow.h"

class Eq2Broker_t;

namespace Broker
{
namespace Transaction
{
namespace OpenBroker
{

/////////////////////////////////////////////////////////////////////////////

    namespace State
    {
        enum E : DP::Transaction::State_t
        {
            First                          = DP::Transaction::State::User_First,
            MoveToLastPoint                = First,
            FindBroker,
            ClickBroker,
        };
    }

/////////////////////////////////////////////////////////////////////////////

    struct Data_t :
        public DP::Transaction::Data_t
    {
        size_t position;
        POINT  point;
        bool   skip;

        Data_t() :
            DP::Transaction::Data_t(
                Id::OpenBroker,
                sizeof(Data_t))
            ,position(0)
            ,skip(false)
        { }
    };

/////////////////////////////////////////////////////////////////////////////

    class Handler_t :
        public DP::Handler_t
    {
    private:
    
        static const size_t kCursorWidth = 32;
        static const size_t kCursorHeight = 32;

        typedef char CursorBits_t[kCursorHeight * (kCursorWidth / 8)];

    private:

        static POINT s_lastPoint;

        Eq2Broker_t& m_broker;
        CursorBits_t m_cursorBits;

    public:

        Handler_t(
            Eq2Broker_t& broker);

        //
        // DP::Handler_t virtual
        //
        HRESULT
        MessageHandler(
            const DP::Message::Data_t* pData) override;

        HRESULT
        ExecuteTransaction(
            DP::Transaction::Data_t& Data) override;

        HRESULT
        OnTransactionComplete(
            const DP::Transaction::Data_t& Data) override;

    private:

        void
        loadCursor();

        // could be static
        bool
        IsBrokerCursor() const;

        // could be static
        bool
        GetCursorBits(
            size_t width, 
            size_t height,
            char*  bits) const;

        // could be static
        bool
        GetCursorBits(
            HCURSOR hCursor,
            size_t width,
            size_t height,
            char*  bits) const;

        // Could be static member of BrokerWindow_t
        bool
        IsBrokerWindow(
            Ui::WindowId_t windowId);

        HRESULT
        OpenBroker(
            const SsWindow::Acquire::Data_t& ssData,
                  Data_t&                    txData) const;

        bool
        GetPositionPoint(
            const Rect_t& rect,
                  size_t  position,
                  POINT&  point) const;

    private:

        Handler_t();
    };

} // OpenBroker
} // Transaction
} // Broker

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TXOPENBROKER_H

/////////////////////////////////////////////////////////////////////////////
