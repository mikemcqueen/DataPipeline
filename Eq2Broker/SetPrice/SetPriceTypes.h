///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// SetPriceTypes.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_SETPRICETYPES_H
#define Include_SETPRICETYPES_H

#include "UiWindowManager.h"
#include "BrokerUi.h"

namespace Broker::SetPrice {
  constexpr auto kWindowId = Broker::Window::Id::SetPrice;
  constexpr auto kWindowName = "SetPrice";
  constexpr auto kMsgName = "msg::set_price";

  namespace Translate {
    namespace Legacy {
      struct Data_t;
    }
    struct data_t;
    class Handler_t;
  } // Translate

  namespace Interpret {
    namespace Legacy {
      struct Data_t;
    }
    struct data_t;
    class Handler_t;
  } // Interpret

  class Window_t;
#if 0
  namespace Window {
    typedef Ui::Window::Manager_t<Window_t, Translate::Handler_t,
      Interpret::Handler_t> ManagerBase_t;
    class Manager_t;
  } // Window
#endif

} // namespace Broker::SetPrice

#endif // Include_SETPRICETYPES_H
