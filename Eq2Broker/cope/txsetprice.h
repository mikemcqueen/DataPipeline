#pragma once

#include "dp.h"
#include <string_view>

namespace setprice {
  namespace msg {
    constexpr std::string_view name{ "msg::set_price_popup" };

    struct data_t : public dp::msg::data_t {
      data_t(int p) : dp::msg::data_t(name), price(p) {}

      int price;
    };

    inline auto validate(const dp::msg_t& msg) {
      return dp::msg::validate<data_t>(msg, name);
    }
  }

  namespace txn {
    constexpr std::string_view name{ "txn::set_price" };

    struct state_t {
      std::string prev_msg_name; // i.e. "who called us"
      int price;
    };

    using start_t = dp::txn::start_t<state_t>;

    auto handler() -> dp::txn::handler_t;

    inline auto validate_start(const dp::msg_t& txn) {
      return dp::txn::validate_start<start_t, msg::data_t>(txn, name, msg::name);
    }
  }
}