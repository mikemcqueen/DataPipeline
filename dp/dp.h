#pragma once

#include <coroutine>
#include <format>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include "dp_msg.h"
#include "log.h"
//#include "Rect.h"

namespace dp::txn {
  enum class state : int {
    ready = 0,
    started,
    complete
  };

  struct data_t : msg::data_t {
    data_t(std::string_view msg_name, std::string_view txn_name) :
      msg::data_t(msg_name), txn_name(txn_name) {}

    std::string txn_name;
  };

  template<typename stateT>
  struct start_t : data_t {
    using state_ptr_t = std::unique_ptr<stateT>;
 
    static const msg_t& msg_from(const msg_t& txn) {
      auto& txn_start = dynamic_cast<const start_t<stateT>&>(txn);
      return *txn_start.msg_ptr.get();
    }

    // TODO getting state from a message usually precedes a move, therefore 
    // no const. maybe this should just be move_state(txn, stateT*)
    static stateT&& state_from(msg_t& txn) {
      start_t<stateT>& txn_start = dynamic_cast<start_t<stateT>&>(txn);
      return std::move(*txn_start.state.get());
    }

    static auto validate(const msg_t& msg, std::string_view txn_name) {
      if (!is_start_txn(msg)) {
        return result_code::e_unexpected_msg_name;
      }
      const auto* txn = dynamic_cast<const start_t<stateT>*>(&msg);
      if (!txn) {
        return result_code::e_unexpected_msg_type;
      }
      if (txn->txn_name != txn_name) {
        return result_code::e_unexpected_txn_name;
      }
      return result_code::s_ok;
    }

    constexpr start_t(std::string_view txn_name, msg_ptr_t msg_ptr,
        state_ptr_t state_ptr) :
      data_t(msg::name::kTxnStart, txn_name),
      msg_ptr(std::move(msg_ptr)), state_ptr(std::move(state_ptr)) {}

    msg_ptr_t msg_ptr;
    state_ptr_t state_ptr;
  };

  class handler_t {
  public:
    struct promise_type;
    using handle_t = std::coroutine_handle<promise_type>;

    struct basic_awaitable {
      bool await_ready() const { return false; }
      void await_suspend(handle_t h) { promise_ = &h.promise(); }
      promise_type& await_resume() const { return *promise_; }

      const promise_type& promise() const { return *promise_; }
      promise_type& promise() { return *promise_; }

    private:
      promise_type* promise_ = nullptr;
    };

    struct initial_awaitable;
    struct promise_type {
      auto get_return_object() noexcept { return handler_t{ this }; }
      initial_awaitable initial_suspend() noexcept { return {}; }
      std::suspend_always final_suspend() noexcept { return {}; }
      void unhandled_exception() {
        LogError(L"************* Exception in %S **************",
          active_handle_.promise().txn_name().c_str());
        throw;
      }
      void return_void() { throw std::runtime_error("co_return not allowed"); }
      // TODO: return private awaitable w/o txn name?
      basic_awaitable yield_value(msg_ptr_t msg_ptr) {
        LogInfo(L"Yielding %S from %S...", msg_ptr.get()->msg_name.c_str(),
          txn_name().c_str());
        root_handle_.promise().emplace_out(std::move(msg_ptr));
        return {};
      }

      auto active_handle() const { return active_handle_; }
      void set_active_handle(handle_t h) { active_handle_ = h; }

      auto root_handle() const { return root_handle_; }
      void set_root_handle(handle_t h) { root_handle_ = h; }

      auto prev_handle() const { return prev_handle_; }
      void set_prev_handle(handle_t h) { prev_handle_.emplace(h); }

      // const auto& in() const { return *in_ptr_.get(); }
      auto& in() const { return *in_ptr_.get(); }
      void emplace_in(msg_ptr_t in_ptr) { in_ptr_ = std::move(in_ptr); }
      [[nodiscard]] msg_ptr_t&& in_ptr() { return std::move(in_ptr_); }

      auto& out() const { return *out_ptr_.get(); }
      void emplace_out(msg_ptr_t out_ptr) { out_ptr_ = std::move(out_ptr); }
      [[nodiscard]] msg_ptr_t&& out_ptr() { return std::move(out_ptr_); }

      const std::string& txn_name() const noexcept { return txn_name_; }
      void set_txn_name(std::string_view txn_name) { txn_name_ = txn_name; }

      operator result_t&() { return result_; }
      auto result() const noexcept { return result_; }
      result_t set_result(result_code rc) noexcept { result_.set(rc); return result_;  }

