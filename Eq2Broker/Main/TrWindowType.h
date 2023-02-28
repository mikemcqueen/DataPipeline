////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TrWindowType.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef INCLUDE_IDWINDOWTYPE_H
#define INCLUDE_IDWINDOWTYPE_H

#include "DpHandler_t.h"
#include "BrokerUi.h"
#include "Flag_t.h"
//#include "UiWindowId.h"

class CSurface;
class Rect_t;
class MainWindow_t;

namespace Broker
{

  class TrWindowType_t : public DP::Handler_t {
    typedef Ui::WindowId_t(TrWindowType_t::* FnGetWindowId_t)(const CSurface& surface, Flag_t flags) const;

    static const FnGetWindowId_t s_brokerFunc;
    static const FnGetWindowId_t s_mainChatFunc;
    static const FnGetWindowId_t s_windowIdFuncs[];

  public:
    TrWindowType_t(MainWindow_t& mainWindow);

    // DP::Handler_t virtual:
    HRESULT MessageHandler(const DP::Message::Data_t* pMessage) override;

  private:
    Ui::WindowId_t GetWindowId(const CSurface& Surface) const;

    const char* GetWindowName(Ui::WindowId_t windowId) const;

    Ui::WindowId_t GetBrokerWindowId(
      const CSurface& Surface,
      Flag_t flags) const;

    Ui::WindowId_t GetLogonWindowId(
      const CSurface& Surface,
      Flag_t flags) const;

    Ui::WindowId_t GetOtherWindowId(
      const CSurface& Surface,
      Flag_t flags) const;

    Ui::WindowId_t GetMainChatWindowId(
      const CSurface& Surface,
      Flag_t flags) const;

    Ui::WindowId_t CompareAllLastOrigins(
      const CSurface& surface) const;

    Ui::WindowId_t SearchLoggedInWindows(
      const CSurface& surface) const;

    Ui::WindowId_t SearchAllWindows(
      const CSurface& surface) const;

  private:
    TrWindowType_t();
    TrWindowType_t(const TrWindowType_t&);
    TrWindowType_t& operator=(const TrWindowType_t&);

    // Member data:
    MainWindow_t& m_mainWindow;
    mutable Ui::WindowId_t  m_lastWindowId;
    mutable FnGetWindowId_t m_lastWindowIdFunc;
  };

} // Broker

#endif // INCLUDE_IDWINDOWTYPE_H