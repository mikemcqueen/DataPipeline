#pragma once

#ifndef SETPRICE_WIDGETS_H
#define SETPRICE_WIDGETS_H

#include "UiTypes.h"
#include "BrokerUi.h"

namespace Broker::SetPrice {
  constexpr SIZE PriceTextSize = { 140, 16 };
  constexpr SIZE ButtonSize = { 10, 10 };

  // ok (text) size 23x10
  // clear (text) size 8x8
  // 10 x 10 probably ok in both cases

  constexpr int Row[5] = { 33, 76, 108, 140, 172 };
  constexpr int Col[4] = { 39, 89, 139, 200 };

  constexpr struct Ui::Widget::Data_t Widgets[] = {
    { Widget::Id::PriceText, { RelativeRect_t::LeftTop, -187, 4, //23, Row[0],
      PriceTextSize.cx, PriceTextSize.cy } },
    { Widget::Id::ClearButton,  { RelativeRect_t::LeftTop, -72, 137, // Col[3], Row[0],
      ButtonSize.cx, ButtonSize.cy } },
    { Widget::Id::OkButton, { RelativeRect_t::LeftTop, 0, 0,
      ButtonSize.cx, ButtonSize.cy } },

  #if 0
        { Widget::Id::OneButton,      { RelativeRect_t::LeftTop, Col[0], Row[1], ButtonSize.cx, ButtonSize.cy } },
        { Widget::Id::TwoButton,      { RelativeRect_t::LeftTop, Col[1], Row[1], ButtonSize.cx, ButtonSize.cy } },
        { Widget::Id::ThreeButton,    { RelativeRect_t::LeftTop, Col[2], Row[1], ButtonSize.cx, ButtonSize.cy } },
        { Widget::Id::PlatinumButton, { RelativeRect_t::LeftTop, Col[3], Row[1], ButtonSize.cx, ButtonSize.cy } },

        { Widget::Id::FourButton,     { RelativeRect_t::LeftTop, Col[0], Row[2], ButtonSize.cx, ButtonSize.cy } },
        { Widget::Id::FiveButton,     { RelativeRect_t::LeftTop, Col[1], Row[2], ButtonSize.cx, ButtonSize.cy } },
        { Widget::Id::SixButton,      { RelativeRect_t::LeftTop, Col[2], Row[2], ButtonSize.cx, ButtonSize.cy } },
        { Widget::Id::GoldButton,     { RelativeRect_t::LeftTop, Col[3], Row[2], ButtonSize.cx, ButtonSize.cy } },

        { Widget::Id::SevenButton,    { RelativeRect_t::LeftTop, Col[0], Row[3], ButtonSize.cx, ButtonSize.cy } },
        { Widget::Id::EightButton,    { RelativeRect_t::LeftTop, Col[1], Row[3], ButtonSize.cx, ButtonSize.cy } },
        { Widget::Id::NineButton,     { RelativeRect_t::LeftTop, Col[2], Row[3], ButtonSize.cx, ButtonSize.cy } },
        { Widget::Id::SilverButton,   { RelativeRect_t::LeftTop, Col[3], Row[3], ButtonSize.cx, ButtonSize.cy } },

        { Widget::Id::ZeroButton,     { RelativeRect_t::LeftTop, (Col[0] + Col[1]) / 2, Row[4], ButtonSize.cx, ButtonSize.cy } },

        { Widget::Id::CopperButton,   { RelativeRect_t::LeftTop, Col[3], Row[4], ButtonSize.cx, ButtonSize.cy } }
  #endif
  };

} // namespace Broker::SetPrice

#endif // SETPRICE_WIDGETS_H