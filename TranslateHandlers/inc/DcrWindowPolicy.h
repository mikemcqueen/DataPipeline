////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// DcrWindowPolicy.h
//
// Translate and Validate policies for DcrWindow.
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_DCRWINDOWPOLICY_H
#define Include_DCRWINDOWPOLICY_H

#include "DcrWindow.h"

namespace DcrWindow::Policy { // TODO Strategy?
  namespace Translate {
    class Many_t {
      using Handler_t = DcrWindow::Translate::AbstractHandler_t;

    public:

      explicit Many_t(
        const Handler_t& handler,
        DcrVector_t& DcrVector);

      ~Many_t();

      Many_t() = delete;
      Many_t(const Many_t&) = delete;
      Many_t& operator=(const Many_t&) = delete;

      bool Initialize();

      bool Translate(const AcquireData_t& Data);

      // new-dp hack
      static bool Translate(CSurface& surface, DcrVector_t& dcrVector,
        Ui::WindowId_t windowId, const Handler_t& handler);
        
    private:
      const Handler_t& handler_;
      DcrVector_t& m_DcrVector;
    };
  } // Translate

  class NoValidate_t {
  public:
    NoValidate_t() = default;

    bool Initialize() { return true; }
    bool Validate(const AcquireData_t& /*Data*/) { return true; }
  };

#if PENDING
  class ValidateWindowPolicy_t
  {

  public:

    ValidateWindowPolicy_t();

    bool Initialize();

    bool Validate(
        const SsTrades_t::ScreenShotData_t* pData);

  private:

    bool InitAllBitmaps();

    bool ValidateSides(
        const CSurface* pSurface,
        const RECT& rcBounds);

    bool ValidateCorners(
        const CSurface* pSurface,
        const RECT& rcBounds);
  };
#endif

} // DcrWindow::Policy

#endif // Include_DCRWINDOWPOLICY_H