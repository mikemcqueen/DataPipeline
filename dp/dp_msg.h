#pragma once
#include <memory>
#include <string>
#include <string_view>
#include "dp_result.h"
#include "log.h"

using namespace std::literals;

namespace dp {
  namespace msg { struct Data_t; }

  using Msg_t = msg::Data_t;
  using MsgPtr_t = std::unique_ptr<Msg_t>;

  namespace msg {
    namespace name {
      constexpr auto txn_start{ "msg::txn_start"sv };
      constexpr auto txn_complete{ "msg::txn_complete"sv };
    }

    struct Data_t {
      Data_t(std::string_view name) : msg_name(name) {}
      Data_t(const Data_t&) = delete;
      Data_t& operator=(const Data_t&) = delete;
      virtual ~Data_t() {}

      template<typename T> const T& as() const {
        return *dynamic_cast<const T*>(this);
      }
      template<typename T> T& as() {
        return *dynamic_cast<T*>(this);
      }

      std::string msg_name;
    };

    auto validate_name(const Msg_t& msg, std::string_view msg_name) -> result_code;

    template<typename msgT>
    auto validate(const Msg_t& msg, std::string_view msg_name) {
      result_code rc = validate_name(msg, msg_name);
      if (rc == result_code::success) {
        if (!dynamic_cast<const msgT*>(&msg)) {
          LogError(L"msg::validate: type mismatch");
          rc = result_code::unexpected_error;
        }
      }
      return rc;
    }
  } // namespace msg
} // namespace dp

