#include "stdafx.h"
#include <optional>
#include "txsellitem.h"
#include "txsetprice.h"
#include "dp_msg.h"
#include "ui_msg.h"
#include "DcrBrokerSell.h"
#include "DcrSetPrice.h"
#include "BrokerSellTypes.h"

namespace Broker::Sell::txn {
  using namespace std::literals;

  using cope::msg_t;
  using cope::result_code;
  using cope::txn::handler_t;
  using promise_type = handler_t::promise_type;

  auto is_candidate_row(const Table::RowData_t& row, const state_t& state) {
    if (row.item_name == state.item_name) {
      if ((row.item_price.GetPlat() != state.item_price) || !row.item_listed) {
        return true;
      }
    }
    return false;
  }

  auto get_candidate_row(const Translate::data_t& msg, const state_t& state)
    -> std::optional<int>
  {
    for (size_t row_index{}; row_index < msg.rows.size(); ++row_index) {
      if (is_candidate_row(msg.rows[row_index], state)) {
        LogInfo(L"get_candidate_row(%d)", row_index);
        return std::optional<int>(row_index);
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

  struct row_data_t {
    const Table::RowData_t* row;
    int row_index;
  };

  auto get_row(const promise_type& promise, size_t row_index,
    const Table::RowData_t** row)
  {
    const Translate::data_t& msg = promise.in().as<Translate::data_t>();
    if (row_index < 0 || row_index >= msg.rows.size()) {
      // e_unexpected_state_change?
      // though, i suppose not. if an item "sold" while txn was in progress,
      // this could fire on a for-sale list refresh. it might even case the
      // scroll position to tweak a bit.  maybe a rare condition for current
      // use case, but I could imagine "realtime" market data that could 
      // change that fast. maybe it's the responsibility of whatever layer
      // sits on top of the data source to ensure the data comes in relatively
      // consistently on subsequent fetches during a txn.  at best, it's txn
      // specific. so, do whatever makes sense for sell_items, here.
      return result_code::e_fail;
    }
    *row = &msg.rows[row_index];
    return result_code::s_ok;
  }

  auto get_row_data(const promise_type& promise, const state_t& state,
    row_data_t& row_data)
  {
    auto row_index = get_candidate_row(promise, state).value_or(-1);
    if (row_index == -1) return result_code::e_fail;
    row_data.row_index = row_index;
    return get_row(promise, row_index, &row_data.row);
  }

  struct validate_row_options {
    bool selected{};
    bool price{};
    bool listed{};
  };

  auto validate_row(const promise_type& promise, const state_t& state,
    int row_index, validate_row_options options)
  {
    result_code rc = Translate::msg::validate(promise.in());
    if (failed(rc)) return rc;

    const Table::RowData_t* row_ptr;
    rc = get_row(promise, row_index, &row_ptr);
    if (failed(rc)) return rc;

    const Table::RowData_t& row = *row_ptr;
    if ((options.selected && !row.selected)
      || (options.listed && !row.item_listed)
      || (options.price && (row.item_price.GetPlat() != state.item_price)))
    {
      LogInfo(L"sellitem::validate_row failed, options: "
        L"selected(%d), listed(%d), price(%d) state_price(%d)",
        options.selected && !row.selected, options.listed && !row.item_listed,
        options.price && (row.item_price.GetPlat() != state.item_price), state.item_price);
      rc = result_code::e_fail;
    }
    return rc;
  }

  namespace yield_validate {
    using yield_fn_t = std::function<std::unique_ptr<DP::msg::event_wrapper_base_t>()>;
    using validate_fn_t = std::function<result_code(promise_type& promise)>;

    struct state_t {
      yield_fn_t yield_fn;
      validate_fn_t validate_fn;
    };

    auto handler() -> handler_t {
      state_t state;

      while (true) {
        auto& promise = co_await cope::txn::receive_awaitable{ "txn::yield_validate", state };
        const auto& error = [&promise](result_code rc) {
          return promise.set_result(rc).failed();
        };
        while (!promise.result().unexpected()) {
          co_yield state.yield_fn();
          if (!error(state.validate_fn(promise))) break;
        }
        cope::txn::complete(promise);
      }
    }

    auto start_txn(handler_t::handle_t handle, promise_type& promise,
      yield_fn_t yield_fn, validate_fn_t validate_fn)
    {
      promise;
      auto state = std::make_unique<state_t>(/*std::string(kMsgName),*/
        std::move(yield_fn), std::move(validate_fn));
      return cope::txn::start_awaitable<state_t>{ handle, {},//std::move(promise.in_ptr()),
        std::move(state) };
    }
  }

  namespace retry {
    using fn_t = std::function<result_code(promise_type& promise)>;

    struct state_t {
      int retries_remaining;
      fn_t fn;
    };

    auto handler() -> handler_t {
      state_t state;

      while (true) {
        auto& promise = co_await cope::txn::receive_awaitable{ "txn::retry", state };
        const auto& error = [&promise](result_code rc)
          { return promise.set_result(rc).failed(); };
        while (!promise.result().unexpected()) {
          if (!error(state.fn(promise)) || (state.retries_remaining-- < 1)) break;
          LogInfo(L" retry: yielding noop");
          co_yield cope::msg::make_noop();
        }
        cope::txn::complete(promise);
      }
    }

    auto start_txn(handler_t::handle_t handle, promise_type& promise, fn_t fn,
      int retry_count = 2)
    {
      auto state = std::make_unique<state_t>(retry_count, std::move(fn));
      return cope::txn::start_awaitable<state_t>{ handle,
        std::move(promise.in_ptr()), std::move(state) };
    }
  }

  auto click_table_row(const Rect_t& rect) {
    // maybe this should be a separate ui::msg, so we don't need to muck
    // with window stuff here. we could dynamic_cast current window to 
    // TableWindow, and call GetRowRect (or ClickRow directly). click_table_row maybe
    using Ui::Event::Click::Data_t;
    auto click_event_ptr = std::make_unique<Data_t>(Broker::Window::Id::BrokerSell,
      rect.Center(), DP::Event::Flag::Synchronous);
    return std::make_unique<DP::msg::event_wrapper_t<Data_t>>(
      std::move(click_event_ptr));
  }

  // todo generalize "click id" function
  auto click_setprice_button() {
    using Ui::Event::Click::Data_t;
    auto click_event_ptr = std::make_unique<Data_t>(Broker::Window::Id::BrokerSell,
      Broker::Sell::Widget::Id::SetPriceButton, DP::Event::Flag::Synchronous);
    return std::make_unique<DP::msg::event_wrapper_t<Data_t>>(
      std::move(click_event_ptr));
  }

  auto click_listitem_button() {
    using Ui::Event::Click::Data_t;
    auto click_event_ptr = std::make_unique<Data_t>(Broker::Window::Id::BrokerSell,
      Broker::Sell::Widget::Id::ListItemButton, DP::Event::Flag::Synchronous);
    return std::make_unique<DP::msg::event_wrapper_t<Data_t>>(
      std::move(click_event_ptr));
  }

  auto handler() -> handler_t {
    handler_t retry_handler{ retry::handler() };
    handler_t setprice_handler{ SetPrice::txn::handler() };
    state_t state;

    while (true) {
      auto& promise = co_await cope::txn::receive_awaitable{ kTxnName, state };
      const auto& error = [&promise](result_code rc) { return promise.set_result(rc).failed(); };
      // here we should consider adding some part of the first message to our
      // "state", such as the visible row range, or scroll position, or names
      // of items, or some combination of those. If that state changes in a 
      // subsequent message, it would trigger an "e_unexpected_state_change"
      // result, which should cause us to exit the transaction. the "do the
      // scrolly bits" outer transaction would be responsible for figuring out
      // how to recover from that state change.
      // see: comments in get_row() above

      while (!promise.result().unexpected()) {
        if (error(Sell::Translate::msg::validate(promise.in()))) break; // txn_complete(error)
        row_data_t row_data;
        if (error(get_row_data(promise, state, row_data))) break; // txn_complete(error)
        const auto& row = *row_data.row;

        if (!row.selected) {
          co_yield click_table_row(row.rect);
          if (error(validate_row(promise, state, row_data.row_index,
            { .selected{true} }))) continue;
        }

        if (row.item_price.GetPlat() != state.item_price) {
          co_yield click_setprice_button();
          using SetPrice::Translate::msg::validate;
          co_await retry::start_txn(retry_handler.handle(), promise,
            [](promise_type& promise) { return validate(promise.in()); });
          if (promise.result().failed()) continue;

          co_await SetPrice::txn::start(setprice_handler.handle(), promise, state);
          if (error(validate_row(promise, state, row_data.row_index,
            { .selected{true}, .price{true} }))) continue;
        }
  
        if (!row.item_listed) {
          co_yield click_listitem_button();
          if (error(validate_row(promise, state, row_data.row_index,
            { .selected{true}, .price{true}, .listed{true} }))) continue;
        }
        break; // txn_complete(success)
      }
      cope::txn::complete(promise);
    }
  }
} // namespace Broker::Sell:txn