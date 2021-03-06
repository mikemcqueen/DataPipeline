////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// PipelineManager.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PipelineManager.h"
#include "TransactionManager.h"
#include "Macros.h"
#include "Log.h"

////////////////////////////////////////////////////////////////////////////////

DP::PipelineManager_t&
GetPipelineManager()
{
    return DP::PipelineManager_t::Get();
}

namespace DP
{

////////////////////////////////////////////////////////////////////////////////

/* static */
PipelineManager_t&
PipelineManager_t::
Get()
{
    static PipelineManager_t thePM;
    return thePM;
}

////////////////////////////////////////////////////////////////////////////////

bool
PipelineManager_t::
ProcessThreadData_t::
operator()(
    ThreadQueue::State_t State,
    Message::Data_t*     pData,
    PipelineManager_t*   pPM) const
{
    using namespace ThreadQueue;
    switch (State)
    {
    case State::Execute:
        pPM->Dispatch(pData);
        break;
    case State::Free:
        pPM->Release(pData);
        pPM->Free(pData);
        break;
    case State::Display:
/*
//        pPM->Display(pData);
        LogInfo(L"Display(%d, '%ls')", pData->Id, pData->szClass);
        if (DP::Acquire == pData->Stage)
        {
            DP::AcquireData_t ad = *(DP::AcquireData_t*)pData;
            if (DP::SurfacePoolItem == ad.Format)
            {
                LogInfo(L"    Acquire, SurfacePoolItem");
            }
            else
                LogInfo(L"    Acquire, Unknown(%d)", ad.Format);
        }
        else
            LogInfo(L"    Unknown(%d)", pData->Stage);
*/
        break;
    case State::Startup:
    case State::Shutdown:
        break;
    default:
        ASSERT(false);
        break;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
PipelineManager_t::
CompareMessage_t::
operator()(
    const Message::Data_t* pD1,
    const Message::Data_t* pD2) const
{
    if (0 == (pD1->Stage & pD2->Stage))
        return false;
    if (0 != wcscmp(pD1->Class, pD2->Class))
        return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////////

PipelineManager_t::
PipelineManager_t()
{
    m_MessageThread.m_LogLevel = ThreadQueue::High;
}

////////////////////////////////////////////////////////////////////////////////

PipelineManager_t::
~PipelineManager_t()
{
}

////////////////////////////////////////////////////////////////////////////////

bool
PipelineManager_t::
Initialize()
{
    return m_MessageThread.Initialize(this, L"PM::MessageThread", false);
}

////////////////////////////////////////////////////////////////////////////////

void
PipelineManager_t::
Shutdown()
{
    m_MessageThread.Shutdown();
}

////////////////////////////////////////////////////////////////////////////////

size_t
PipelineManager_t::
StartAcquiring(
    const wchar_t* /*pszClass*/)
{
    Event::StartAcquire_t eventStart;
    return SendEvent(eventStart);
}

////////////////////////////////////////////////////////////////////////////////

size_t
PipelineManager_t::
StopAcquiring(
    const wchar_t* /*pszClass*/)
{
    Event::StopAcquire_t eventStop;
    return SendEvent(eventStop);
}

////////////////////////////////////////////////////////////////////////////////

class StageNameMap_t :
    public map<Stage_t, const wstring>
{
public:

    const wstring&
    GetStageName(
        Stage_t stage)
    {
        if (empty())
        {
            Init();
        }
        const_iterator it = find(stage);
        if (end() != it)
        {
            return it->second;
        }
        else
        {
            // TODO: if 1 or 0 bit set return "unknown" else return "multiple"
            static const wstring strUnknown(L"Unknown");
            return strUnknown;
        }
    }

private:

    void
    Init()
    {
        using namespace Stage;
        insert(make_pair(None,      wstring(L"None")));
        insert(make_pair(Acquire,   wstring(L"Acquire")));
        insert(make_pair(Translate, wstring(L"Translate")));
        insert(make_pair(Interpret, wstring(L"Interpret")));
        insert(make_pair(Analyze,   wstring(L"Analyze")));
        insert(make_pair(Execute,   wstring(L"Execute")));
        insert(make_pair(Any,       wstring(L"Any")));
    }
};

////////////////////////////////////////////////////////////////////////////////

/* static */
const wchar_t*
PipelineManager_t::
GetStageString(
    Stage_t stage)
{
    static StageNameMap_t stageNameMap;
    return stageNameMap.GetStageName(stage).c_str();
}

////////////////////////////////////////////////////////////////////////////////
//
// Assumes m_Handlers is locked on entry.
//

bool
PipelineManager_t::
GetNextHandler(
          Stage_t                          Stage,
    const wchar_t*                         pszClass,
          HandlerVector_t::const_iterator& it) const
{
    while (m_Handlers.end() != it)
    {
        if ((0 != (it->Stage & Stage)) &&
            ((NULL == pszClass) || (0 == it->pHandler->GetClass().compare(pszClass))))
        {
            return true;
        }
        ++it;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

void
PipelineManager_t::
AddHandler(
          Stage_t    Stage,
          Handler_t& Handler,
    const wchar_t*   pszClass)
{
    if (Handler.Initialize(pszClass))
    {
        HandlerData_t hd(Stage, &Handler);
        CLock lock(m_csHandlers);
        m_Handlers.push_back(hd);
        return;
    }
    LogError(L"PM::AddHandler(%d): Handler.Initialize() failed", Stage);
    throw runtime_error("PM::AddHandler()");
}

////////////////////////////////////////////////////////////////////////////////

void
PipelineManager_t::
AddTransactionHandler(
    Stage_t         stage,
    TransactionId_t transactionId,
    Handler_t&      handler,
    const wchar_t*  displayName /*= NULL*/)
{
    bool added = false;
    if (handler.Initialize(NULL))
    {
        // NOTE: obviously this only allows for one handler per transaction Id,
        // regardless of stage.
        // in the future, maybe we could allow different handlers for different stages?
        CLock lock(m_csHandlers);
        TxIdHandlerMap_t::const_iterator itFind = m_txHandlerMap.find(transactionId);
        if (m_txHandlerMap.end() == itFind)
        {
            wchar_t stringId[16];
            if (NULL == displayName)
            {
                swprintf_s(stringId, L"TX-0%x", transactionId);
                displayName = stringId;
            }
            HandlerData_t handlerData(stage, &handler, displayName);
            TxIdHandlerMap_t::_Pairib ibPair = m_txHandlerMap.insert(
                make_pair(transactionId, handlerData));
            added = ibPair.second;
            if (!added)
            {
                LogError(L"PM::AddTransactionHandler(): HandlerMap.insert() failed");
            }
        }
        else
        {
            LogError(L"PM::AddTransactionHandler(): Handler already registered for id (%d)",
                     transactionId);
        }
    }
    else
    {
        LogError(L"PM::AddTransactionHandler(): Handler.Initialize(%d, %d) failed",
                 stage, transactionId);
    }
    if (!added)
    {
        throw runtime_error("PM::AddTransactionHandler()");
    }
}

////////////////////////////////////////////////////////////////////////////////

size_t
PipelineManager_t::
SendEvent(
    Event::Data_t& Data)
{
    size_t Total = 0;
    size_t Handled = 0;
    CLock lock(m_csHandlers);
    HandlerVector_t::const_iterator it = m_Handlers.begin();
    const wchar_t* pClass = NULL;
    if (L'\0' != Data.Class[0])
    {
        pClass = Data.Class;
    }
    using namespace DP::Message;
    if (Type::Event != Data.Type)// && (Type::Transaction != Data.Type))
    {
        throw logic_error("PM::SendEvent(): Invalid message type");
    }
#if 0
    if (SUCCEEDED(TrySendTransactionEvent(Data)))
    {
        return 1;
    }
#endif
    while (GetNextHandler(Data.Stage, pClass, it))
    {
        ++Total;
        HRESULT hr = it->pHandler->EventHandler(Data);
        // S_FALSE means not handled, no error
        if (S_OK == hr)
        {
            ++Handled;
        }
        else if (FAILED(hr))
        {
            LogError(L"SendEvent(%d,%d) failed (%08x)", Data.Stage, Data.Id, hr);
        }
        ++it;
    }
    LogInfo(L"SendEvent(): Total (%d) Handled (%d)", Total, Handled);
    return Handled;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
PipelineManager_t::
SendTransactionEvent(
    Transaction::Data_t& Data,
    Transaction::Data_t* pPrevTxData /*= NULL*/)
{
    using namespace DP::Message;
    if (Type::Transaction != Data.Type)
    {
//        return E_FAIL;
        throw invalid_argument("PM::SendTransactionEvent");
    }
    if (sizeof(Transaction::Data_t) > Data.Size)
    {
        throw logic_error("PM::SendTransactionEvent(): Invalid data size");
    }
    CLock lock(m_csHandlers);
    TxIdHandlerMap_t::const_iterator it = m_txHandlerMap.find(Data.Id);
    if (m_txHandlerMap.end() != it)
    {
        using Transaction::Data_t;
        Handler_t* pHandler = it->second.pHandler;
        Data_t& TxData = static_cast<Data_t&>(Data);
        using namespace Transaction;
        switch (TxData.GetState())
        {
        case State::New:      return pHandler->ExecuteTransaction(TxData);
        case State::Complete: return pHandler->OnTransactionComplete(TxData);
        default:              return pHandler->ResumeTransaction(TxData, pPrevTxData);
        }
    }
    return S_FALSE;
    // LogError(L"PM::SendEvent(): Transaction event not handled");
    // throw logic_error("PM::SendEvent(): Transaction event not handled");
}

////////////////////////////////////////////////////////////////////////////////

void*
PipelineManager_t::
Alloc(
    size_t Size)
{
    return malloc(Size);
}

////////////////////////////////////////////////////////////////////////////////

void
PipelineManager_t::
Free(
    void* pMem)
{
    free(pMem);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
PipelineManager_t::
Callback(
    Message::Data_t* pMessage)
{
    if (NULL == pMessage)
    {
        throw std::invalid_argument("PipelineManager_t::Callback(): Invalid message");
    }
    switch (pMessage->Stage)
    {
    case Stage::Acquire:
    case Stage::Translate:
    case Stage::Interpret:
        break;
    default:
        throw std::invalid_argument("PipelineManager_t::Callback() Invalid stage");
    }
    m_MessageThread.QueueData(pMessage);
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

size_t
PipelineManager_t::
Flush(
          Stage_t   Stage,
    const wchar_t* pszClass)
{
    // TODO:
    // if (!SameThread()) SignalObjectAndWait(hFlush, hFlushNotify);
    Message::Data_t Data;
    memset(&Data, 0, sizeof(Data));
    Data.Stage = Stage;
    wcscpy_s(Data.Class, _countof(Data.Class), pszClass);
    size_t RemovedCount = m_MessageThread.RemoveAll(&Data);
    if (0 < RemovedCount)
    {
        LogInfo(L"PM::Flush(%d, '%ls'): Removed %d item(s)", Stage, pszClass, RemovedCount);
    }
    return RemovedCount;
}

////////////////////////////////////////////////////////////////////////////////

void
PipelineManager_t::
Dispatch(
    Message::Data_t* pMessage)
{
    Stage_t NextStage = Stage::None;
    switch (pMessage->Stage)
    {
    case Stage::Acquire:     NextStage = Stage::Translate; break;
    case Stage::Translate:   NextStage = Stage::Interpret; break;
    case Stage::Interpret:   NextStage = Stage::Analyze;   break;
    default:
        ASSERT(false);
        return;
    }
    if (SUCCEEDED(TrySendTransactionMessage(pMessage, NextStage)))
    {
        return;
    }
    // create list of output formats desired
    // walk list of translate handlers.  if a desired format is supported,
    //   translate and remove it from list
    // 
    size_t Total = 0;
    size_t Handled = 0;
    {
        CLock lock(m_csHandlers);
        HandlerVector_t::const_iterator it = m_Handlers.begin();
        while (GetNextHandler(NextStage, pMessage->Class, it))
        {
            // at this point, how do we know that TiText is the required 
            // output type? a handler can support multiple? we don't just 
            // do them all, we need to know if there are any Analyze/Compare 
            // handlers that take TiText, presumably, so we can walk back 
            // the dependency chain.
            // TODO: *pMessage
            ++Total;
            HRESULT hr = it->pHandler->MessageHandler(pMessage);
            if (SUCCEEDED(hr))
            {
                ++Handled;
            }
            if (E_ABORT == hr)
            {
                LogInfo(L"PM::Dispatch(%s) Aborted after (%d) handlers",
                        GetStageString(pMessage->Stage), Total);
                return;
            }
            ++it;
        }
    }
    if (0 == Total)
    {
        LogError(L"PM::Dispatch(%s) No handlers", GetStageString(pMessage->Stage));
    }
    else
    {
        LogInfo(L"PM::Dispatch(%s) Total (%d) Handled(%d)",
                GetStageString(pMessage->Stage), Total, Handled);
    }
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
PipelineManager_t::
TrySendTransactionMessage(
    Message::Data_t* pMessage,
    Stage_t          stage)
{
    HRESULT hr = E_FAIL;
    DP::TransactionManager_t::AutoRelease_t TxData(GetTransactionManager().Acquire());
    const DP::Transaction::Data_t* pTxData = TxData.get();
    if (NULL != pTxData)
    {
        // There is a transaction active: see if we have a handler registered
        // for the active transaction & specified stage
        CLock lock(m_csHandlers);
        TxIdHandlerMap_t::const_iterator it = m_txHandlerMap.find(pTxData->Id);
        if ((m_txHandlerMap.end() != it) && (it->second.Stage == stage))
        {
            hr = it->second.pHandler->MessageHandler(pMessage);
            if (SUCCEEDED(hr))
            {
                LogInfo(L"PM::TrySendTransactionMessage(): Message handled");
            }
            else
            {
                LogError(L"PM::TrySendTransactionMessage(): Message not handled");
            }
        }
    }
    return hr;
}

////////////////////////////////////////////////////////////////////////////////

void
PipelineManager_t::
Release(
    DP::Message::Data_t* pData)
{
    switch (pData->Stage)
    {
    case DP::Stage::Acquire:
        {
            // Haxoid
            pData->~Data_t();
/*
            const DP::AcquireData_t& ad = *(DP::AcquireData_t*)pData;
            switch (ad.Format)
            {
            case DP::SurfacePoolItem:
                {
                    // If AcquireData is available, find out if any TranslateHandlers
                    //  are running that process that form format
                    DP::AcquireSurfacePoolItem_t& item = *(DP::AcquireSurfacePoolItem_t*)pData;
                    item.pPoolItem->release();
                }
                break;

            default:
                ASSERT(false);
                break;
            }
*/
        }
        break;
    case DP::Stage::Translate:
    case DP::Stage::Interpret:
        break;
    default:
        LogError(L"PM::ReleaseData('%ls', %d)", pData->Class, pData->Stage);
        throw std::invalid_argument("PM::ReleaseData() invalid stage");
    }
}

////////////////////////////////////////////////////////////////////////////////

void
PipelineManager_t::
QueueApc(
    PAPCFUNC  ApcFunc,
    ULONG_PTR Param)
{
    DWORD dwRet = QueueUserAPC(ApcFunc, m_MessageThread.GetThread(), Param);
    if (0 == dwRet)
    {
        LogError(L"PM: QueueApc() failed, %d", GetLastError());
    }
}

////////////////////////////////////////////////////////////////////////////////

const wchar_t*
PipelineManager_t::
GetTransactionName(
    TransactionId_t txId) const
{
    static const wchar_t unknownName[] = L"[Unknown]";
    TxIdHandlerMap_t::const_iterator it = m_txHandlerMap.find(txId);
    return (m_txHandlerMap.end() != it) ? it->second.name.c_str()
                                        : unknownName;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace DP

////////////////////////////////////////////////////////////////////////////////
