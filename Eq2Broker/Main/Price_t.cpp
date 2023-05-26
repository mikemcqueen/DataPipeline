/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// Price_t.cpp
//
// Price_t class.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Price_t.h"
#include "Log.h"

// static
std::optional<Price_t> Price_t::MakeFromString(std::string_view text) {
  static const auto digits = "0123456789"sv;
  const auto& is_end = [](size_t pos) { return pos == std::string_view::npos; };

  // find 'p'
  auto p_pos = text.find_first_of('p');
  if (is_end(p_pos)) return std::nullopt;

  // find a digit
  auto digit_pos = text.find_first_of(digits);
  if (is_end(digit_pos) || (digit_pos > p_pos)) return std::nullopt;

  // grab all text between digit & 'p' non-inclusive.
  auto all_digits = text.substr(digit_pos, (p_pos - digit_pos));
  // validate all charactersr are digits
  auto non_digit_pos = all_digits.find_first_not_of(digits);
  if (!is_end(non_digit_pos)) return std::nullopt;

  int plat;
  std::from_chars(all_digits.data(), all_digits.data() + all_digits.size(), plat);
  // assert(ptr == all_digits.end());
  return std::make_optional<Price_t>(plat);
}

bool Price_t::FromString(std::string_view text) {
  const auto price = Price_t::MakeFromString(text);
  plat_ = price.has_value() ? price.value().GetPlat() : 0;
  return plat_ > 0;
}

std::string Price_t::ToString() const {
  std::array<char, 15> str;
  std::string result;
  if (plat_ > 0) {
    if (auto [ptr, ec] = std::to_chars(str.data(), str.data() + str.size(), plat_);
      ec == std::errc())
    {
      *ptr++ = 'p';
      result.assign(str.data(), ptr - str.data());
    }
  }
#if 0
  // lets just say no; result.empty() means "0p"
  else {
    //result.assign(std::string_view(std::make_error_code(ec).message()));
  }
#endif
  return result;
}