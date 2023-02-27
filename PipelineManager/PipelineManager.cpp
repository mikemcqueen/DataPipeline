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
#include "CommonTypes.h"
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
  Message::Data_t* pData,
  PipelineManager_t* pPM) const
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
    //[[fallthrough]]
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
    if (0 == (intValue(pD1->Stage) & intValue(pD2->Stage)))
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

class StageNameMap_t final :
    public std::unordered_map<Stage_t, std::wstring_view>
{
    using value_type = std::wstring_view;
    using base_type = std::unordered_map<Stage_t, value_type>;

public:

    value_type
    GetStageName(
        Stage_t stage)
    {
        const_iterator it = find(stage);
        if (end() != it) {
            return it->second;
        } else {
            // TODO: if 1 or 0 bit set return "unknown" else return "multiple"
            // TODO: add "unknown" entry to map
            return L"Unknown";
        }
    }

private:
    friend class PipelineManager_t; // PM calls make_new()

    static
    StageNameMap_t
    make_new()
    {
        StageNameMap_t m;
        m.insert(make_pair(Stage_t::None,      L"None"));
        m.insert(make_pair(Stage_t::Acquire,   L"Acquire"));
        m.insert(make_pair(Stage_t::Translate, L"Translate"));
        m.insert(make_pair(Stage_t::Interpret, L"Interpret"));
        m.insert(make_pair(Stage_t::Analyze,   L"Analyze"));
        m.insert(make_pair(Stage_t::Execute,   L"Execute"));
        m.insert(make_pair(Stage_t::Any,       L"Any"));
        return m;
    }
};

////////////////////////////////////////////////////////////////////////////////

