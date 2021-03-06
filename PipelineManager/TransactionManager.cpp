///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TransactionManager.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TransactionManager.h"
#include "PipelineManager.h"
#include "DpTransaction.h"
#include "Log.h"

///////////////////////////////////////////////////////////////////////////////

//#define EXTRALOG

DP::TransactionManager_t&
GetTransactionManager()
{
    return DP::TransactionManager_t::Get();
}

namespace DP
{

///////////////////////////////////////////////////////////////////////////////

    namespace Transaction
    {
        void
        Data_t::Complete(
            Transaction::Error_t Error /*= Transaction::Error::None*/) const
        {
            GetTransactionManager().CompleteTransaction(Id, Error);
        }

        bool
        Data_t::Execute(
            bool fInterrupt)
        {
            return GetTransactionManager().ExecuteTransaction(this, fInterrupt);
        }

        void
        Data_t::SetState(
            State_t state)
        {
            if (State::Complete == State)
            {
                throw logic_error("TxData::SetState() Transaction already completed");
            }
            State = state;
            stateTimeout = 0;
            LogAlways(L"%s::SetState(%x)",
                      GetPipelineManager().GetTransactionName(Id), state);
        }
    } // Transaction

///////////////////////////////////////////////////////////////////////////////

/*static*/
TransactionManager_t&
TransactionManager_t::
Get()
{
    static DP::TransactionManager_t theTransactionManager;
    return theTransactionManager;
}

///////////////////////////////////////////////////////////////////////////////

TransactionManager_t::
TransactionManager_t()
{
}

///////////////////////////////////////////////////////////////////////////////

bool
TransactionManager_t::
ExecuteTransaction(
    Transaction::Data_t* pTxData,
    bool                 fInterrupt)
{
    return ExecuteTransaction(pTxData, NULL, fInterrupt);
}

///////////////////////////////////////////////////////////////////////////////

bool
TransactionManager_t::
ExecuteTransaction(
    Transaction::Data_t* pTxData,
    Transaction::Data_t* pPrevTxData,
    bool                 fInterrupt)
{
    if (NULL == pTxData)
    {
        throw invalid_argument("XM::ExecuteTransaction()");
    }
    LogInfo(L"%s::ExecuteTransaction()", GetPipelineManager().GetTransactionName(pTxData->Id));
    bool bRet = false;
    CLock lock(m_cs);
    if (SetTransactionExecuting(pTxData, fInterrupt))
    {
        ExecuteNotify(pPrevTxData);
        bRet = true;
    }
//    LogInfo(L"XM: --ExecuteTransaction(%x) State (%d)", pTxData->Id, pTxData->GetState());
    return bRet;
}

///////////////////////////////////////////////////////////////////////////////

void
TransactionManager_t::
ExecuteNotify(
    Transaction::Data_t* pPrevTxData)
{
    GetPipelineManager().QueueApc(ApcExecute, ULONG_PTR(pPrevTxData));
}

///////////////////////////////////////////////////////////////////////////////

void
TransactionManager_t::
CompleteTransaction(
    MessageId_t TransactionId,
    DWORD       Error)
{
    CLock lock(m_cs);
    Data_t* pData = GetTransactionExecuting();
    if (NULL == pData)
    {
        throw logic_error("XM::CompleteTransaction() No transaction executing");
    }
    Transaction::Data_t* pTxData = pData->pTxData;
    if (TransactionId != pTxData->Id)
    {
        throw invalid_argument("XM::CompleteTransaction() TransactionId invalid");
    }
    LogAlways(L"%s::CompleteTransaction() Error(%x)", 
              GetPipelineManager().GetTransactionName(pTxData->Id), Error);
    pTxData->Error = Error;
    pTxData->SetState(Transaction::State::Complete);
    GetPipelineManager().QueueApc(ApcComplete, ULONG_PTR(pTxData));
}

///////////////////////////////////////////////////////////////////////////////

/* static */
void
CALLBACK
TransactionManager_t::
ApcExecute(
    ULONG_PTR Param)
{
    TransactionManager_t& tm = GetTransactionManager();
    Transaction::Data_t* pPrevTxData = reinterpret_cast<Transaction::Data_t*>(Param);
    tm.SendEvent(L"ApcExecute()", pPrevTxData);
    delete pPrevTxData;
}

///////////////////////////////////////////////////////////////////////////////

/* static */
void
CALLBACK
TransactionManager_t::
ApcComplete(
    ULONG_PTR Param)
{
    TransactionManager_t& tm = GetTransactionManager();
    Transaction::Data_t* pTxData = reinterpret_cast<Transaction::Data_t*>(Param);
    if (tm.GetTransactionExecuting()->pTxData != pTxData)
    {
        // not sure if this is even a problem necessarily, but seems impossible
        LogError(L"XM: ApcComplete() pTxData != TransactionExecuting");
        //throw logic_error("XM::ApcComplete() pTxData != TransactionExecuting");
    }
    tm.SendEvent(L"ApcComplete()");
    tm.Release(pTxData);
}

///////////////////////////////////////////////////////////////////////////////

void
TransactionManager_t::
SendEvent(
    const wchar_t*       eventName,
    Transaction::Data_t* pPrevTxData /*= NULL*/)
{
    CLock lock(m_cs);
    Data_t* pData = GetTransactionExecuting();
    if (NULL == pData)
    {
        throw logic_error("XM::CompleteTransaction() No transaction executing");
    }
    Transaction::Data_t* pTxData = pData->pTxData;
    LogAlways(L"%s::%s State(%d)", GetPipelineManager().GetTransactionName(pTxData->Id),
              eventName, pTxData->GetState());
	HRESULT hr = GetPipelineManager().SendTransactionEvent(*pTxData, pPrevTxData);
    if (FAILED(hr))
    {
        LogError(L"XM::SendEvent() %s:%s Error(%08x) State(%d)",
                 GetPipelineManager().GetTransactionName(pTxData->Id),
                 eventName, hr, pTxData->GetState());
    }
}

///////////////////////////////////////////////////////////////////////////////

TransactionManager_t::Data_t*
TransactionManager_t::
GetTransactionExecuting()
{
    CLock lock(m_cs);
    if (!m_stack.empty())
    {
        return &m_stack.back();
    }
    else if (!m_queue.empty())
    {
        return &m_queue.front();
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

bool
TransactionManager_t::
SetTransactionExecuting(
    Transaction::Data_t* pData,
    bool                 fInterrupt)
{
    CLock lock(m_cs);
    bool emptyBefore = m_queue.empty();
    if (emptyBefore || !fInterrupt)
    {
        m_queue.push_back(Data_t(pData));
        return emptyBefore;
    }
    else
    {
        m_stack.push_back(Data_t(pData));
        return true;
    }
}

///////////////////////////////////////////////////////////////////////////////

Transaction::Data_t*
TransactionManager_t::
Acquire()
{
    CLock lock(m_cs);
    Data_t* pData = GetTransactionExecuting();
    if (NULL != pData)
    {
        ++pData->refCount;
#ifdef EXTRALOG
    LogAlways(L"XM: Acquire(%x) refCount(%d)", pData->pTxData->Id, pData->refCount);
#endif
        return pData->pTxData;
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

void
TransactionManager_t::
Release(
    Transaction::Data_t* pTxData,
    bool /*bQuiet*/)
{
    CLock lock(m_cs);
    Data_t* pData = FindTransaction(pTxData);
    if (NULL == pData)
    {
        throw logic_error("XM::Release() FindTransaction failed");
    }
    long refCount = --pData->refCount;
#ifdef EXTRALOG
    LogAlways(L"XM: Release() Id(%x) RefCount(%d)", pData->pTxData->Id, pData->refCount);
#endif
    if ((0 == refCount) && (GetTransactionExecuting() == pData))
    {
        ExecuteNext();
    }
}

///////////////////////////////////////////////////////////////////////////////

TransactionManager_t::Data_t*
TransactionManager_t::
FindTransaction(
    const Transaction::Data_t* pTxData)
{
    CLock lock(m_cs);
    if (!m_stack.empty())
    {
        Stack_t::reverse_iterator it = m_stack.rbegin();
        for (; m_stack.rend() != it; ++it)
        {
            if (pTxData == it->pTxData)
            {
                return &*it;
            }
        }
    }
    if (!m_queue.empty())
    {
        Queue_t::iterator it = m_queue.begin();
        for (; m_queue.end() != it; ++it)
        {
            if (pTxData == it->pTxData)
            {
                return &*it;
            }
        }
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

void
TransactionManager_t::
ExecuteNext()
{
    Transaction::Data_t* pPrevTxData = NULL;
    CLock lock(m_cs);
    if (!m_stack.empty())
    {
        Data_t data = m_stack.back();
        m_stack.pop_back();
        if (0 != data.refCount)
        {
            LogWarning(L"XM::ExecuteNext() Current stack transaction refCount non-zero (%d)",
                       data.refCount);
        }
        LogAlways(L"XM: PopStack(%d)", m_stack.size());
        // pre-emptive (stack-based) transactions may send their txData to 
        // ResumeTransaction() of the transaction they pre-empted, so don't
        // delete it yet; gets deleted in ApcExecute.
        pPrevTxData = data.pTxData;
    }
    else if (!m_queue.empty())
    {
        Data_t data = m_queue.front();
        m_queue.pop_front();
        if (0 != data.refCount)
        {
            LogWarning(L"XM::ExecuteNext() Current queue transaction refCount non-zero (%d)",
                       data.refCount);
        }
        LogAlways(L"XM: PopQueue(%d)", m_queue.size());
        delete data.pTxData;
    }
    else
    {
        throw logic_error("XM::ExecuteNext() Nothing to execute!");
    }
    // if one of the containers is non-empty, we have a newly active transaction
    if (!m_stack.empty() || !m_queue.empty())
    {
        // notify the transaction it is active
        ExecuteNotify(pPrevTxData);
    }
}

///////////////////////////////////////////////////////////////////////////////

} // DP
