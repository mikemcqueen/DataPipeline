#pragma once

#include "dp.h"
#include <string_view>
#include "DcrSetPrice.h"

namespace Broker::SetPrice::txn {
  constexpr std::string_view kTxnName{ "txn::set_price" };

  struct state_t {
    std::string prev_msg_name; // i.e. "who called us"
    int price;
  };

  using start_t = dp::txn::start_t<state_t>;

  auto handler() -> dp::txn::handler_t;

  inline auto validate_start(const dp::Msg_t& txn) {
    using namespace Broker::SetPrice;
    return dp::txn::validate_start<start_t, Translate::Data_t>(txn, kTxnName,
      Translate::kMsgName);
  }
} // namespace Broker::SetPrice::txn