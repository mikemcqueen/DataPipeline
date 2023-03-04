/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TiSetPrice.h
//
// Set Price popup window text interpreter.
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TISETPRICE_H
#define Include_TISETPRICE_H

/////////////////////////////////////////////////////////////////////////////

#include "DpHandler_t.h"
#include "SetPriceTypes.h"
#include "BrokerId.h"
#include "AutoCs.h"

namespace Broker {
namespace SetPrice{
namespace Interpret {
  class Handler_t :
    public DP::Handler_t
  {
  public:
    explicit Handler_t(const Window_t& window);

    // DP::Handler_t virtual:
    HRESULT MessageHandler(const DP::Message::Data_t* pData) override;
    HRESULT ExecuteTransaction(DP::Transaction::Data_t& Data) override;

  private:
#if 0
    Window::ManagerBase_t& GetManager() const { return m_Manager; }
    Window::ManagerBase_t& m_Manager;
#endif
    const Window_t& window_;
  };
} // Interpret
} // SetPrice
} // Broker

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TISetPrice

/////////////////////////////////////////////////////////////////////////////
