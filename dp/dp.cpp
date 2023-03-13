#include "stdafx.h"
#include <format>
#include <memory>
#include "dp.h"
#include "log.h"

namespace dp {

#if 0
  inline result_code dispatch(const msg_t& msg) {
    result_code rc{ result_code::success };
    if (!msg.msg_name.starts_with("ui::msg")) {
      LogInfo(L"dispatch(): unsupported message name, %S", msg.msg_name.c_str());
      rc = result_code::unexpected_error;
    }
    else {
      // this is a real problem.  how do we dispatch to UI without bringing it in
      // as a dependency. SendEvent? PostMessage? UiHandler?
      rc = ui::msg::dispatch(msg);
    }
    return rc;
  }
#endif

  namespace msg {
#if 0
    auto validate_name(const msg_t& msg, std::string_view name)
      -> result_code
    {
      result_code rc = result_code::s_ok;
      if (msg.msg_name != name) {
        LogError(L"msg::validate_name: name mismatch, expected(%S), actual(%S)",
          name.data(), msg.msg_name.c_str());
        rc = result_code::e_unexpected;
      }
      return rc;
    }
#endif
  } // namespace msg

  namespace txn {
    using promise_type = handler_t::promise_type;
  } // namespace txn
} // namespace dp
