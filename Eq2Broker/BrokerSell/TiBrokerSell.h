/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TiBrokerSell.h
//
// Broker Sell window text interpreter.
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TIBROKERSELL_H
#define Include_TIBROKERSELL_H

#include "DpHandler_t.h"
#include "BrokerSellTypes.h"
#include "BrokerId.h"
//#include "TiBase_t.h"
#include "AutoCs.h"
//#include "PageNumber_t.h"

//class PageNumber_t;

namespace Broker::Sell::Interpret {
  class Handler_t : public DP::Handler_t {
  public:
    explicit Handler_t(const Window_t& window);
    //explicit Handler_t(Window::ManagerBase_t& Manager);

    Handler_t() = default;

    // DP::Handler_t virtual:
    HRESULT MessageHandler(const DP::Message::Legacy::Data_t* pData) override;
    HRESULT ExecuteTransaction(DP::Transaction::Data_t& Data) override;

    static bool IsScrolledToTop(const Translate::Legacy::Data_t& Message);
    static bool IsScrolledToBottom(const Translate::Legacy::Data_t& Message);

  private:
#if 0
    Window::ManagerBase_t& GetManager() const { return m_windowManager; }

    Window::ManagerBase_t& m_windowManager;
#endif
    const Window_t& window_;
  };
} // namespace Broker::Sell::Interpret

#endif // Include_TIBROKERSELL_H

/////////////////////////////////////////////////////////////////////////////
