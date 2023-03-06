////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TxSellItems.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PipelineManager.h"
#include "TxSellItems.h"
#include "DcrBrokerSell.h"
#include "MainWindow_t.h"
#include "Eq2Broker_t.h"
//#include "UiEvent.h"
#include "ui_msg.h"

namespace dp {
  result_code Dispatch(const Msg_t& msg) {
    result_code rc{ result_code::success };
    if (!msg.msg_name.starts_with("ui::msg")) {
      LogInfo(L"dispatch(): unsupported message name, %S", msg.msg_name.c_str());
      rc = result_code::unexpected_error;
    }
    else {
      rc = ui::msg::dispatch(msg);
    }
    return rc;
  }
}

namespace Broker::Transaction::SellItems {
  Handler_t::Handler_t() :
    tx_sellitems_{ Broker::Sell::txn::handler() }
  {}

  HRESULT Handler_t::EventHandler(DP::Event::Data_t& event) {
    switch (event.Id) {
    case DP::Event::Id::Start: return Start(event);
    case DP::Event::Id::Stop:  return Stop(event);
    default:
      break;
    }
    return DP::Handler_t::EventHandler(event);
  }

  HRESULT Handler_t::Start(const DP::Event::Data_t& event) {
    LogInfo(L"TxSellItems::Start()");
    if (started_ || strcmp(event.msg_name.data(), Broker::Sell::txn::kTxnName) != 0) {
      return S_FALSE;
    }
    ++started_;
    return S_OK;
  }

  HRESULT Handler_t::Stop(const DP::Event::Data_t& event) {
    LogInfo(L"TxSellItems::Stop()");
    if (strcmp(event.msg_name.data(), Broker::Sell::txn::kTxnName) != 0) {
      return S_FALSE;
    }
    started_ = 0;
    return S_OK;
  }

  HRESULT Handler_t::MessageHandler(const DP::Message::Data_t* pMessage) {
    LogInfo(L"TxSellItems::MessageHandler, msg_name: %S", pMessage->msg_name.data());
    using namespace Broker::Sell;
    if (!started_ || (strcmp(pMessage->msg_name.data(), kMsgName) != 0)) {
      return S_FALSE;
    }
    using Translate::Legacy::Data_t;
    const Data_t& msg = reinterpret_cast<const Data_t&>(*pMessage);
    if (started_ == 1) {
      ++started_;
      return StartTxn(Transform(msg));
    }
    return SendMsgToTxn(Transform(msg));
  }

  dp::MsgPtr_t Handler_t::Transform(
    const Broker::Sell::Translate::Legacy::Data_t& msg) const {
    msg;
    return std::make_unique<Broker::Sell::Translate::Data_t>(1);
  }

  HRESULT Handler_t::StartTxn(dp::MsgPtr_t msg_ptr) {
    using namespace Broker::Sell;
    LogInfo(L"starting txn::sell_item");
    auto state = std::make_unique<txn::state_t>("magic beans"s, 2);
    auto start_txn_msg = dp::txn::make_start_txn<txn::state_t>(txn::kTxnName,
      std::move(msg_ptr), std::move(state));
    return SendMsgToTxn(std::move(start_txn_msg));
  }

  HRESULT Handler_t::SendMsgToTxn(dp::MsgPtr_t msg_ptr) {
    dp::MsgPtr_t out = tx_sellitems_.send_value(std::move(msg_ptr));
    if (out) {
      dp::Dispatch(*out.get());
    }
    return S_OK;
  }
} // Broker::Transaction::SellItems