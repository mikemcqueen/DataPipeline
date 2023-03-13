/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// Price_t.h
//
// Price_t class.
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_PRICE_T_H
#define Include_PRICE_T_H

class Price_t {
public:
  static std::optional<Price_t> MakeFromString(std::string_view text);

  Price_t() : plat_(0) {}
  Price_t(int plat) : plat_(plat) {}
  
  Price_t(const Price_t& price) = default;

  int GetPlat() const { return plat_; }
  void Reset() { plat_ = 0; }
  std::string ToString() const;
  bool FromString(std::string_view text); // i'm not convinced this is necessary

private:
  int plat_;
};

#endif Include_PRICE_T_H
