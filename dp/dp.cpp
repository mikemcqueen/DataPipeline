#include "stdafx.h"
#include <format>
#include <memory>
#include "dp.h"
#include "log.h"

namespace dp {
  namespace msg {
    auto validate_name(const msg_t& msg, std::string_view name)
      -> result_code
    {
      result_code rc = result_code::success;
      if (msg.msg_name != name) {
        LogInfo(L"msg::validate_name: name mismatch, expected(%S), actual(%S)",
          name, msg.msg_name);
        rc = result_code::unexpected_error;
      }
      return rc;
    }
  } // namespace msg

  namespace txn {
    complete_txn_awaitable complete(promise_type& promise, msg_ptr_t msg_ptr) {
      return complete_txn_awaitable{
        promise.prev_handle(),
        std::move(msg_ptr)
      };
    }

    complete_txn_awaitable complete(promise_type& promise) {
      return complete(promise, promise.in_ptr());
    }

    complete_txn_awaitable complete(promise_type& promise, result_code rc) {
      if (rc != result_code::success) {
        LogInfo(L"%S::complete() error: %d", promise.txn_name(), (int)rc);
        return complete(promise, std::move(
          std::make_unique<dp::txn::complete_t>(promise.txn_name(), rc)));
      } else {
        return complete(promise);
      }
    }
  } // namespace txn
} // namespace dp
