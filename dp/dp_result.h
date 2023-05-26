#pragma once

#include <functional>

#if 0
#include <format>
#include <unordered_map>
#endif

namespace dp {
  enum class result_code : unsigned {
    s_ok = 0,
    s_false = 1,
    e_abort = 4,
    e_fail = 5,
    e_unexpected = 0xffff,
    e_unexpected_msg_name,
    e_unexpected_msg_type,
    e_unexpected_txn_name
  };

  inline auto succeeded(result_code rc) { return rc <= result_code::s_false; }
  inline auto failed(result_code rc) { return rc > result_code::s_false; }
  inline auto unexpected(result_code rc) {
    return rc >= result_code::e_unexpected;
  }

  struct result_t {
    result_code code = result_code::s_ok;

    void set(result_code rc) { code = rc; }

    auto succeeded() { return dp::succeeded(code); }
    auto failed() { return dp::failed(code); }
    auto unexpected() { return dp::unexpected(code); }
  };

/*
  template<std::invocable T> // todo: callable
  auto result_of(T& op, result_t& result) {
    result.set(std::invoke(op)());
//    result.set(op());
    return result;
  }
*/

#if 0
#define E_NOINTERFACE _HRESULT_TYPEDEF_(0x80004002)
#define E_POINTER _HRESULT_TYPEDEF_(0x80004003)

  struct move_only {
    move_only& operator=(const move_only& m) = delete;
    move_only& operator=(move_only&& m) = default;
  };
#endif
}

#if 0
// could be useful for result_code
template <>
struct std::formatter<dp::result_code> : std::formatter<std::string> {
  auto format(dp::result_code rc, format_context& ctx) {
    using dp::result_code;
    static std::unordered_map<dp::result_code, std::string> rc_map = {
      { s_ok, "s_ok" },
      { s_false, "s_false" },
      { e_abort, "e_abort" },
      { e_fail, "e_fail" },
      { e_unexpected, "e_unexpected" }
    };
    if (rc_map.contains(rc)) {
      return std::formatter<string>::format(
        std::format("{}", rc_map[rc]));
    }
    else {
    }
  }
};
#endif

