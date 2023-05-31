#pragma once

#include <string_view>
#include "DcrBrokerSell.h"
#include "cope.h"
#include "txsellitem.h"

namespace Broker::SetPrice::txn {
  constexpr std::string_view kTxnName{ "txn::set_price" };

  struct state_t {
    std::string_view prev_msg_name; // i.e. "who called us"
    int price;
  };

  using start_t = cope::txn::start_t<state_t>;
  using handler_t = cope::txn::handler_t;

  inline auto start(handler_t::handle_t handle, handler_t::promise_type& promise,
    const Broker::Sell::txn::state_t& sell_state)
  {
    auto setprice_state = std::make_unique<state_t>(
      Broker::Sell::Translate::kMsgName, sell_state.item_price);
    return cope::txn::start_awaitable<state_t>{
      handle, std::move(promise.in_ptr()), std::move(setprice_state)
    };
  }

  auto handler() -> cope::txn::handler_t;
} // namespace Broker::SetPrice::txn