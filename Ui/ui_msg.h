// ui_msg.h

#ifndef INCLUDE_UI_MSG_H
#define INCLUDE_UI_MSG_H

#include <string_view>
#include "cope_msg.h"
#include "UiTypes.h"

namespace ui::msg {
  constexpr std::string_view kMsgPrefix{ "ui::msg" };

  namespace name {
    using namespace std::literals;

    constexpr std::string_view click_widget{ "ui::msg::click_widget" };
    constexpr std::string_view click_point{ "ui::msg::click_point" };
    constexpr std::string_view click_table_row{ "ui::msg::click_table_row" };
    constexpr std::string_view send_chars{ "ui::msg::send_chars" };
    constexpr std::string_view set_widget_text{ "ui::msg::set_widget_text" };
  }

  struct data_t : cope::msg_t {
    data_t(std::string_view msg_name, std::string_view wnd_name) :
      cope::msg_t(msg_name), window_name(wnd_name) {}

    std::string window_name;
  };

  namespace input {
    enum class method : int {
        SendMessage,
        SendInput,
    };

    union destination_t {
      destination_t(Ui::WidgetId_t id) : widget_id(id) {}
      destination_t(POINT pt) : point(pt) {}

      Ui::WidgetId_t widget_id;
      POINT point;
    };

    struct data_t : msg::data_t {
      // POINT constructor
      data_t(std::string_view msg_name, std::string_view window_name,
        POINT pt, method mtd) :
        msg::data_t(msg_name, window_name), destination(pt), method(mtd) {}

      // Ui::WidgetId_t constructor:
      data_t(std::string_view msg_name, std::string_view window_name,
        Ui::WidgetId_t widget_id, method mtd) :
        msg::data_t(msg_name, window_name), destination(widget_id), method(mtd) {}

      destination_t destination;
      input::method method;
    };

    constexpr auto default_method{ method::SendInput };
    constexpr auto default_button{ Ui::Mouse::Button::Left };
  } // namespace input

  namespace click {
    struct data_t : input::data_t {
      // POINT constructor:
      data_t(std::string_view window_name, POINT pt,
          input::method method = input::default_method,
          Ui::Mouse::Button_t btn = input::default_button,
          int cnt = 1, bool dbl = false) :
        input::data_t(name::click_point, window_name, pt, method),
        button(btn), count(cnt), double_click(dbl)
      {}

      // Ui::WidgetId_t constructor:
      data_t(std::string_view window_name, Ui::WidgetId_t widget_id,
          input::method method = input::default_method,
          Ui::Mouse::Button_t btn = input::default_button,
          int cnt = 1, bool dbl = false) :
        input::data_t(name::click_widget, window_name, widget_id, method),
        button(btn), count(cnt), double_click(dbl)
      {}

      Ui::Mouse::Button_t button;
      int count;
      bool double_click;
    };
  } // namespace click

  // note there is a possibility that a "raw" sendchars is useful, and
  // may contain a window id. and set_widget_text may derive from it.
  // namespace SendChars {}

  namespace send_chars {
    struct data_t : input::data_t {
      data_t(std::string_view window_name, Ui::WidgetId_t widget_id,
        std::string_view chs, input::method method = input::default_method) :
        input::data_t(name::send_chars, window_name, widget_id, method),
        chars(chs) {}

      std::string chars;
    };
  } // namespace send_chars

  void toggle_enabled();
  auto dispatch(const cope::msg_t& msg) -> cope::result_code;
} // namespace ui::msg

#endif // INCLUDE_UI_MSG_H
