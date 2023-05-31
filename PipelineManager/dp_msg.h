#pragma once

#include "cope_msg.h"
#include "DpEvent.h"

namespace DP::msg {
  namespace name {
    constexpr std::string_view kEventWapper{ "msg::event_wrapper" };
  }

  struct event_wrapper_base_t : cope::msg_t {
    event_wrapper_base_t() : cope::msg_t(name::kEventWapper) {}
    virtual ~event_wrapper_base_t() = default;
    virtual Event::Data_t* get_event_data() = 0;
  };

  template<typename T> // TODO: requires inherit from Event::Data_t
  struct event_wrapper_t : event_wrapper_base_t {
    event_wrapper_t(std::unique_ptr<T> event_ptr) :
      event_ptr(std::move(event_ptr)) {}
    event_wrapper_t() = delete;

    Event::Data_t* get_event_data() override {
      return reinterpret_cast<Event::Data_t*>(event_ptr.get());
    }

    std::unique_ptr<T> event_ptr;
  };
} // namespace DP::msg