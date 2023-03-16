#pragma once
#include <memory>
#include <string>
#include <string_view>
#include "dp_result.h"
#include "DpEvent.h"
#include "log.h"

using namespace std::literals;

namespace dp {
  namespace msg { struct data_t; }

  using msg_t = msg::data_t;
  using msg_ptr_t = std::unique_ptr<msg_t>;

  namespace msg {
    namespace name {
      constexpr auto txn_start{ "msg::txn_start" };
      constexpr auto kEventWapper{ "msg::event_wrapper" };
    }

    struct data_t {
      data_t(std::string_view name) : msg_name(name) {}
      data_t(const data_t&) = delete;
      data_t& operator=(const data_t&) = delete;
      virtual ~data_t() {}

      template<typename T> const T& as() const {
        return dynamic_cast<const T&>(*this);
      }
      template<typename T> T& as() {
        return dynamic_cast<T&>(*this);
      }

      std::string msg_name;
    };

    struct event_wrapper_base_t : data_t {
      event_wrapper_base_t() : data_t(name::kEventWapper) {}
      virtual ~event_wrapper_base_t() = default;
      virtual DP::Event::Data_t* get_event_data() = 0;
    };

    template<typename T> // TODO: requires inherit from Event::Data_t
    struct event_wrapper_t : event_wrapper_base_t {
      event_wrapper_t(std::unique_ptr<T> event_ptr) :
        event_ptr(std::move(event_ptr)) {}
      event_wrapper_t() = delete;

      DP::Event::Data_t* get_event_data() override {
        return reinterpret_cast<DP::Event::Data_t*>(event_ptr.get());
      }

      std::unique_ptr<T> event_ptr;
    };

    inline auto validate_name(const msg_t& msg, std::string_view msg_name) {
      result_code rc = result_code::s_ok;
      if (msg.msg_name != msg_name) {
        LogError(L"msg::validate_name() mismatch, expected(%S), actual(%S)",
          msg_name.data(), msg.msg_name.c_str());
        rc = result_code::e_unexpected_msg_name;
      }
      return rc;
    }

    template<typename msgT>
    auto validate(const msg_t& msg, std::string_view msg_name) {
      result_code rc = validate_name(msg, msg_name);
      if (succeeded(rc) && !dynamic_cast<const msgT*>(&msg)) {
        LogWarning(L"msg::validate(%S) type mismatch, expected(%S), actual(%S)",
          msg_name.data(), msg.msg_name.c_str());
        rc = result_code::e_unexpected_msg_type;
      }
      return rc;
    }

    constexpr auto is_start_txn(const msg_t& msg) {
      return msg.msg_name == msg::name::txn_start;
    }
  } // namespace msg
} // namespace dp