      state txn_state() const noexcept { return txn_state_; }
      bool txn_started() const noexcept { return txn_state_ == state::started; }
      bool txn_complete() const noexcept { return txn_state_ == state::complete; }
      void set_txn_state(state txn_state) {
        LogInfo(L"%S:set_txn_state(%S)", txn_name_.c_str(),
          std::format("{}", txn_state).c_str());
        txn_state_ = txn_state;
      }

    private:
      handle_t root_handle_;
      handle_t active_handle_;
      std::optional<handle_t> prev_handle_;
      msg_ptr_t in_ptr_;
      msg_ptr_t out_ptr_;
      std::string txn_name_;
      state txn_state_ = state::ready;
      result_t result_;
    };

    // TODO: private?
    struct initial_awaitable {
      bool await_ready() { return false; }
      bool await_suspend(handle_t h) {
        h.promise().set_root_handle(h);
        h.promise().set_active_handle(h);
        return false;
      }
      void await_resume() {}
    };

  public:
    handler_t(handler_t& h) = delete;
    handler_t operator=(handler_t& h) = delete;

    explicit handler_t(promise_type* p) noexcept :
      coro_handle_(handle_t::from_promise(*p)) {}
    ~handler_t() { if (coro_handle_) coro_handle_.destroy(); }

    auto handle() const noexcept { return coro_handle_; }
    auto& promise() const noexcept { return coro_handle_.promise(); }

  private:
    auto active_handle() const noexcept { return promise().active_handle(); }
    auto root_handle() const noexcept { return promise().root_handle(); }
    auto& txn_name() const noexcept { return promise().txn_name(); }

    void validate_send_value_msg(const msg_t& msg) const {
      if (msg::is_start_txn(msg)) {
        if (handle().promise().txn_started()) {
          LogError(L"send_value() %S already started", handle().promise().txn_name().c_str());
          throw std::logic_error("dp::txn::handler::start_txn(): txn already started");
        }
      } else if (!handle().promise().txn_started()) {
        LogError(L"send_value() %S is not started", handle().promise().txn_name().c_str());
        throw std::logic_error("dp::txn::handler::send_value(): txn not started");
      }
    }

  public:
    // TODO: this logic should be in an awaitable?
    [[nodiscard]] msg_ptr_t&& send_value(msg_ptr_t msg_ptr) {
      validate_send_value_msg(*msg_ptr.get());
      auto& active_p = active_handle().promise();
      active_p.emplace_in(std::move(msg_ptr));
      LogInfo(L"Sending %S to %S", active_p.in().msg_name.c_str(),
        active_p.txn_name().c_str());
      active_handle().resume();
      auto& root_p = root_handle().promise();
      LogInfo(L"Received %S from %S", root_p.out_ptr() ?
        root_p.out().msg_name.c_str() : "nothing",
        active_handle().promise().txn_name().c_str()); // active_handle() may have changed
      return std::move(root_p.out_ptr());
    }

  private:
    handle_t coro_handle_;
  }; // struct handler_t

  template<typename stateT>
  struct receive_awaitable : handler_t::basic_awaitable {
    receive_awaitable(std::string_view txn_name, stateT& state) :
      txn_name_(txn_name), state_(state) {}

    std::coroutine_handle<> await_suspend(handler_t::handle_t h) {
      handler_t::basic_awaitable::await_suspend(h);
      if (promise().txn_state() == state::started) {
        throw std::logic_error(std::format("receive_awaitable({}), cannot "
          "be used with txn in 'started' state", promise().txn_name()));
      }
      promise().set_txn_name(txn_name_);
      if (promise().txn_state() == state::complete) {
        promise().set_txn_state(state::ready);
        if (promise().prev_handle().has_value()) {
          // txn complete, prev handle exists; symmetric xfer to prev_handle
          handler_t::handle_t dst_handle = promise().prev_handle().value();
          auto& dst_promise = dst_handle.promise();
          // move promise.in to dst_promise.in
          dst_promise.emplace_in(std::move(promise().in_ptr()));
          LogInfo(L"  returning a %S to %S", dst_promise.in().msg_name.c_str(),
            dst_promise.txn_name().c_str());
          auto& root_promise = promise().root_handle().promise();
          root_promise.set_active_handle(dst_handle);
          // reset all handles of this completed coro to itself
          promise().set_root_handle(h);
          promise().set_active_handle(h);
          promise().prev_handle().reset();
          return dst_handle; // symmetric transfer to dst (prev) handle
        }
        else if (promise().in_ptr()) {
          // txn complete, no prev handle; root txn returning to send_value()
          // move promise.in to root_promise.out
          auto& root_promise = promise().root_handle().promise();
          root_promise.emplace_out(std::move(promise().in_ptr()));
          LogInfo(L"  returning a %S to send_value()",
            root_promise.out().msg_name.c_str());
        }
        else {
          // txn complete, no prev handle, no "in" value; not possible
          throw std::runtime_error("impossible state");
        }
      }
      LogInfo(L"  %S - returning noop_coroutine", promise().txn_name().c_str());
      return std::noop_coroutine();
    }

