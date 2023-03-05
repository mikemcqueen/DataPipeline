#include "stdafx.h"
#include <cassert>
#include <chrono>
#include <iostream>
#include <format>
#include <string>
#include "cope.h"
#include "dp.h"
#include "log.h"
#include "txsetprice.h"
#include "txsellitem.h"
#include "ui_msg.h" // dispatch, or "register_dispatch"
#include "BrokerUi.h"
//#include "Eq2UiIds.h"

using namespace std::literals;

namespace dp {
  result_code dispatch(const Msg_t& msg) {
    result_code rc{ result_code::success };
    if (!msg.msg_name.starts_with("ui::msg")) {
      LogInfo(L"dispatch(): unsupported message name, %S", msg.msg_name.c_str());
      rc = result_code::unexpected_error;
    } else {
      rc = ui::msg::dispatch(msg);
    }
    return rc;
  }
}

dp::MsgPtr_t start_txn_sellitem(dp::MsgPtr_t msg_ptr) {
  using namespace Broker::Sell;
  LogInfo(L"starting txn::sell_item");
  // TODO: would like to allow this and build a unique_ptr from it
  // sellitem::txn::state_t state{ "some item", 1 };
  auto state = std::make_unique<txn::state_t>("magic beans"s, 2);
  return std::move(dp::txn::make_start_txn<txn::state_t>(txn::kTxnName,
    std::move(msg_ptr), std::move(state)));
}

auto screenshot() {
  return std::make_unique<dp::Msg_t>("dp::msg::screenshot");
}

dp::MsgPtr_t translate(dp::MsgPtr_t msg_ptr, std::string& out_msg,
  std::string& out_extra)
{
  msg_ptr;
  using namespace Broker::Sell;

  static Table::RowData_t test1{
    "magic beans"s, 1, { 1 }, false, false
  };

  static Table::RowVector rows_page_1{
    //{ "magic balls", 1, 7, false, false },
    { "magic beans", 1, { 1 }, false, false },
#if 0
    { "magic beans", 1, 1, false, false },
    { "magic beans", 1, 1, true, false },
    { "magic beans", 1, 1, false, true },
    { "magic beans", 1, 1, true, true },

    { "magic balls", 1, 7, false, false },

    { "magic beans", 1, 2, false, false },
    { "magic beans", 1, 2, true, false},
    { "magic beans", 1, 2, false, true },
    { "magic beans", 1, 2, true, true },

    { "magic balls", 1, 8, false, false },
    { "magic beans", 1, 3, false, true },
    { "magic beans", 1, 5, true, false },
    { "magic balls", 1, 9, false, false }
#endif
  };
  static bool first = true;
  static bool in_setprice = false;
  static bool setprice_clicked = false;
  static bool listed_clicked = false;
  static bool final_message_sent = false;
  static size_t index = 0;
  bool xtralog = true;

  out_msg.clear();
  out_extra.clear();
  //row_data_t prev_row = rows_page_1[index];
  for (; index < rows_page_1.size(); ++index,
    first = true,
    setprice_clicked = false,
    in_setprice = false,
    listed_clicked = false)
  {
    auto& row = rows_page_1[index];

    if (row.item_name != "magic beans") continue;
    if (row.item_price.GetPlat() == 2 && row.item_listed) continue;

    if (first) {
      LogInfo(L"---ROW %d--- selected: %d, listed: %d", index, row.selected,
        row.item_listed);
    }

    if (!row.selected) {
      if (first) {
        out_msg.assign(ui::msg::name::click_table_row);
        if (xtralog) LogInfo(L"****1 ");
        break;
      }
      row.selected = true;
    }
    if (row.item_price.GetPlat() != 2 && !setprice_clicked) {
      out_msg.assign(ui::msg::name::click_widget); // click set_price_button
      out_extra.assign(std::to_string(Broker::Sell::Widget::Id::SetPriceButton));
      setprice_clicked = true;
      if (xtralog) LogInfo(L"****2");
      break;
    }
    if (row.item_price.GetPlat() != 2) {
      if (!in_setprice) {
        in_setprice = true;
        out_msg.assign(ui::msg::name::send_chars); // enter price_text
        if (xtralog) LogInfo(L"****3  price(%d)", row.item_price.GetPlat());
        break;
      }
      out_msg.assign(ui::msg::name::click_widget); // click ok_button
      out_extra.assign(std::to_string(Broker::SetPrice::Widget::Id::OkButton));
      if (xtralog) LogInfo(L"****4  price(%d) row(%d)", row.item_price.GetPlat(), index);
      row.item_price = Price_t(2);
      break;
    }
    in_setprice = false;
    if (!row.item_listed) {
      if (!listed_clicked) {
        listed_clicked = true;
        out_msg.assign(out_msg.assign(ui::msg::name::click_widget)); // click list_item_button
        out_extra.assign(std::to_string(Broker::Sell::Widget::Id::ListItemButton));
        if (xtralog) LogInfo(L"****5 ");
        break;
      }
      row.item_listed = true;
      //if (xtralog) LogInfo(L"****6 ");
      //out_msg.assign("skip");
      //break;
    }
  }
  first = false;

  if (index == rows_page_1.size()) {
    if (final_message_sent) {
      final_message_sent = false;
      return std::move(std::make_unique<dp::Msg_t>("done"));
    } else {
      final_message_sent = true;
    }
  } else if (in_setprice) {
    using namespace Broker::SetPrice::Translate;
    return std::move(std::make_unique<Data_t>(rows_page_1[index].item_price));
  }
  /*Data_t::row_vector*/auto rows_copy = rows_page_1; //unnecessary. pass & copy directly below/ (no move)
  using namespace Broker::Sell::Translate;
  return std::move(std::make_unique<Data_t>(std::move(rows_copy), 0,
    Ui::Scroll::Position::Unknown));
}

namespace cope {
  dp::result_code run() {
    using namespace std::chrono;

    dp::txn::handler_t tx_sell{ Broker::Sell::txn::handler() };
    bool tx_active = false;
    dp::MsgPtr_t out{};

    auto start = high_resolution_clock::now();
    int i{};
    for (; i >= 0; i++) {
      std::string expected_out_msg_name;
      std::string extra;

      dp::MsgPtr_t out_ptr = std::move(translate(screenshot(),
        expected_out_msg_name, extra));
      if (out_ptr->msg_name == "done") break;
      if (!tx_active) {
        out_ptr = std::move(start_txn_sellitem(std::move(out_ptr)));
        tx_active = true;
      }
      out = tx_sell.send_value(std::move(out_ptr));
      if (!expected_out_msg_name.empty()) {
        LogInfo(L"expected out msg_name: %S %S", expected_out_msg_name.c_str(),
          extra.c_str());
      }
      if (out) {
        assert(out->msg_name == expected_out_msg_name);
        dp::dispatch(*out.get());
      }
      else {
        assert(expected_out_msg_name.empty());
      }
    }
    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = 1e-6 * duration_cast<nanoseconds>(end - start).count();

    std::cerr << "Elapsed: " << std::fixed << elapsed << std::setprecision(9)
      << "ms (" << i << " frames)" << std::endl;

    return dp::result_code::success;
  }
} // namespace cope
