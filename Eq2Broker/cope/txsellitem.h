#pragma once

#include "dp.h"
#include <optional>
#include <string_view>
#include <vector>
#include "DcrBrokerSell.h"

namespace Broker::Sell::txn {
  inline constexpr auto kTxnName{ "txn::sell_items" };

  struct state_t {
    std::string item_name;
    int item_price;
  };

  using start_t = dp::txn::start_t<state_t>;

  auto handler() -> dp::txn::handler_t;
} // namespace Broker::Sell::txn
