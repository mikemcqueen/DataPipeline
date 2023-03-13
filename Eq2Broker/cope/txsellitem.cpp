#include "stdafx.h"
#include <optional>
#include "txsellitem.h"
#include "txsetprice.h"
#include "ui_msg.h"
#include "BrokerSellTypes.h"

namespace Broker::Sell::txn {
  using namespace std::literals;

  using dp::msg_t;
  using dp::result_code;
  using dp::txn::handler_t;
  using promise_type = handler_t::promise_type;

  auto get_row(const promise_type& promise, size_t row_index,
    const Table::RowData_t** row)
  {
    const Translate::data_t& msg = promise.in().as<Translate::data_t>();
    if (row_index < 0 || row_index >= msg.rows.size()) {
      return dp::result_code::e_fail; // expected
    }
    *row = &msg.rows[row_index];
    return dp::result_code::s_ok;
  }

  auto is_candidate_row(const Translate::data_t& msg, const state_t& state,
    int row_index)
  {
    auto& row = msg.rows[row_index];
    if (row.item_name == state.item_name) {
      if ((row.item_price.GetPlat() != state.item_price)
        || !row.item_listed)
      {
        return true;
      }
    }
    return false;
  }

  auto get_candidate_row(const Translate::data_t& msg, const state_t& state)
    -> std::optional<int>
  {
    using result_t = std::optional<int>;
    for (size_t row{}; row < msg.rows.size(); ++row) {
      if (is_candidate_row(msg, state, row)) {
        LogInfo(L"get_candidate_row(%d)", row);
        return result_t(row);
      }
    }
    LogInfo(L"get_candidate_row(none)");
    return std::nullopt;
  }

  auto get_candidate_row(const promise_type& promise, const txn::state_t& state)
    -> std::optional<int>
  {
    return get_candidate_row(promise.in().as<Translate::data_t>(), state);
  }

  struct validate_row_options {
    bool selected = false;
    bool price = false;
    bool listed = false;
  };

  auto validate_row(const promise_type& promise, int row_index,
    const state_t& state, const Table::RowData_t** out_row,
    validate_row_options options)
  {
    result_code rc = Translate::msg::validate(promise.in());
    if (failed(rc)) return rc;

    rc = get_row(promise, row_index, out_row);
    if (failed(rc)) return rc;

    const Table::RowData_t& row = **out_row;
    if ((options.selected && !row.selected)
      || (options.listed && !row.item_listed)
      || (options.price && (row.item_price.GetPlat() != state.item_price)))
    {
      LogInfo(L"sellitem::validate_row failed, options: "
        L"selected(%d), listed(%d), price(%d)",
        options.selected && !row.selected, options.listed && !row.item_listed,
        options.price && (row.item_price.GetPlat() != state.item_price));
      rc = result_code::e_fail;
    }
    return rc;
  }

  auto start_txn_setprice(handler_t::handle_t handle, promise_type& promise,
    const state_t& sell_state)
  {
    // build a SetPrice txn state that contains a Broker::Sell::Translate msg
    auto setprice_state = std::make_unique<SetPrice::txn::state_t>(
      std::string(kMsgName), sell_state.item_price);
    return dp::txn::start_txn_awaitable<SetPrice::txn::state_t>{
      handle, std::move(promise.in_ptr()), std::move(setprice_state)
    };
  }

  auto click_table_row(const Rect_t& rect) {
    // maybe this should be a separate ui::msg, so we don't need to muck
    // with window stuff here. we could dynamic_cast current window to 
    // TableWindow, and call GetRowRect (or ClickRow directly). click_table_row maybe
    using Ui::Event::Click::Data_t;
    auto click_event_ptr = std::make_unique<Data_t>(Broker::Window::Id::BrokerSell,
      rect.Center(), DP::Event::Flag::Synchronous);
    return std::make_unique<dp::msg::event_wrapper_t<Data_t>>(
      std::move(click_event_ptr));
  }

  auto click_setprice_button() {
    using Ui::Event::Click::Data_t;
    auto click_event_ptr = std::make_unique<Data_t>(Broker::Window::Id::BrokerSell,
      Broker::Sell::Widget::Id::SetPriceButton, DP::Event::Flag::Synchronous);
    return std::make_unique<dp::msg::event_wrapper_t<Data_t>>(
      std::move(click_event_ptr));
  }

  auto click_listitem_button() {
    using Ui::Event::Click::Data_t;
    auto click_event_ptr = std::make_unique<Data_t>(Broker::Window::Id::BrokerSell,
      Broker::Sell::Widget::Id::ListItemButton, DP::Event::Flag::Synchronous);
    return std::make_unique<dp::msg::event_wrapper_t<Data_t>>(
      std::move(click_event_ptr));
  }

  auto handler() -> handler_t {
    result_code rc{ result_code::s_ok };
    const auto& error = [&rc](result_code new_rc) noexcept {
      rc = new_rc;
      if (dp::failed(rc)) {
        LogError(L"  txn::sellitem::onepage_handler error(%d)", (int)rc);
      }
      return dp::failed(rc);
    };
    dp::txn::handler_t setprice_handler{ SetPrice::txn::handler() };
    state_t state;

    while (true) {
      auto& promise = co_await dp::txn::receive_txn_awaitable{ kTxnName, state };

      while (true) {//rc != result_code::e_unexpected) {
        auto opt_row_index = get_candidate_row(promise, state);
        if (!opt_row_index.has_value()) break;
        auto row_index = opt_row_index.value();

        const Table::RowData_t* row;
        if (error(get_row(promise, row_index, &row))) continue;

        if (!row->selected) {
          co_yield click_table_row(row->rect);
          if (error(validate_row(promise, row_index, state, &row,
            { .selected{true} }))) continue;
        }

        if (row->item_price.GetPlat() != state.item_price) {
          co_yield click_setprice_button();
          if (error(SetPrice::Translate::msg::validate(promise.in()))) continue;

          co_await start_txn_setprice(setprice_handler.handle(), promise, state);
          if (error(validate_row(promise, row_index, state, &row,
            { .selected{true}, .price{true} }))) continue;
        }

        if (!row->item_listed) {
          co_yield click_listitem_button();
          if (error(validate_row(promise, row_index, state, &row,
            { .selected{true}, .price{true}, .listed{true} }))) continue;
        }
      }
      dp::txn::complete(promise, rc);
    }
  }
} // namespace Broker::Sell:txn
