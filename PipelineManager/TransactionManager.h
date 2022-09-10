///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TransactionManager.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif

#ifndef Include_TRANSACTIONMANAGER_H
#define Include_TRANSACTIONMANAGER_H

#include "DpTransaction.h"
#include "AutoCs.h"

namespace DP
{
    class TransactionManager_t;
}

extern
DP::TransactionManager_t&
GetTransactionManager();

namespace DP
{

    namespace Transaction
    {
        struct Data_t;
    }

///////////////////////////////////////////////////////////////////////////////

class TransactionManager_t final
{
friend class PipelineManager_t;

public:

    class AutoRelease_t final
    {
        Transaction::Data_t* m_pData;

    public:

        explicit
        AutoRelease_t(
            Transaction::Data_t* pData)
            :
            m_pData(pData)
        { }

        ~AutoRelease_t()
        {
            if (nullptr != m_pData)
            {
                GetTransactionManager().Release(m_pData);
            }
        }

        Transaction::Data_t*
        get()
        {
            return m_pData;
        }

    private:

        AutoRelease_t();
        AutoRelease_t(const AutoRelease_t&);
        AutoRelease_t operator=(const AutoRelease_t&);
    };

private:

    struct Data_t final
    {
        volatile long        refCount;
        Transaction::Data_t* pTxData;

        Data_t(Transaction::Data_t* p) :
            pTxData(p), refCount(1) {}
    };
    typedef std::deque<Data_t>  Queue_t;
    typedef std::vector<Data_t> Stack_t;

private:

    Queue_t              m_queue;
    Stack_t              m_stack;
    mutable CAutoCritSec m_cs;

public:

    static
    TransactionManager_t&
    Get();

    TransactionManager_t();

    bool
    ExecuteTransaction(
        Transaction::Data_t* pData,
        bool                 fInterrupt = false);

    void
    CompleteTransaction(
        TransactionId_t TransactionId,
        DWORD           Error = 0);

    Transaction::Data_t*
    Acquire();

    void
    Release(
        Transaction::Data_t* pData,
        bool bQuiet = true);

private:

    bool
    ExecuteTransaction(
        Transaction::Data_t* pTxData,
        Transaction::Data_t* pPrevTxData,
        bool                 fInterrupt);

    void
    ExecuteNotify(
        Transaction::Data_t* pPrevTxData);

    Data_t*
    GetTransactionExecuting();

    static
    void CALLBACK
    ApcExecute(ULONG_PTR Param);

    static
    void CALLBACK
    ApcComplete(ULONG_PTR Param);

    void
    SendEvent(
        const wchar_t*       name,
        Transaction::Data_t* pPrevTxData = nullptr);

    bool
    DoExecuteTransaction(
        Transaction::Data_t* pData);

    bool
    SetTransactionExecuting(
        Transaction::Data_t* pData,
        bool fInterrupt = false);

    TransactionManager_t::Data_t*
    FindTransaction(
        const Transaction::Data_t* pTxData);

    void
    ExecuteNext();
};

} // DP

///////////////////////////////////////////////////////////////////////////////

#endif Include_TRANSACTIONMANAGER_H

///////////////////////////////////////////////////////////////////////////////

