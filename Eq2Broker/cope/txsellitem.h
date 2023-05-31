#pragma once

#include "cope.h"
#include <optional>
#include <string_view>
#include <vector>

namespace Broker::Sell::txn {
  inline constexpr std::string_view kTxnName{ "txn::sell_item" };

  struct state_t {
    std::string item_name;
    int item_price;
  };

  using start_t = cope::txn::start_t<state_t>;

  auto handler() -> cope::txn::handler_t;
} // namespace Broker::Sell::txn
