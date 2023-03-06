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

namespace DP {

  ////////////////////////////////////////////////////////////////////////////////
  //
  // PipelineManager_t
  //
  ////////////////////////////////////////////////////////////////////////////////

  class PipelineManager_t {
    friend class TransactionManager_t;
//    friend class dp::PmParasite_t;
  public:
    struct ProcessThreadData_t
    {
      bool
        operator()(
          ThreadQueue::State_t State,
          Message::Data_t* pData,
          PipelineManager_t* pPm) const;
    };

  private:

    struct HandlerData_t {
      HandlerData_t() :
        HandlerData_t(Stage_t::None, nullptr)
      { }

      HandlerData_t(
        Stage_t stage,
        Handler_t* handler,
        std::string_view nm = {})
        :
        Stage(stage),
        pHandler(handler),
        name(nm)
      { }

      Stage_t Stage;
      Handler_t* pHandler;
      std::string name;
    };

    typedef vector<HandlerData_t>                HandlerVector_t;
    typedef map<TransactionId_t, HandlerData_t>  TxIdHandlerMap_t;

    struct CompareMessage_t final {
      bool operator()(
        const Message::Data_t* pD1,
        const Message::Data_t* pD2) const;
    };

    typedef ThreadQueue_t<Message::Data_t*,
      PipelineManager_t,
      ProcessThreadData_t,
      CompareMessage_t>  MessageThread_t;

    friend class MessageThread_t;

  public:

    static PipelineManager_t& Get();

    static std::wstring_view GetStageString(Stage_t stage);

    // Constructor/destructor

    PipelineManager_t();
    ~PipelineManager_t();

    bool Initialize();

    void Shutdown();

    void AddHandler(
      Stage_t    Stage,
      Handler_t& Handler,
      std::string_view msg_name = {});


    void AddTransactionHandler(
      Stage_t          stage,
      TransactionId_t  transactionId,
      Handler_t& handler,
      std::string_view displayName);

    size_t StartAcquiring(
      std::string_view msg_name = {},
      bool on_demand = false);

    size_t StopAcquiring(std::string_view msg_name = {});

    size_t SendEvent(
      Event::Data_t& Data);

    void* Alloc(size_t Size);

    void Free(void* pMem);

    HRESULT Callback(Message::Data_t* pData);

    size_t Flush(Stage_t Stage, std::string_view msg_name);

    void QueueApc(
      PAPCFUNC ApcFunc,
      ULONG_PTR Param);

    HANDLE GetIdleEvent() const { return m_MessageThread.GetIdleEvent(); }

    const char* GetTransactionName(TransactionId_t txId) const;

  private:

    void Dispatch(Message::Data_t* pMessage);

    void Release(Message::Data_t* pData);

    bool GetNextHandler(
      Stage_t Stage,
      std::string_view msg_name,
      HandlerVector_t::const_iterator& it) const;

    HRESULT TrySendTransactionMessage(
      Message::Data_t* pMessage,
      Stage_t stage);

    // TransactionManager internal call
    HRESULT SendTransactionEvent(
      Transaction::Data_t& Data,
      Transaction::Data_t* pLastTxData = nullptr);

  private:
    PipelineManager_t(const PipelineManager_t&) = delete;
    PipelineManager_t& operator=(const PipelineManager_t&) = delete;

    MessageThread_t  m_MessageThread;
    HandlerVector_t  m_Handlers;
    TxIdHandlerMap_t m_txHandlerMap;
    CAutoCritSec     m_csHandlers;
    bool acquire_on_demand_ = false;
  };

} // DP

DP::PipelineManager_t& GetPipelineManager();

#endif // Include_PIPELINEMANAGER_H
