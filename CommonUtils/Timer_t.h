
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//

#pragma once
#ifndef Include_TIMER_T_H
#define Include_TIMER_T_H

#include "Log.h"

class Timer_t final {
public:
  using TimePoint = std::chrono::time_point<std::chrono::steady_clock,
    std::chrono::duration<double>>;

  explicit Timer_t(std::string_view msg, bool log_now = false) :
    msg_(msg)
  {
    using namespace std::chrono;
    start_ = high_resolution_clock::now();
    if (log_now) {
      LogInfo(L"++%S", msg_.data());
    }
    done_ = false;
  }

  ~Timer_t() {
    done();
  }

  void done() {
    if (!done_) {
      using namespace std::chrono;
      auto end = high_resolution_clock::now();
      double elapsed = 1e-6 * duration_cast<nanoseconds>(end - start_).count();
      LogInfo(L"%S, elapsed: %.2fms", msg_.data(), elapsed);
      done_ = true;
    }
  }

private:
  TimePoint start_;
  std::string_view msg_;
  bool done_ = true;
};

#endif // Include_TIMER_T_H
