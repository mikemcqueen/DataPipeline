////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// WindowType.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef INCLUDE_IDWINDOWTYPE_H
#define INCLUDE_IDWINDOWTYPE_H

#include "DpHandler_t.h"
#include "BrokerUi.h"
#include "Flag_t.h"

class CSurface;
struct Rect_t;
class MainWindow_t;

namespace Broker::Translate {
  class WindowType_t : public DP::Handler_t {
  public:
    WindowType_t(MainWindow_t& mainWindow);

    // DP::Handler_t virtual:
    HRESULT MessageHandler(const DP::Message::Data_t* pMessage) override;

    Ui::WindowId_t GetBrokerWindowId(
      const CSurface& Surface,
      Flag_t flags) const;

    Ui::WindowId_t GetLogonWindowId(
      const CSurface& Surface,
      Flag_t flags) const;

    Ui::WindowId_t GetOtherWindowId(
      const CSurface& Surface,
      Flag_t flags) const;

    Ui::WindowId_t ProcessSurface(const CSurface& surface);

  private:
    Ui::WindowId_t GetWindowId(const CSurface& Surface) const;

    Ui::WindowId_t LastOriginMatchAll(
      const CSurface& surface) const;

    Ui::WindowId_t OriginSearchAll(
      const CSurface& surface) const;

    Ui::WindowId_t SearchLoggedInWindows(
      const CSurface& surface) const;

    const char* GetWindowName(Ui::WindowId_t windowId) const;
    void UpdateLastWindowId(Ui::WindowId_t window_id) const;

  private:
    MainWindow_t& main_window_;
    mutable Ui::WindowId_t last_window_id_;
  };
} // Broker::Translate

#endif // INCLUDE_IDWINDOWTYPE_H