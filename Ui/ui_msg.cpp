#include "stdafx.h"
#include "dp.h"
#include "ui_msg.h"
#include "UiInput.h"
#include "log.h"

namespace ui::msg {
  using dp::result_code;

  static bool s_enabled = true;

  void toggle_enabled() {
    s_enabled = !s_enabled;
    LogInfo(L"ui::msg::enabled(%d)", (int)s_enabled);
  }

  auto enabled() { return s_enabled; }

  result_code validate(const dp::msg_t& msg) {
    msg;
    return result_code::s_ok;
  }

  result_code validate_window(std::string_view window_name, HWND* result_hwnd) {
    result_code rc = result_code::s_ok;

    // TODO: Shouldn't we use GetMainWindow() for the GetForegroundWindow() check?
    HWND hwnd{ nullptr }; // m_Window.GetHwnd(window_name);
    if (!hwnd || !::IsWindowVisible(hwnd) || (hwnd != ::GetForegroundWindow())) {
      //LogError(L"ui::msg::validate_window failed for hwnd (%d)", hwnd);
      return result_code::e_fail;
    }
    result_hwnd;
    window_name;
#if 0
    const Ui::Window::Handle_t top{}; // m_Window.GetTopWindow() };
    // TODO : Window::IsParent
    if ((top.hWnd != hwnd)) { // && !util::IsParent(top.hWnd, hwnd)) {
      LogError(L"ui::msg::validate_window: (%S) is not top window or a child "
        L"of top window(%d)", std::string(window_name).c_str(), top.WindowId);
      return result_code::expected_error;
    }
    if (result_hwnd) {
      *result_hwnd = hwnd;
    }
#endif
    return rc;
  }

  result_code click_point(const click::data_t& msg) {
    result_code rc = result_code::s_ok;
    POINT point = msg.destination.point;
    LogInfo(L"click_point(%S, (%d), %d))", msg.window_name.c_str(),
      point.x, point.y);
    if (!enabled()) return rc;

    rc = validate(msg);
    if (failed(rc)) return rc;

    HWND hwnd{};
    rc = validate_window(msg.window_name, &hwnd);
    if (failed(rc)) return rc;

    for (int count = msg.count; count > 0; --count) {
      switch (msg.method) {
#if 0
      case Input::Method::SendMessage:
        Ui::Window::ClickSendMessage(hWnd, MAKELONG(Point.x, Point.y),
          Data.Button, Data.bDoubleClick);
        break;
#endif
      case input::method::SendInput:
        Ui::Input_t::Click(hwnd, point);
        break;
      default:
        LogError(L"click_point() invalid method (%d)", (int)msg.method);
        return result_code::e_unexpected;
      }
    }
    return rc;
  }

  result_code click_widget(const click::data_t& msg) {
    result_code rc = result_code::s_ok;
    const Ui::WidgetId_t widget_id = msg.destination.widget_id;
    LogInfo(L"click_widget(%S, %d)", msg.window_name.c_str(), widget_id);
    if (!enabled()) return rc;

    rc = validate(msg);
    if (failed(rc)) return rc;

    HWND hwnd{};
    rc = validate_window(msg.window_name, &hwnd);
    if (failed(rc)) return rc;

    //m_Window.GetWindow(msg.window_name).ClickWidget(widget_id, true);
    return rc;
  }

  result_code send_characters(const send_chars::data_t& msg) {
    result_code rc = result_code::s_ok;

    LogInfo(L"ui::msg::send_chars(%S)", msg.chars.c_str());
    HWND hwnd;
    rc = validate_window(msg.window_name, &hwnd);
    if (failed(rc)) return rc;

    return result_code::e_fail; // for now
    /*
    const ui::WidgetId_t widget_id = msg.destination.widget_id;
        if (ui::Widget::Id::Unknown != widget_id) {
      Rect_t rect;
      if (!m_Window.GetWindow(Data.WindowId).GetWidgetRect(WidgetId, &rect)) {
        LogError(L"SsWindow::SendChars(): GetWidgetRect(%d) failed", WidgetId);
        return;
      }
      ui::input::Click(hWnd, rect.Center());
    }
    ui::input::SendChars(Data.Chars);
*/
  }

  auto dispatch(const dp::msg_t& msg) -> result_code {
    result_code rc = result_code::s_ok;
    if (msg.msg_name == name::click_point) {
      rc = click_point(msg.as<click::data_t>());
    } else if (msg.msg_name == name::click_widget) {
      rc = click_widget(msg.as<click::data_t>());
    } else if (msg.msg_name == name::send_chars) {
      rc = send_characters(msg.as<send_chars::data_t>());
    } else {
      LogInfo(L"ui::msg::dispatch() unsupported msg name, %S", msg.msg_name.c_str());
      rc = result_code::e_unexpected;
    }
    return rc;
  }
} // namespace ui::msg