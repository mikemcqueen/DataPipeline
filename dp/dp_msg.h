#pragma once
#include <memory>
#include <string>
#include <string_view>
#include "dp_result.h"
#include "log.h"

using namespace std::literals;

namespace dp {
  namespace msg { struct Data_t; }

  using msg_t = msg::Data_t;
  using msg_ptr_t = std::unique_ptr<msg_t>;

  namespace msg {
    namespace name {
      constexpr auto txn_start{ "msg::txn_start" };
      constexpr auto txn_complete{ "msg::txn_complete" };
    }

    struct Data_t {
      Data_t(std::string_view name) : msg_name(name) {}
      Data_t(const Data_t&) = delete;
      Data_t& operator=(const Data_t&) = delete;
      virtual ~Data_t() {}

      template<typename T> const T& as() const {
        return dynamic_cast<const T&>(*this);
      }
      template<typename T> T& as() {
        return dynamic_cast<T&>(*this);
      }

      std::string msg_name;
    };

    auto validate_name(const msg_t& msg, std::string_view msg_name) -> result_code;

    template<typename msgT>
    auto validate(const msg_t& msg, std::string_view msg_name) {
      cout << "    msg 1. validate name" << endl;
      result_code rc = validate_name(msg, msg_name);
      if (rc == result_code::success) {
        cout << "    msg 2. dynacast" << endl;
        if (!dynamic_cast<const msgT*>(&msg)) {
          LogError(L"msg::validate: type mismatch");
          rc = result_code::unexpected_error;
        }
      }
      cout << "    msg 3. rc = " << (int)rc << endl;
      return rc;
    }

    constexpr auto is_start_txn(const msg_t& msg) {
      return msg.msg_name == msg::name::txn_start;
    }
  } // namespace msg
} // namespace dp

