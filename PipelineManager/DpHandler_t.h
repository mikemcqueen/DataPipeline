////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DpHandler_t.h
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif

#ifndef Include_DPHANDLER_T_H
#define Include_DPHANDLER_T_H

#include "DpTransaction.h"
#include "DpEvent.h"
#include "DpMessage.h"
#include "Log.h"

namespace DP
{

////////////////////////////////////////////////////////////////////////////////

class PipelineManager_t;

class Handler_t
{
    friend class PipelineManager_t;

public:
    Handler_t() = default;
    
    virtual
    ~Handler_t() {}

    Handler_t(const Handler_t&) = delete;
    Handler_t& operator=(const Handler_t&) = delete;
    Handler_t(const Handler_t&&) = delete;
    Handler_t& operator=(const Handler_t&&) = delete;

    virtual
    bool
    Initialize(
        const wchar_t* pszClass)
    {
        SetClass(pszClass);
        return true;
    }

    // TODO:
    // virtual void Shutdown() {}

    virtual
    HRESULT
    MessageHandler(
        const Message::Data_t* /* pMessage */)
    {
        return S_FALSE;
    }

    virtual
    HRESULT
    EventHandler(
        Event::Data_t& Data)
    {
        using namespace Message;
        switch (Data.Type)
        {
            // TODO: move this hack into PM.SendEvent/Dispatch
        case Type::Transaction:
            {
                using namespace Transaction;
                using Transaction::Data_t;
                Data_t& TransactionData = static_cast<Data_t&>(Data);
                switch (TransactionData.GetState())
                {
                case State::New:
                    {
                        HRESULT hr = ExecuteTransaction(TransactionData);
#if 1
                        if (S_FALSE == hr)
                            hr = S_OK;
#endif
                        return hr;
                    }
                    break;
                case State::Complete:
                    {
                        HRESULT hr = OnTransactionComplete(TransactionData);
#if 1
                        if (S_FALSE == hr)
                            hr = S_OK;
#endif
                        return hr;
                    }
                    break;
                default:
                    break;
                }
            }
            break;
        default:
            break;
        }
        return S_FALSE;
    }

    virtual
    HRESULT
    ExecuteTransaction(
        Transaction::Data_t& /*txData*/)
    {
        LogWarning(L"DP::Handler_t::ExecutionTransaction fall-through (S_FALSE)");
        return S_FALSE;
    }
 
    virtual
    HRESULT
    ResumeTransaction(
        Transaction::Data_t& /*txData*/,
        const Transaction::Data_t* /*pPrevTxData*/)
    {
        LogWarning(L"DP::Handler_t::ResumeTransaction fall-through (S_FALSE)");
        return S_FALSE;
    }

    virtual
    HRESULT
    OnTransactionComplete(
        const Transaction::Data_t&)
    {
        LogWarning(L"DP::Handler_t::OnTransactionComplete fall-through (S_FALSE)");
        return S_FALSE;
    }

    const std::wstring& GetClass() const { return m_strClass; }

private:

    void
    SetClass(
        const wchar_t* pszClass)
    {
        if (nullptr != pszClass) {
            m_strClass.assign(pszClass);
        } else {
            m_strClass.clear();
        }
    }

private:
    std::wstring m_strClass;
};

} // namespace DP

////////////////////////////////////////////////////////////////////////////////

#endif // Include_DPHANDLER_T_H

////////////////////////////////////////////////////////////////////////////////
