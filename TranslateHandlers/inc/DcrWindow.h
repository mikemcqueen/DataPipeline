////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// DcrWindow.h
//
// Base translate handler class for DCR of a "window" - loosely defined
// as an entity with a unique ID which can be validated.
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_DCRWINDOW_H
#define Include_DCRWINDOW_H

#include <format>
#include "DpHandler_t.h"
#include "SsWindow.h"
#include "UiWindowId.h"
#include "Log.h"

class DCR;
using DcrVector_t = std::vector<DCR*>;

namespace DcrWindow {

using AcquireData_t = SsWindow::Acquire::Data_t;

namespace Translate {

  class AbstractHandler_t :
    public DP::Handler_t
  {
  public:

    // 
    // AbstractHandler_t virtual:
    //

    virtual            bool            PreTranslateSurface(
      CSurface* /*pSurface*/,
      Ui::WindowId_t /*windowId*/,
      int /*dcrId*/,
      Rect_t* /*pRect*/) const = 0;

    virtual            void            PostData(DWORD /*Unused*/) const = 0;
  };

  ////////////////////////////////////////////////////////////////////////////////

  template<
    class TranslatePolicy_t,
    class ValidatePolicy_t> //  = Policy::NoValidate_t<int, int>>
  class Handler_t :
    public AbstractHandler_t
  {
  public:

    Handler_t(
      Ui::WindowId_t     WindowId,
      TranslatePolicy_t& TranslatePolicy,
      ValidatePolicy_t& ValidatePolicy,
      const wchar_t* pName = nullptr)
      :
      m_WindowId(WindowId),
      m_TranslatePolicy(TranslatePolicy),
      m_ValidatePolicy(ValidatePolicy),
      m_name((nullptr != pName) ? pName : L"[unnamed]")
    { }

    Handler_t(const Handler_t&) = delete;

    // 
    // DP::Handler_t virtual:
    //

    bool Initialize(const wchar_t* pClass) override
    {
      return DP::Handler_t::Initialize(pClass)
        && m_TranslatePolicy.Initialize()
        && m_ValidatePolicy.Initialize();
    }

    HRESULT MessageHandler(const DP::Message::Data_t* pMessage) override
    {
      LogInfo(L"Dcr%s::MessageHandler()", m_name.c_str());
      if (Validate(pMessage, m_WindowId)) {
        auto& ssData = static_cast<const AcquireData_t&>(*pMessage);
        if (m_ValidatePolicy.Validate(ssData) &&
          m_TranslatePolicy.Translate(ssData))
        {
          PostData(0);
          return S_OK;
        }
      }
      return S_FALSE;
    }

  private:

    bool Validate(
      const DP::Message::Data_t* pMessage,
      Ui::WindowId_t       WindowId) const
    {
      Ui::WindowId_t ssWindowId = Ui::Window::Id::Unknown;
      if (0 == wcscmp(pMessage->Class, L"SsWindow")) {
        auto& ssData = static_cast<const AcquireData_t&>(*pMessage);
        ssWindowId = ssData.WindowId;
        if (ssData.WindowId == WindowId) {
          return true;
        }
      }
      LogError(L"validate failed, class: %ls, windowId actual(%d) expected(%d)",
        pMessage->Class, ssWindowId, WindowId);
      return false;
    }

  private:
    Ui::WindowId_t m_WindowId;
    TranslatePolicy_t& m_TranslatePolicy;
    ValidatePolicy_t& m_ValidatePolicy;
    wstring m_name;
  };

} // Translate
} // DcrWindow

#endif  // Include_DCRWINDOW_H
