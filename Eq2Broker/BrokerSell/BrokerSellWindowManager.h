///////////////////////////////////////////////////////////////////////////////
//
// BrokerSellWindowManager.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_BROKERSELLWINDOWMANAGER_H
#define Include_BROKERSELLWINDOWMANAGER_H

#include "BrokerSellWindow.h"
#include "DcrBrokerSell.h"
#include "TiBrokerSell.h"
#include "BrokerSellTypes.h"

namespace Broker::Sell::Window {
  class Manager_t : public ManagerBase_t {
  public:
    Manager_t(const Window_t& window) : ManagerBase_t(window) {}
  };
} // namespace Broker::Sell::Window

#endif // Include_BROKERSELLWINDOWMANAGER_H