    handler_t::promise_type& await_resume() {
      using start_t = start_t<stateT>;
      auto& txn = promise().in();
      // validate this is a start_txn message for the txn we are expecting.
      promise().set_result(start_t::validate(txn, txn_name_));
      if (promise().result().failed()) {
        LogError(L"Resuming %S... ****ERROR**** %d", txn_name_.c_str(),
          (unsigned)promise().result().code);
        return promise();
      }
      auto& txn_start = dynamic_cast<start_t&>(txn);
      // copy/move initial state into coroutine frame
      // NOTE: state move/copy MUST happen BEFORE emplace_in()
      // why exactly?
      // TODO: std::move into a reference, does this actually move?
      // try with non-copyable type.
      state_ = std::move(*txn_start.state_ptr.get());
      // move msg_ptr from incoming txn to promise().in()
      promise().emplace_in(std::move(txn_start.msg_ptr));
      promise().set_txn_state(state::started);
      return promise();
    }

  private:
    std::string txn_name_;
    stateT& state_;
  };

  template<typename stateT>
  struct start_awaitable : handler_t::basic_awaitable {
    using handle_t = handler_t::handle_t;
    using state_ptr_t = start_t<stateT>::state_ptr_t;

    start_awaitable(handle_t dst_handle, msg_ptr_t msg_ptr,
      state_ptr_t state_ptr) :
      dst_handle_(dst_handle), msg_ptr_(std::move(msg_ptr)),
      state_ptr_(std::move(state_ptr)) {}

    auto await_suspend(handle_t h) {
      handler_t::basic_awaitable::await_suspend(h);
      LogInfo(L"start_txn_awaitable: Suspending %S...",
        promise().txn_name().c_str());
      auto& dst_promise = dst_handle_.promise();
      dst_promise.emplace_in(make_start_txn<stateT>(dst_promise.txn_name(),
        std::move(msg_ptr_), std::move(state_ptr_)));
      LogInfo(L"  sending a %S to %S", dst_promise.in().msg_name.c_str(),
        dst_promise.txn_name().c_str());
      dst_promise.set_root_handle(promise().root_handle());
      dst_promise.set_prev_handle(promise().active_handle());
      auto& root_promise = promise().root_handle().promise();
      root_promise.set_active_handle(dst_promise.active_handle());
      return dst_handle_; // symmetric transfer to dst
    }

    auto& await_resume() {
      LogInfo(L"Resuming %S...", promise().txn_name().c_str());
      return promise();
    }

  private:
    handle_t dst_handle_;
    msg_ptr_t msg_ptr_;
    state_ptr_t state_ptr_;
  };

  template<typename stateT>
  auto make_start_txn(std::string_view txn_name, msg_ptr_t msg_ptr,
    typename start_t<stateT>::state_ptr_t state_ptr)
  {
    return std::make_unique<start_t<stateT>>(txn_name, std::move(msg_ptr),
      std::move(state_ptr));
  }

  inline void complete(handler_t::promise_type& promise) { //, result_code rc) {
    if (!promise.txn_started()) {
      throw std::logic_error("txn::complete(): txn is not in started state");
    }
    promise.set_txn_state(state::complete);
    //promise.set_result(rc);
  }
} // namespace dp::txn

template <>
struct std::formatter<dp::txn::state> {
  constexpr auto parse(std::format_parse_context& ctx) {
    return ctx.begin();
  }
  auto format(const dp::txn::state s, std::format_context& ctx) {
    using dp::txn::state;
    static std::unordered_map<state, std::string> state_name_map = {
      { state::ready, "ready" },
      { state::started, "started" },
      { state::complete, "complete" }
    };
    return std::format_to(ctx.out(), "{}", state_name_map[s]);
  }
};

