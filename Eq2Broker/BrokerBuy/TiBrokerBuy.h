/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// TiBrokerBuy.h
//
// Broker Buy window text interpreter.
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TIBROKERBUY_H
#define Include_TIBROKERBUY_H

#include "DpHandler_t.h"
#include "DpTransaction.h"
#include "BrokerBuyTypes.h"
#include "AutoCs.h"
#include "PageNumber_t.h"

namespace Broker::Buy::Interpret {
  class Handler_t : public DP::Handler_t {
  public:
    explicit Handler_t(const Window_t& window); //  Window::ManagerBase_t& windowManager);

    // DP::Handler_t virtual:
    bool Initialize(const wchar_t* pszClass) override;
    HRESULT MessageHandler(const DP::Message::Legacy::Data_t* pData) override;

  private:
#if 0
    Window::ManagerBase_t& GetWindowManager() const { return windowManager_; }

    Window::ManagerBase_t& windowManager_;
#endif
    const Window_t& window_;
    PageNumber_t m_PageNumber;
  };
} // Broker::Buy::Interpret

#endif // Include_TIBROKERBUY
