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
#include "DcrSetPrice.h"
#include "MainWindow_t.h"
#include "Eq2Broker_t.h"
#include "CommonTypes.h"
#include "ui_msg.h"
#include "DcrSetPrice.h"

namespace dp {
  result_code Dispatch(msg_t& msg) {
    result_code rc{ result_code::s_ok };
    if (msg.msg_name != dp::msg::name::kEventWapper) {
      LogInfo(L"dispatch(): unsupported message name, %S", msg.msg_name.c_str());
      rc = result_code::e_fail;
    }
    else {
      LogInfo(L"dispatching %S", msg.msg_name.c_str());
      auto& event_wrapper = msg.as<dp::msg::event_wrapper_base_t>();
      GetPipelineManager().SendEvent(*event_wrapper.get_event_data());
      ::Sleep(50);
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
    if (!array_equal(event.msg_name, Broker::Sell::txn::kTxnName)) {
      return S_FALSE;
    }
    GetPipelineManager().set_txn_executing(true);
    return S_OK;
  }

  HRESULT Handler_t::Stop(const DP::Event::Data_t& event) {
    LogInfo(L"TxSellItems::Stop()");
    if (!array_equal(event.msg_name, Broker::Sell::txn::kTxnName)) {
      return S_FALSE;
    }
    GetPipelineManager().set_txn_executing(false);
    return S_OK;
  }

  HRESULT Handler_t::MessageHandler(const DP::Message::Data_t* pMessage) {
    LogInfo(L"TxSellItems::MessageHandler, %S", pMessage->msg_name.data());
    // TODO this is all a bit haxy and needs some thought & cleanup
    // on multiple aisles probably
    if (!GetPipelineManager().txn_executing()) {
      LogInfo(L"  No txn executing");
      return S_FALSE;
    }
    dp::msg_ptr_t msg_ptr = Transform(*pMessage);
    if (!msg_ptr) {
      LogError(L"  Transform failed");
      return S_FALSE;
    }
    if (!tx_sellitems_.promise().txn_started()) {
      if (!array_equal(pMessage->msg_name, Sell::kMsgName)) {
        return S_FALSE;
      }
      return StartTxn(std::move(msg_ptr), CreateTxnState());
    }
    else {
      return SendMsgToTxn(std::move(msg_ptr));
    }
  }

  dp::msg_ptr_t Handler_t::Transform(const DP::Message::Data_t& msg) const {
    if (array_equal(msg.msg_name, Sell::kMsgName)) {
      using namespace Broker::Sell::Translate;
      return Legacy::Transform(reinterpret_cast<const Legacy::Data_t&>(msg));
    }
    else if (array_equal(msg.msg_name, SetPrice::kMsgName)) {
      using namespace Broker::SetPrice::Translate;
      return Legacy::Transform(reinterpret_cast<const Legacy::Data_t&>(msg));
    }
    else {
      return {};
    }
  }

  // hack. figure out txn vs. Transaction namespace issue
  using /*state_t = */Broker::Sell::txn::state_t;
  using state_ptr_t = dp::txn::start_t<state_t>::state_ptr_t;

  state_ptr_t Handler_t::CreateTxnState() {
    return std::make_unique<state_t>("agoblineye"s, 4);
  }

  HRESULT Handler_t::StartTxn(dp::msg_ptr_t msg_ptr, state_ptr_t state_ptr) {
    using namespace Broker::Sell;
    LogInfo(L"starting txn::sell_item");
    auto start_txn_msg = dp::txn::make_start_txn<txn::state_t>(txn::kTxnName,
      std::move(msg_ptr), std::move(state_ptr));
    return SendMsgToTxn(std::move(start_txn_msg));
  }

  HRESULT Handler_t::SendMsgToTxn(dp::msg_ptr_t msg_ptr) {
    //DP::Event::StopAcquire_t stop(DP::Event::Flag::Flush);
    //GetPipelineManager().SendEvent(stop);
    LogInfo(L"sending message to txn::sell_item");
    dp::msg_ptr_t out = tx_sellitems_.send_value(std::move(msg_ptr));
    if (out) {
      dp::Dispatch(*out.get());
      if ((tx_sellitems_.promise().txn_state() == dp::txn::state::ready)
        && tx_sellitems_.promise().result().succeeded())
      {
        LogInfo(L"TxSellItems::TXN_COMPLETE!");
        GetPipelineManager().set_txn_executing(false);
      }
      else {
        LogInfo(L"TxSellItems::Active because txn_state(%d), result(%d)",
          tx_sellitems_.promise().txn_state(), tx_sellitems_.promise().result());
      }
    }
    return S_OK;
  }
} // Broker::Transaction::SellItems