/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DcrSetPrice.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_DCRSETPRICE_H
#define Include_DCRSETPRICE_H

#include "DcrWindow.h"
#include "DcrWindowPolicy.h"
#include "SetPriceTypes.h"
#include "Macros.h"
#include "Rect.h"
#include "BrokerId.h"
#include "BrokerUi.h"
#include "DcrRect_t.h"
#include "DpMessage.h"
#include "dp_msg.h"
#include "Price_t.h"

namespace Broker::SetPrice::Translate {
  struct Data_t : dp::msg::Data_t {
    Data_t(Price_t pr) : dp::msg::Data_t(kMsgName), price(pr) {}

    Price_t price;
  };

  namespace msg {
    inline auto validate(const dp::msg_t& msg) {
      return dp::msg::validate<Data_t>(msg, kMsgName);
    }
  }

  namespace Legacy {
    struct Data_t : DP::Message::Data_t {
      Data_t(const int InitPrice) :
        DP::Message::Data_t(
          DP::Stage_t::Translate,
          Message::Id::SetPrice,
          sizeof(Data_t),
          kMsgName),
        Price(InitPrice)
      {}

      int Price;
    };
  }

  typedef SsWindow::Acquire::Data_t     AcquireData_t;
  typedef DcrWindow::Policy::Translate::Many_t  TranslatePolicy_t;
  typedef DcrWindow::Policy::NoValidate_t       ValidatePolicy_t;

  typedef DcrWindow::Translate::Handler_t<TranslatePolicy_t,
    ValidatePolicy_t>  HandlerBase_t;

  class Handler_t : public HandlerBase_t
  {
    friend struct Translate::Data_t;

  public:
    explicit Handler_t(const Window_t& window); //  Window::ManagerBase_t& Manager);

    // DcrWindow virtual:
    void PostData(DWORD) const override;
    bool PreTranslateSurface(
      CSurface* pSurface,
      Ui::WindowId_t windowId,
      int dcrId,
      Rect_t* pRect) const override;

//    Window::ManagerBase_t& m_windowManager;
    const Window_t& window_;
    TranslatePolicy_t m_TranslatePolicy;
    ValidatePolicy_t  m_ValidatePolicy;
    DcrVector_t       m_DcrVector;
    DcrRect_t         m_DcrPriceText;
  };
} // namespace Broker::SetPrice::Translate

#endif // Include_DCRSETPRICE_H
