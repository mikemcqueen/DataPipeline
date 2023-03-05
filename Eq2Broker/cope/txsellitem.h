#pragma once

#include "dp.h"
#include <optional>
#include <string_view>
#include <vector>
#include "DcrBrokerSell.h"

namespace Broker::Sell::txn {
  inline constexpr auto kTxnName{ "txn::sell_item"sv };

  struct state_t {
    std::string item_name;
    int item_price;
  };

  using start_t = dp::txn::start_t<state_t>;

  auto handler() -> dp::txn::handler_t;

  inline auto validate_start(const dp::Msg_t& txn) {
    return dp::txn::validate_start<start_t, Translate::Data_t>(txn, kTxnName,
      Translate::kMsgName);
  }
} // namespace Broker::Sell::txn
