// txsetprice.cpp

#include "stdafx.h"
#include "txsetprice.h"
#include "ui_msg.h"
#include "DcrSetPrice.h" // Types.h"
#include "DpEvent.h"
#include "BrokerUi.h"

namespace Broker::SetPrice::txn {
  using dp::result_code;
  using dp::txn::handler_t;
  using promise_type = handler_t::promise_type;

  auto validate_price(const dp::msg_t& msg, int price) {
    result_code rc = Translate::msg::validate(msg);
    if (succeeded(rc)) {
      const auto& sp_msg = msg.as<Translate::data_t>();
      if (sp_msg.price.GetPlat() != price) {
        LogError(L"setprice::validate_price, price mismatch: "
          "expected(%d), actual(%d)", price, sp_msg.price.GetPlat());
        rc = result_code::e_fail; // expected
      }
      else {
        LogInfo(L"setprice::validate_price, prices match(%d)", price);
      }
    }
    return rc;
  }

  auto validate_price(const promise_type& promise, int price) {
    return validate_price(promise.in(), price);
  }

  auto validate_complete(const promise_type& promise, std::string_view msg_name) {
    return dp::msg::validate_name(promise.in(), msg_name);
  }

  auto enter_price_text(int price) {
    using Ui::Event::SendChars::Data_t;

    std::string text = std::to_string(price).append("p");
    auto sc_event_ptr = std::make_unique<Data_t>(text, Broker::Window::Id::SetPrice,
      Ui::Widget::Id::Unknown, DP::Event::Flag::Synchronous);
    return std::make_unique<dp::msg::event_wrapper_t<Data_t>>(
      std::move(sc_event_ptr));
  }

  auto click_ok_button() {
    using Ui::Event::Click::Data_t;
    auto click_event_ptr = std::make_unique<Data_t>(Broker::Window::Id::SetPrice,
      Broker::SetPrice::Widget::Id::OkButton, DP::Event::Flag::Synchronous);
    return std::make_unique<dp::msg::event_wrapper_t<Data_t>>(
      std::move(click_event_ptr));
  }

  auto handler() -> handler_t {
    result_code rc{ result_code::s_ok };
    const auto& error = [&rc](result_code new_rc) noexcept {
      rc = new_rc;
      if (failed(rc)) { 
        LogError(L"  txn::setprice error(%d)", (int)rc);
        return true;
      }
      return false;
    };
    state_t state;

    while (true) {
      auto& promise = co_await dp::txn::receive_txn_awaitable{ kTxnName, state };
      while (true) {
        // TODO: I don't like this. validation errors should always continue?
        // can't we access the price? have it returned via &price param?
        // consider:
        // e_unexpected_msg_name
        // e_unexpected_msg_type
        // unexpected() (rc >= 0xf000)
        // error() -> capture_error()
        if (error(validate_price(promise, state.price))
          && (rc != result_code::e_unexpected))
        {
          co_yield enter_price_text(state.price);
          if (error(validate_price(promise, state.price))) continue;
        }
        if (rc != result_code::e_unexpected) {
          co_yield click_ok_button();
        }
        if (error(validate_complete(promise, state.prev_msg_name))) continue;
        break;
      }
      dp::txn::complete(promise, rc);
    }
  }
} // namespace Broker::SetPrice::Txn
