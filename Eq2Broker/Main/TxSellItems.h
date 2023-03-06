/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TxSellItems.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TXSELLITEMS_H
#define Include_TXSELLITEMS_H

#include "DpHandler_t.h"
#include "DpEvent.h"
#include "BrokerSellTypes.h"
#include "txsellitem.h"

class Eq2Broker_t;

namespace Broker::Transaction::SellItems {
  struct StartEvent_t : DP::Event::Data_t {
    StartEvent_t() : DP::Event::Data_t(
      DP::Stage_t::Interpret,
      DP::Event::Id::Start,
      sizeof(StartEvent_t),
      0,
      Broker::Sell::txn::kTxnName) {}
  };

  class Handler_t : public DP::Handler_t {
  public:
    Handler_t(); //  Eq2Broker_t& broker);
//    Handler_t() = delete;

    // DP::Handler_t virtual
    HRESULT MessageHandler(const DP::Message::Data_t* pData) override;
    HRESULT EventHandler(DP::Event::Data_t& Data) override;

  private:
    HRESULT Start(const DP::Event::Data_t& event);
    HRESULT Stop(const DP::Event::Data_t & event);
    HRESULT StartTxn(dp::MsgPtr_t msg_ptr);
    HRESULT SendMsgToTxn(dp::MsgPtr_t msg_ptr);
    dp::MsgPtr_t Transform(const Broker::Sell::Translate::Legacy::Data_t& msg) const;

    dp::txn::handler_t tx_sellitems_;
    int started_ = 0;
  };

} // namespace Broker::Transaction::SellItems

#endif // Include_TXSELLITEMS_H