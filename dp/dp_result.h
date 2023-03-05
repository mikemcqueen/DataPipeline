#pragma once

namespace dp {
  enum class result_code {
    success = 0,
    expected_error,
    unexpected_error
  };

#if 0
  // could be useful for result_code
  template <>
  struct std::formatter<Point> : std::formatter<std::string> {
    auto format(Point p, format_context& ctx) {
      return formatter<string>::format(
        std::format("[{}, {}]", p.x, p.y), ctx);
    }
  };

  struct move_only {
    move_only& operator=(const move_only& m) = delete;
    move_only& operator=(move_only&& m) = default;
  };
#endif
}
