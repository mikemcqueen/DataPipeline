///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// SetPrice.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_SETPRICE_H
#define Include_SETPRICE_H

#include "SetPriceWindow.h"
#include "DcrSetPrice.h"
#include "TiSetPrice.h"
#include "SetPriceTypes.h"

namespace Broker::SetPrice::Window {
  using ManagerBase_t = Ui::Window::Manager_t<Window_t,
    Translate::Handler_t, Interpret::Handler_t>;

  class Manager_t : public ManagerBase_t {
  public:
    Manager_t(const Ui::Window::Base_t& Window) : ManagerBase_t(Window) {}
  };
} // namespace Broker::SetPrice::Window

#endif // Include_SETPRICE_H
