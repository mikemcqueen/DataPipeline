#pragma once

#include "UiTypes.h"
#include "BrokerUi.h"

namespace Broker::Buy {

constexpr Ui::Widget::Data_t BrokerTabs[] =
{
    { Frame::Widget::Id::SellTab,        { RelativeRect_t::LeftTop,       72, -98,    9,    9 } }, // Broker Buy
    { Frame::Widget::Id::SalesLogTab,    { RelativeRect_t::LeftTop,      122, -98,    9,    9 } }, // Broker Buy
};

constexpr Ui::Widget::Data_t MarketTabs[] =
{
    { Frame::Widget::Id::SellTab,        { RelativeRect_t::LeftTop,       94,  -98,    9,    9 } }, // Market Browse
    { Frame::Widget::Id::SalesLogTab,    { RelativeRect_t::LeftTop,      145,  -98,    9,    9 } }, // Market Browse
};

constexpr Ui::Widget::Data_t Widgets[] =
{
    { Widget::Id::SearchLabel,           { RelativeRect_t::LeftTop,       12,  -70,    9,    9 } },
    { Widget::Id::SearchEdit,            { RelativeRect_t::LeftTop,       53,  -73,  620,   16 } },
    { Widget::Id::FindButton,            { RelativeRect_t::LeftTop,      717,  -70,    9,    9 } },
    { Widget::Id::SearchDropdown,        { RelativeRect_t::LeftTop,      778,  -70,  133,   12 } },
    { Widget::Id::PageNumber,            { RelativeRect_t::CenterBottom,   0,   11,  200,   19 } },
    { Widget::Id::NextButton,            { RelativeRect_t::RightBottom, -214,   12,   88,   16 } },
    { Widget::Id::BuyButton,             { RelativeRect_t::RightBottom,  -76,   66,   21,   12 } },
};

} // Broker::Buy