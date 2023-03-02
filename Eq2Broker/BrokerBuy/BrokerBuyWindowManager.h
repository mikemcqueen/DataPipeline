///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// BrokerBuyWindowManager.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_BROKERBUYWINDOWMANAGER_H
#define Include_BROKERBUYWINDOWMANAGER_H

#include "BrokerBuyWindow.h"
#include "DcrBrokerBuy.h"
#include "TiBrokerBuy.h"
#include "BrokerBuyTypes.h"

namespace Broker::Buy::Window {
  class Manager_t : public ManagerBase_t {
  public:
    Manager_t(const Window_t& window) : ManagerBase_t(window) {}
  };
} // namespace Broker::Buy::Window

#endif // Include_BROKERBUYWINDOWMANAGER_H
