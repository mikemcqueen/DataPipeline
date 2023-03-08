#pragma once

#include <coroutine>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include "dp_msg.h"
#include "log.h"
#include "Rect.h"

namespace dp::txn {
  enum class state : int {
    ready = 0,
    started,
    complete
  };

  struct Data_t : msg::Data_t {
    Data_t(std::string_view msg_name, std::string_view t_name) :
      msg::Data_t(msg_name), txn_name(t_name) {}

    std::string txn_name;
  };

  template<typename stateT>
  struct start_t : Data_t {
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

    constexpr start_t(std::string_view txn_name, msg_ptr_t m_ptr,
        state_ptr_t s_ptr) :
      Data_t(msg::name::txn_start, txn_name),
      msg_ptr(std::move(m_ptr)), state_ptr(std::move(s_ptr)) {}

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
      initial_awaitable initial_suspend() noexcept { return {}; }
      std::suspend_always final_suspend() noexcept { return {}; }
      auto get_return_object() noexcept { return handler_t{ this }; }
      void unhandled_exception() { throw; }
      void return_void() { throw std::runtime_error("co_return not allowed"); }
      // TODO: return private awaitable w/o txn name?
      basic_awaitable yield_value(dp::msg_ptr_t msg_ptr) {
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

      //auto& txn_handler() const { return *txn_handler_;  }
      //void set_txn_handler(handler_t* handler) { txn_handler_ = handler; }

      /*
      // would be nice for this overload to work. doesn't default to const? why?
      // read up on overload selection
      dp::msg_ptr_t&& in() { return std::move(in_); }
      template<typename T> const T& in_as() const {
        return *dynamic_cast<const T*>(in_.get());
      }
      */
      //      const auto& in() const { return *in_ptr_.get(); }
      auto& in() const { return *in_ptr_.get(); }
      void emplace_in(dp::msg_ptr_t in_ptr) { in_ptr_ = std::move(in_ptr); }
      [[nodiscard]] dp::msg_ptr_t&& in_ptr() { return std::move(in_ptr_); }

      auto& out() const { return *out_ptr_.get(); }
      void emplace_out(dp::msg_ptr_t out_ptr) { out_ptr_ = std::move(out_ptr); }
      [[nodiscard]] dp::msg_ptr_t&& out_ptr() { return std::move(out_ptr_); }

      const std::string& txn_name() { return txn_name_; }
      void set_txn_name(const std::string& txn_name) { txn_name_ = txn_name; }

      auto result() const { return result_; }
      void set_result(result_code rc) { result_ = rc; }

      state txn_state() const { return txn_state_; }
      bool txn_started() const { return txn_state_ == state::started; }
      void set_txn_state(state txn_state) {
        LogInfo(L"%S:set_txn_state(%d)", txn_name_.c_str(), (int)txn_state);
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
      result_code result_ = result_code::success;
    };

    // TODO: private?
    struct initial_awaitable {
      bool await_ready() { return false; }
      bool await_suspend(handle_t h) {
        LogInfo(L"initial_awaitable::await_suspend()");
        h.promise().set_root_handle(h);
        h.promise().set_active_handle(h);
        return false;
      }
      void await_resume() { /* will never be called */ }
    };

  public:
    handler_t(handler_t& h) = delete;
    handler_t operator=(handler_t& h) = delete;

    explicit handler_t(promise_type* p) noexcept :
      coro_handle_(handle_t::from_promise(*p)) { /*p->set_txn_handler(this);*/
    }
    ~handler_t() { if (coro_handle_) coro_handle_.destroy(); }

    auto handle() const noexcept { return coro_handle_; }
    auto txn_started() const noexcept { return coro_handle_.promise().txn_started(); }

  private:
    auto active_handle() const noexcept { return coro_handle_.promise().active_handle(); }
    auto root_handle() const noexcept { return coro_handle_.promise().root_handle(); }
    auto& txn_name() const noexcept { return coro_handle_.promise().txn_name(); }

    void validate_send_value_msg(const msg_t& msg) {
      if (msg::is_start_txn(msg)) {
        if (handle().promise().txn_started()) {
          LogError(L"start_txn() already started: %S", handle().promise().txn_name().c_str());
          throw std::logic_error("dp::txn::handler::start_txn(): txn already started");
        }
      }
      else if (!handle().promise().txn_started()) {
        LogError(L"send_value() no txn started");
        throw std::logic_error("dp::txn::handler::send_value(): no txn started");
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

  inline result_code validate_start(const msg_t& msg, std::string_view txn_name) {
    if (msg.msg_name == msg::name::txn_start) {
      auto* txn = dynamic_cast<const Data_t*>(&msg);
      if (txn && (txn->txn_name == txn_name)) {
        return result_code::success;
      }
    }
    return result_code::unexpected_error;
  }

  template<typename stateT>
  struct receive_txn_awaitable : handler_t::basic_awaitable {
    using start_t = dp::txn::start_t<stateT>;

    receive_txn_awaitable(std::string_view txn_name, stateT& state) :
      txn_name_(txn_name), state_(state) {}

    std::coroutine_handle<> await_suspend(handler_t::handle_t h) {
      handler_t::basic_awaitable::await_suspend(h);
      if (promise().txn_state() == state::started) {
        throw std::logic_error("receive_txn_awaitable(), cannot be used with"
          "txn in 'started' state");
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
      LogInfo(L"  returning noop_coroutine");
      return std::noop_coroutine();
    }

    handler_t::promise_type& await_resume() {
      auto& txn = promise().in();
      // validate this is the txn we expect, just because. although
      // not sure what to do about failure at this point.
      result_code rc = validate_start(txn, txn_name_);
      if (rc == result_code::success) {
        auto& txn_start = dynamic_cast<start_t&>(txn);
        // copy/move initial state into coroutine frame
        // NOTE: state move/copy MUST happen BEFORE replacing promise.in()
        // TODO: is this actually a move? try w/moveonly state
        state_ = std::move(*txn_start.state_ptr.get());
        // extract msg_ptr from txn and plug it back in to promise().in().
        promise().emplace_in(std::move(txn_start.msg_ptr));
        promise().set_txn_state(state::started);
        return promise();
      }
      LogError(L"Resuming %S... ****ERROR**** %d", txn_name_.c_str(), (int)rc);
      // TODO: return empty_promise();
      return promise();
    }

  private:
    std::string txn_name_;
    stateT& state_;
  };

  struct transfer_txn_awaitable {
    using handle_t = handler_t::handle_t;

    transfer_txn_awaitable(std::optional<handle_t> dst_handle, dp::msg_ptr_t msg_ptr) :
      dst_handle_(dst_handle), msg_ptr_(std::move(msg_ptr)) {}

    transfer_txn_awaitable(handle_t dst_handle, dp::msg_ptr_t msg_ptr) :
      transfer_txn_awaitable(std::make_optional(dst_handle), std::move(msg_ptr)) {}

    auto await_ready() { return false; }
    auto& await_resume() {
      LogInfo(L"Resuming %S...", src_promise_->txn_name().c_str());
      return *src_promise_;
    }

  protected:
    handler_t::promise_type* src_promise_ = nullptr;
    std::optional<handle_t> dst_handle_;
    dp::msg_ptr_t msg_ptr_;
  };

#if 0
  template<typename stateT>
  struct start_txn_awaitable;
#endif    

  template<typename stateT>
  auto make_start_txn(std::string_view txn_name, msg_ptr_t msg_ptr,
    typename start_t<stateT>::state_ptr_t state_ptr)
  {
    return std::make_unique<start_t<stateT>>(txn_name, std::move(msg_ptr),
      std::move(state_ptr));
  }

  template<typename stateT>
  struct start_txn_awaitable : transfer_txn_awaitable {
    using state_ptr_t = dp::txn::start_t<stateT>::state_ptr_t;

    start_txn_awaitable(handle_t handle, dp::msg_ptr_t msg_ptr, state_ptr_t state_ptr) :
      transfer_txn_awaitable(handle, std::move(msg_ptr)),
      state_ptr_(std::move(state_ptr)) {}

    auto await_suspend(handle_t h) {
      src_promise_ = &h.promise();
      LogInfo(L"start_txn_awaitable: Suspending %S...",
        src_promise_->txn_name().c_str());
      auto& dst_p = dst_handle_->promise();
      dst_p.emplace_in(make_start_txn<stateT>(dst_p.txn_name(), std::move(msg_ptr_),
        std::move(state_ptr_)));
      LogInfo(L"  sending a %S to %S", dst_p.in().msg_name.c_str(),
        dst_p.txn_name().c_str());
      dst_p.set_root_handle(src_promise_->root_handle());
      dst_p.set_prev_handle(src_promise_->active_handle());
      auto& root_p = src_promise_->root_handle().promise();
      root_p.set_active_handle(dst_p.active_handle());
      return dst_handle_.value(); // symmetric transfer to dst
    }
  private:
    state_ptr_t state_ptr_;
  };

#if 0
  struct complete_txn_awaitable : transfer_txn_awaitable {
    complete_txn_awaitable(std::optional<handle_t> handle, dp::msg_ptr_t msg_ptr,
        result_code rc) :
      transfer_txn_awaitable(handle, std::move(msg_ptr)),
      rc_(rc) {}

    std::coroutine_handle<> await_suspend(handle_t h) {
      src_promise_ = &h.promise();
      LogInfo(L"complete_txn_awaitable: Suspending %S...",
        src_promise_->txn_name().c_str());

      assert(src_promise_->txn_started());
      src_promise_->set_txn_started(false);
      src_promise_->set_result(rc_);
      if (dst_handle_.has_value()) {
        auto& dst_p = dst_handle_->promise();
        // move msg_ptr to dst_promise.in()
        dst_p.emplace_in(std::move(msg_ptr_));
        LogInfo(L"  returning a %S to %S", dst_p.in().msg_name.c_str(),
          dst_p.txn_name().c_str());
        // TODO dst_p makes slightly more sense here?
        auto& root_p = src_promise_->root_handle().promise();
        root_p.set_active_handle(dst_handle_.value());
        // reset source (completing) coro state
        src_promise_->set_root_handle(h);
        src_promise_->set_active_handle(h);
        src_promise_->prev_handle().reset();
        return dst_handle_.value(); // symmetric transfer to dst
      } else {
        if (msg_ptr_) {
          // move msg_ptr to root_promise.out()
          auto& root_p = src_promise_->root_handle().promise();
          root_p.emplace_out(std::move(msg_ptr_));
          LogInfo(L"  returning a %S to send_value()", root_p.out().msg_name.c_str());
        } else {
          // is this even possible? not currently I don't think
          assert(false);
        }
        LogInfo(L"  returning noop_coroutine");
        return std::noop_coroutine();
      }
    }

  private:
    result_code rc_;
  };
#endif
  inline void complete(handler_t::promise_type& promise, result_code rc) {
    if (!promise.txn_started()) {
      throw std::logic_error("txn::complete(): txn is not in started state");
    }
    promise.set_txn_state(state::complete);
    promise.set_result(rc);
  }
} // namespace dp::txn
