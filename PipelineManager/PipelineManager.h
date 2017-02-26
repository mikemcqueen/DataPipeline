////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// PipelineManager.h
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_PIPELINEMANAGER_H
#define Include_PIPELINEMANAGER_H

#include "ThreadQueue_t.h"
#include "AutoHand.h"
#include "AutoCs.h"
#include "DpHandler_t.h"
#include "DpTransaction.h"

////////////////////////////////////////////////////////////////////////////////

namespace DP
{
    class PipelineManager_t;
}

DP::PipelineManager_t&
GetPipelineManager();

namespace DP
{

////////////////////////////////////////////////////////////////////////////////
//
// PipelineManager_t
//
////////////////////////////////////////////////////////////////////////////////

class PipelineManager_t
{
    friend class TransactionManager_t;

public:

    struct ProcessThreadData_t
    {
        bool
        operator()(
            ThreadQueue::State_t State,
            Message::Data_t*     pData,
            PipelineManager_t*   pPm) const;
    };

private:


    struct HandlerData_t
    {
        Stage_t    Stage;
        Handler_t* pHandler;
        std::wstring    name;

        HandlerData_t() :
            Stage(Stage::None), pHandler(NULL)
        { }

        HandlerData_t(Stage_t s, Handler_t* h, const wchar_t* n = 0) :
            Stage(s), pHandler(h), name((NULL != n) ? n : L"")
        { }
    };
    typedef vector<HandlerData_t>                HandlerVector_t;
    typedef map<TransactionId_t, HandlerData_t>  TxIdHandlerMap_t;

    struct CompareMessage_t
    {
        bool operator()(
            const Message::Data_t* pD1,
            const Message::Data_t* pD2) const;
    };

    typedef ThreadQueue_t<Message::Data_t*,
                          PipelineManager_t,
                          ProcessThreadData_t,
                          CompareMessage_t>  MessageThread_t;

    friend class MessageThread_t;

private:

    MessageThread_t  m_MessageThread;
    HandlerVector_t  m_Handlers;
    TxIdHandlerMap_t m_txHandlerMap;
    CAutoCritSec     m_csHandlers;

public:

    static
    PipelineManager_t&
    Get();

    static
    const wchar_t*
    GetStageString(
        Stage_t stage);

    // Constructor/destructor

    PipelineManager_t();
    ~PipelineManager_t();

    bool
    Initialize();

    void
    Shutdown();

    void
    AddHandler(
              Stage_t    Stage,
              Handler_t& Handler,
        const wchar_t*   pszClass = NULL);

    void
    AddTransactionHandler(
        Stage_t          stage,
        TransactionId_t  transactionId,
        Handler_t&       handler,
        const wchar_t*   displayName = NULL);

    size_t
    StartAcquiring(
        const wchar_t* pszClass = NULL);

    size_t
    StopAcquiring(
        const wchar_t* pszClass = NULL);

    size_t
    SendEvent(
        Event::Data_t& Data);

    void*
    Alloc(
        size_t Size);

    void
    Free(
        void* pMem);

    HRESULT
    Callback(
        Message::Data_t* pData);

    size_t
    Flush(
              Stage_t   Stage,
        const wchar_t* pszClass);

    void
    QueueApc(
        PAPCFUNC  ApcFunc,
        ULONG_PTR Param);

    HANDLE GetIdleEvent() const { return m_MessageThread.GetIdleEvent(); }

    const wchar_t* GetTransactionName(TransactionId_t txId) const;

private:

    void
    Dispatch(
        Message::Data_t* pMessage);

    void
    Release(
        Message::Data_t* pData);

    bool
    GetNextHandler(
              Stage_t   Stage,
        const wchar_t* pszClass,
        HandlerVector_t::const_iterator& it) const;

    HRESULT
    TrySendTransactionMessage(
        Message::Data_t* pMessage,
        Stage_t          stage);

    // TransactionManager internal call
    HRESULT
    SendTransactionEvent(
        Transaction::Data_t& Data,
        Transaction::Data_t* pLastTxData = NULL);

private:

    PipelineManager_t(const PipelineManager_t&);
    PipelineManager_t& operator=(const PipelineManager_t&);
};

} // DP

////////////////////////////////////////////////////////////////////////////////

#endif // Include_PIPELINEMANAGER_H

////////////////////////////////////////////////////////////////////////////////
