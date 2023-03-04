////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// WindowType_t.cpp
//
// Window type translate handler.
//
// Typically the first translate handler in the stack; determine the window
// type (Id) of the supplied acquire data (i.e. screenshot).
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TrWindowType.h"
#include "MainWindow_t.h"
#include "SsWindow.h"
#include "BrokerWindow.h"
#include "TabWindow.h"
#include "DdUtil.h"
#include "Log.h"
#include "BrokerUi.h"
#include "OtherWindows.h"

extern bool g_bWriteBmps;

namespace Broker::Translate {
  static std::unordered_map<Ui::WindowId_t, bool> window_ids = {
    { Broker::Window::Id::BrokerFrame, true }
  };

  WindowType_t::WindowType_t(MainWindow_t& mainWindow) :
    main_window_(mainWindow),
    last_window_id_(Ui::Window::Id::Unknown)
  {}

  HRESULT WindowType_t::MessageHandler(const DP::Message::Data_t* pMessage) {
    LogInfo(L"WindowType::MessageHandler()");
    // TODO: access SsWindow::GetClass() from instance? make class static? why is there a class.
    if (0 != wcscmp(pMessage->Class, L"SsWindow")) {
      LogInfo(L"WindowType_t::MessageHandler(): Data type not supported, %s - expected %s",
        pMessage->Class, L"SsWindow");
      return S_FALSE;
    }
    auto& ssData = *static_cast<const SsWindow::Acquire::Data_t*>(pMessage);
    using namespace Ui::Window;
    assert(Id::Unknown == ssData.WindowId);

    // Try to determine the Window Id by looking at the screenshot bits
    Ui::WindowId_t windowId = GetWindowId(*ssData.pPoolItem->get());
    // If we determined a valid Window Id, hack the id into the message
    if (Id::Unknown != windowId) {
      LogInfo(L"WindowType_t::MessageHandler() Matched window Id(%d) Name(%S)",
        windowId, GetWindowName(windowId));
      const_cast<SsWindow::Acquire::Data_t&>(ssData).WindowId = windowId;
      // i.e.: LastKnownGOODWindowId
      UpdateLastWindowId(windowId);
      return S_OK;
    }
    // Can't determine the Window Id, abort processing this message
    LogInfo(L"WindowType_t::MessageHandler(): Unknown WindowId - "
      "aborting further handler processing");
    return E_ABORT;
  }

  Ui::WindowId_t WindowType_t::GetWindowId(const CSurface& surface) const {
    using namespace Ui::Window;
    Ui::WindowId_t windowId = Id::Unknown;
    if (last_window_id_ != Id::Unknown) {
      // TODO: helper function
      if (main_window_.GetWindow(last_window_id_).IsLocatedOn(surface, LocateBy::LastOriginMatch)) {
        windowId = last_window_id_;
      }
    }
    // No last window, or last window mismatch
    if (windowId == Id::Unknown) {
      windowId = LastOriginMatchAll(surface);
      if (Id::Unknown == windowId) {
        windowId = OriginSearchAll(surface);
      }
    }
    if (windowId != Id::Unknown) {
      // Found topmost window. Now check for children.
      POINT pt{};
      windowId = main_window_.GetWindow(windowId).GetWindowId(surface, &pt);
      assert(windowId != Id::Unknown); // coz i want to know.
    }
    return windowId;
  }

  Ui::WindowId_t WindowType_t::LastOriginMatchAll(const CSurface& surface) const {
    using namespace Ui::Window;
    // attempt 'last origin match' of all windows on the supplied surface
    // (except for last_window_id_)
    // TODO: this is broken if we have windows that don't have a ptLastOrigin set.
    // maybe it should be an optional<POINT> ?
    for (auto window_id : std::views::keys(window_ids)) {
      if (window_id == last_window_id_) continue;
      if (main_window_.GetWindow(window_id).IsLocatedOn(surface, LocateBy::LastOriginMatch)) {
        return window_id;
      }
    }
    return Id::Unknown;
  }

  Ui::WindowId_t WindowType_t::OriginSearchAll(const CSurface& surface) const {
    using namespace Ui::Window;
    for (auto window_id : std::views::keys(window_ids)) {
      if (main_window_.GetWindow(window_id).IsLocatedOn(surface, LocateBy::OriginSearch)) {
        return window_id;
      }
    }
    return Id::Unknown;
  }

#if 0
  Ui::WindowId_t WindowType_t::SearchLoggedInWindows(const CSurface& surface) const {
    // check all 'logged in' windows (currently only broker window)
    using namespace Ui::Window;
    Ui::WindowId_t windowId = (this->*s_brokerFunc)(surface, LocateBy::OriginSearch);
    if (Id::Unknown != windowId) {
      m_lastWindowIdFunc = s_brokerFunc;
    }
    return windowId;
  }
#endif

  const char* WindowType_t::GetWindowName(Ui::WindowId_t windowId) const {
    const char* pName = "Error";
    switch (windowId) {
    case Ui::Window::Id::Unknown:
      pName = "Unknown";
      break;
    default:
      pName = main_window_.GetWindow(windowId).GetWindowName();
      break;
    }
    return pName;
  }

  Ui::WindowId_t WindowType_t::GetBrokerWindowId(
    const CSurface& surface,
    Flag_t flags) const
  {
    Ui::WindowId_t windowId = Ui::Window::Id::Unknown;
    POINT origin = { 0, 0 };
    if (main_window_.GetBrokerWindow().IsLocatedOn(surface, flags, &origin)) {
      windowId = main_window_.GetBrokerWindow().GetWindowId(surface);
#if 0
      if (Ui::Window::Id::Unknown == windowId) {
        windowId = Window::Id::BrokerFrame;
      }
#endif
    }
    return windowId;
  }

  Ui::WindowId_t WindowType_t::GetLogonWindowId(
    const CSurface& surface,
    Flag_t flags) const
  {
    Ui::WindowId_t windowId = Window::Id::Eq2Login;
    return main_window_.GetWindow(windowId).IsLocatedOn(surface, flags) ?
      windowId : Ui::Window::Id::Unknown;
  }

  Ui::WindowId_t WindowType_t::GetOtherWindowId(
      const CSurface& surface,
      Flag_t    flags) const
  {
    constexpr Ui::WindowId_t otherWindowIds[] = {
      Window::Id::Eq2Loading,
      Window::Id::Zoning,
    };

    for (int index = 0; index < _countof(otherWindowIds); ++index) {
      if (main_window_.GetWindow(otherWindowIds[index]).IsLocatedOn(surface, flags)) {
        return otherWindowIds[index];
      }
    }
    return Ui::Window::Id::Unknown;
  }

  void WindowType_t::UpdateLastWindowId(Ui::WindowId_t window_id) const {
    if (last_window_id_ != window_id) {
      last_window_id_ = window_id;
      LogAlways(L"WindowType_t::UpdateLastWindowId() updated to (%d:%S)",
        (int)window_id, GetWindowName(window_id));
    }
  }

} // namespace Broker