/* static */
wstring_view
PipelineManager_t::
GetStageString(
    Stage_t stage)
{
    static StageNameMap_t stageNameMap = StageNameMap_t::make_new();
    return stageNameMap.GetStageName(stage);
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
    // TODO: revisit the entirety of this opaque logic
    while (m_Handlers.end() != it) {
        if ((0 != (intValue(it->Stage) & intValue(Stage)))) // todo template func
            // TODO: revisit this class nonsense when necessary. maybe
            // consider adding an "application" or "domain" message::Data
            // member in addition to "class".
            // 
            // && ((nullptr == pszClass) || (0 == it->pHandler->GetClass().compare(pszClass))))
        {
            pszClass;
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
    if (!Handler.Initialize(pszClass)) {
        LogError(L"PM::AddHandler(%d): Handler.Initialize() failed", Stage);
        throw runtime_error("PM::AddHandler()");
    }
    HandlerData_t hd(Stage, &Handler);
    CLock lock(m_csHandlers);
    m_Handlers.push_back(hd);
}

////////////////////////////////////////////////////////////////////////////////

void
PipelineManager_t::
AddTransactionHandler(
  Stage_t         stage,
  TransactionId_t transactionId,
  Handler_t& handler,
  const wchar_t* displayName /*= nullptr*/)
{
  if (handler.Initialize(nullptr)) {
    // NOTE: obviously this only allows for one handler per transaction Id,
    // regardless of stage.
    // in the future, maybe we could allow different handlers for different stages?
    CLock lock(m_csHandlers);
    TxIdHandlerMap_t::const_iterator itFind = m_txHandlerMap.find(transactionId);
    if (m_txHandlerMap.end() == itFind) {
      wchar_t stringId[16];
      if (nullptr == displayName) {
        swprintf_s(stringId, L"TX-0%x", transactionId);
        displayName = stringId;
      }
      HandlerData_t handlerData(stage, &handler, displayName);
      auto [_, added] = m_txHandlerMap.insert(
        make_pair(transactionId, handlerData));
      if (added) {
        return; // Success
      }
      else {
        LogError(L"PM::AddTransactionHandler(): HandlerMap.insert() failed");
      }
    }
    else {
      LogError(L"PM::AddTransactionHandler(): Handler already registered for id (%d)",
        transactionId);
    }
  }
  else {
    LogError(L"PM::AddTransactionHandler(): Handler.Initialize(%d, %d) failed",
      stage, transactionId);
  }
  throw runtime_error("PM::AddTransactionHandler()");
}

////////////////////////////////////////////////////////////////////////////////

size_t
PipelineManager_t::
SendEvent(
  Event::Data_t& Data)
{
  if (DP::Message::Type::Event != Data.Type) { // && (Type::Transaction != Data.Type))
    throw invalid_argument("PM::SendEvent(): Invalid message type");
  }

  size_t Total = 0;
  size_t Handled = 0;
  CLock lock(m_csHandlers);
  const wchar_t* pClass = nullptr;
  if (L'\0' != Data.Class[0]) {
    pClass = Data.Class;
  }
  //using namespace DP::Message;
#if 0
  if (SUCCEEDED(TrySendTransactionEvent(Data)))
  {
    return 1;
  }
#endif
  HandlerVector_t::const_iterator it = m_Handlers.begin();
  while (GetNextHandler(Data.Stage, pClass, it)) {
    ++Total;
    HRESULT hr = it->pHandler->EventHandler(Data);
    // S_FALSE means not handled, no error
    if (S_OK == hr) {
      ++Handled;
    }
    else if (FAILED(hr)) {
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
  Transaction::Data_t* pPrevTxData /*= nullptr*/)
{
  //using namespace DP::Message;
  if (DP::Message::Type::Transaction != Data.Type) {
    //        return E_FAIL;
    throw invalid_argument("PM::SendTransactionEvent(): Invalid message type");
  }
  if (Data.Size < sizeof(Transaction::Data_t)) {
    throw invalid_argument("PM::SendTransactionEvent(): Invalid data size");
  }
  CLock lock(m_csHandlers);
  TxIdHandlerMap_t::const_iterator it = m_txHandlerMap.find(Data.Id);
  if (m_txHandlerMap.end() != it) {
    using Transaction::Data_t;
    Handler_t* pHandler = it->second.pHandler;
    Data_t& TxData = static_cast<Data_t&>(Data);
    using namespace Transaction;
    switch (TxData.GetState()) {
    case State::New:
      return pHandler->ExecuteTransaction(TxData);

    case State::Complete:
      return pHandler->OnTransactionComplete(TxData);

    default:
      return pHandler->ResumeTransaction(TxData, pPrevTxData);
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
  LogInfo(L"PipelineManager_t::Callback pMessage %S", typeid(*pMessage).name());

  if (nullptr == pMessage) {
    throw std::invalid_argument("PipelineManager_t::Callback(): Invalid message");
  }
  switch (pMessage->Stage) {
  case Stage_t::Acquire:
  case Stage_t::Translate:
  case Stage_t::Interpret:
    LogInfo(L"PM::Callback (%s)", GetStageString(pMessage->Stage));
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
  if (0 < RemovedCount) {
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
  Stage_t NextStage = Stage_t::None;
  switch (pMessage->Stage) {
  case Stage_t::Acquire:     NextStage = Stage_t::Translate; break;
  case Stage_t::Translate:   NextStage = Stage_t::Interpret; break;
  case Stage_t::Interpret:   NextStage = Stage_t::Analyze;   break;
  default:
    ASSERT(false);
    return;
  }
  if (SUCCEEDED(TrySendTransactionMessage(pMessage, NextStage))) {
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
    while (GetNextHandler(NextStage, pMessage->Class, it)) {
      // at this point, how do we know that TiText is the required 
      // output type? a handler can support multiple? we don't just 
      // do them all, we need to know if there are any Analyze/Compare 
      // handlers that take TiText, presumably, so we can walk back 
      // the dependency chain.
      // TODO: *pMessage
      ++Total;
      HRESULT hr = it->pHandler->MessageHandler(pMessage);
      if (SUCCEEDED(hr)) {
        ++Handled;
      }
      if (E_ABORT == hr) {
        LogInfo(L"PM::Dispatch(%s) Aborted after (%d) handlers",
          GetStageString(pMessage->Stage), Total);
        return;
      }
      ++it;
    }
  }
  if (0 == Total) {
    LogError(L"PM::Dispatch(%s) No handlers", GetStageString(pMessage->Stage));
  }
  else {
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
  if (nullptr != pTxData) {
    // There is a transaction active: see if we have a handler registered
    // for the active transaction & specified stage
    CLock lock(m_csHandlers);
    TxIdHandlerMap_t::const_iterator it = m_txHandlerMap.find(pTxData->Id);
    if ((m_txHandlerMap.end() != it) && (it->second.Stage == stage)) {
      hr = it->second.pHandler->MessageHandler(pMessage);
      if (SUCCEEDED(hr)) {
        LogInfo(L"PM::TrySendTransactionMessage(): Message handled");
      }
      else {
        LogError(L"PM::TrySendTransactionMessage(): Message not handled");
      }
    }
  }
  return hr;
}

////////////////////////////////////////////////////////////////////////////////

int release = 0;
int releaseFn = 0;

void
PipelineManager_t::
Release(
  DP::Message::Data_t* pData)
{
  release++;
  if (nullptr != pData->ReleaseFn) {
    releaseFn++;
    pData->ReleaseFn(*pData);
  }

#if 0
  switch (pData->Stage)
  {
  case DP::Stage_t::Acquire:
  {
    // Haxoid
    //pData->~Data_t();

    // TOOD: move to outside switch;

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

  case DP::Stage_t::Translate:
  case DP::Stage_t::Interpret:
    break;

  default:
    LogError(L"PM::ReleaseData('%ls', %d)", pData->Class, pData->Stage);
    throw std::invalid_argument("PM::ReleaseData() invalid stage");
  }
#endif
}

////////////////////////////////////////////////////////////////////////////////

void
PipelineManager_t::
QueueApc(
    PAPCFUNC  ApcFunc,
    ULONG_PTR Param)
{
    DWORD dwRet = QueueUserAPC(ApcFunc, m_MessageThread.GetThread(), Param);
    if (0 == dwRet) {
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

} // namespace DP
