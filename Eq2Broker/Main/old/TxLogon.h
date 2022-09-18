/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TxLogon.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TXLOGON_H
#define Include_TXLOGON_H

#include "Eq2LoginTypes.h"
#include "DpHandler_t.h"
#include "BrokerId.h"
#include "UiWindowId.h"

class Eq2Broker_t;

namespace Broker
{
namespace Transaction
{
namespace Logon
{

/////////////////////////////////////////////////////////////////////////////

    namespace Method
    {
        enum E : unsigned
        {
            Camp,
            Eq2Login,
        };
    }
    typedef Method::E Method_t;

/////////////////////////////////////////////////////////////////////////////

    struct Data_t :
        public DP::Transaction::Data_t
    {
        wstring  loginHandle;
        Method_t method;

        Data_t(
            Method_t initMethod)
        :
            DP::Transaction::Data_t(
                Id::Logon,
                sizeof(Data_t)),
            method(initMethod)
        { }

        Data_t(
            const wstring& handle)
        :
            DP::Transaction::Data_t(
                Id::Logon,
                sizeof(Data_t)),
            method(Method::Eq2Login),
            loginHandle(handle)
        { }

    private:

        Data_t();
    };

/////////////////////////////////////////////////////////////////////////////

    class Handler_t :
        public DP::Handler_t
    {
    private:

        Eq2Broker_t& m_broker;

    public:

        Handler_t(
            Eq2Broker_t& broker)
            :
            m_broker(broker)
        {}

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

        HRESULT
        Eq2Login(
            const Eq2Login::Translate::Data_t& message,
                  Data_t&                      txData) const;


        Ui::WidgetId_t
        GetCharacterButton(
            const Eq2Login::Translate::Data_t& message,
            const wstring&                     characterName) const;

        Handler_t();
    };

} // Logon
} // Transaction
} // Broker

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TXLOGON_H

/////////////////////////////////////////////////////////////////////////////
