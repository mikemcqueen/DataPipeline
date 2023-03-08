#pragma once

#include "dp.h"
#include <string_view>
#include "DcrSetPrice.h"

namespace Broker::SetPrice::txn {
  constexpr auto kTxnName{ "txn::set_price" };

  struct state_t {
    std::string prev_msg_name; // i.e. "who called us"
    int price;
  };

  using start_t = dp::txn::start_t<state_t>;

  auto handler() -> dp::txn::handler_t;
} // namespace Broker::SetPrice::txn