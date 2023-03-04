////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// DcrSetPrice.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SetPrice.h"
#include "PipelineManager.h"
#include "SsWindow.h"
#include "DdUtil.h"
#include "BrokerId.h"
#include "Price_t.h"

#include "DcrBrokerBuy.h" // savewidgets
#include "SetPriceWidgets.h"

namespace Broker::SetPrice::Translate {
  Handler_t::Handler_t(const Window_t& window) : //Window::ManagerBase_t& windowManager) :
    HandlerBase_t(
      kWindowId,
      m_TranslatePolicy,
      m_ValidatePolicy,
      L"SetPrice"),
    m_TranslatePolicy(*this, m_DcrVector),
    m_DcrPriceText(Widget::Id::PriceText),
window_(window)
//    m_windowManager(windowManager)
  {
    m_DcrVector.push_back(&m_DcrPriceText);
  }

  bool Handler_t::PreTranslateSurface(
    CSurface* pSurface,
    Ui::WindowId_t windowId,
    int dcrId,
    Rect_t* pRect) const
  {
#if 1
    static bool first = true;
    if (first) {
      Rect_t originRect(window_.GetLastOrigin(), {0, 0});
      Buy::Translate::Handler_t::SaveImageWithWidgetRects(name(), pSurface,
        originRect, std::span{ Broker::SetPrice::Widgets });
    }
    first = false;
#endif
    windowId;
    pSurface;
    window_.GetWidgetRect(dcrId, pRect);
    return true;
  }

  void Handler_t::PostData(DWORD /*Unused*/) const {
    LogAlways(L"DcrSetPrice::PostData, PriceText [%S]", m_DcrPriceText.GetText().c_str());
#if 0
    bool ExtraLog = 0;
    Price_t Price;
    if (!Price.Parse(m_DcrPriceText.GetText().c_str())) {
      LogError(L"DcrSetPrice::PostData(): Price.Parse(%S) failed",
        m_DcrPriceText.GetText().c_str());
      return;
    }
    if (ExtraLog) {
      LogAlways(L"DcrSetPrice::PostData() Price(%d)", Price);
    }
#endif
    void* pBuffer = GetPipelineManager().Alloc(sizeof(Data_t));
    if (nullptr == pBuffer) {
      LogError(L"DcrSetPrice::PostData(): Alloc callback data failed.");
    }
    else {
      Data_t* pData = new (pBuffer)
        Data_t(GetClass().c_str(), 1); //TODO Price.GetPrice());
      HRESULT hr = GetPipelineManager().Callback(pData);
      if (FAILED(hr)) {
        LogError(L"DcrSetPrice::PostData(): PM.Callback() failed.");
      }
    }
  }

} // namespace Broker::SetPrice::Translate