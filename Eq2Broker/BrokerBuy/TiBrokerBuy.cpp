////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TiBrokerBuy.cpp
//
// Broker Buy window text interpreter.
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TiBrokerBuy.h"
#include "DcrBrokerBuy.h"
//#include "TxGetItemsForSale.h"
#include "PipelineManager.h"
#include "BrokerId.h"
#include "TransactionManager.h"
#include "SsWindow.h"

namespace Broker::Buy::Interpret {
  Handler_t::Handler_t(const Window_t& window) : window_(window) {}

  bool Handler_t::Initialize(std::string_view name) {
    LogInfo(L"TiBrokerBuy::Initialize()");
    if (!DP::Handler_t::Initialize(name))
      return false;
    return true;
  }

#if 0
  void
    Handler_t::
    Shutdown()
  {
    LogAlways(L"TiBrokerBuy::Shutdown()");
    Transaction::GetItemsForSale::Handler_t::Shutdown();
  }
#endif

  HRESULT Handler_t::MessageHandler(const DP::Message::Data_t* pMessage) {
    if (Message::Id::Buy != pMessage->Id) {
      return S_FALSE;
    }
    auto& message = static_cast<const Translate::Legacy::Data_t&>(*pMessage);
    message.tableText.Dump(L"TiBrokerBuy");

    // if page number has changed, queue items to GetItemsForSale thread
    // may be some other restrictions, such as, exactly what transaction
    // is active

#if 0
    DP::Transaction::Data_t* pData = GetTransactionManager().Acquire();
    DP::TransactionManager_t::AutoRelease_t ar(pData);
    if (nullptr != pData)
    {
      using namespace Transaction;
      switch (pData->Id)
      {

      case Id::GetItemsForSale:
        return GetItemsForSale::Handler_t::Message(Message,
          static_cast<GetItemsForSale::Data_t&>(*pData),
          GetManager().GetWindow());
      default:
        break;
      }
    }
#endif
    return S_FALSE;
  }

} // Broker::Buy::Interpret
