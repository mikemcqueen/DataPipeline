#include "stdafx.h"
#include <format>
#include <memory>
#include "dp.h"
#include "log.h"

namespace dp {

#if 0
  result_code dispatch(const msg_t& msg) {
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
    auto validate_name(const msg_t& msg, std::string_view name)
      -> result_code
    {
      result_code rc = result_code::success;
      if (msg.msg_name != name) {
        LogInfo(L"msg::validate_name: name mismatch, expected(%S), actual(%S)",
          name.data(), msg.msg_name.c_str());
        rc = result_code::unexpected_error;
      }
      return rc;
    }
  } // namespace msg

  namespace txn {
    using promise_type = handler_t::promise_type;

#if 0
    complete_txn_awaitable complete(promise_type& promise, msg_ptr_t msg_ptr,
      result_code rc)
    {
      return complete_txn_awaitable{
        promise.prev_handle(),
        std::move(msg_ptr),
        rc
      };
    }

    complete_txn_awaitable complete(promise_type& promise) {
      return complete(promise, promise.in_ptr(), result_code::success);
    }

    complete_txn_awaitable complete(promise_type& promise, result_code rc) {
      return complete(promise, promise.in_ptr(), rc);
    }
#if 0
      if (rc != result_code::success) {
        LogInfo(L"%S::complete() error: %d", promise.txn_name().c_str(), (int)rc);
        return complete(promise, std::move(
          std::make_unique<dp::txn::complete_t>(promise.txn_name(), rc)));
      }
      else {
#endif
#endif
  } // namespace txn
} // namespace dp
