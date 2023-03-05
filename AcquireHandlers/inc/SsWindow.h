////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// SsWindow.h
//
// Screenshot acquire handler with knowledge of Window behaviors such
// as clicking, scrolling, and entering text.
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_SSWINDOW_H
#define Include_SSWINDOW_H

#include "SsTask_t.h"
#include "UiWindow.h"
#include "UiEvent.h"
#include "UiWindowId.h"

namespace SsWindow::Acquire {
  namespace Legacy {
    struct Data_t : SsTask::Acquire::Legacy::Data_t {
      Ui::WindowId_t WindowId;

      Data_t(
        const wchar_t* pClass,
        Ui::WindowId_t InitWindowId,
        pool<CSurface>::item_t* pPoolItem,
        size_t Size = sizeof(Data_t)) :
        SsTask::Acquire::Legacy::Data_t(pClass, pPoolItem, Size),
        WindowId(InitWindowId)
      {}
    };
  } // namespace Legacy

  struct Data_t : SsTask::Acquire::Data_t {
    Data_t(std::string_view msg_name, pool<CSurface>::item_t& pool_item,
      Ui::WindowId_t wid) :
      SsTask::Acquire::Data_t(msg_name, pool_item),
      window_id(wid)
    {}

    Ui::WindowId_t window_id;
  };

  class Handler_t :public SsTask_t {
  public:
    Handler_t(CDisplay& Display, Ui::Window::WithHandle_t& Window);

    // DP::Handler_t virtual:
    HRESULT EventHandler(DP::Event::Data_t& Event) override;

    void ThreadProcessEvent() override;

    void PostData(
      HWND hWnd,
      pool<CSurface>::item_t* pPoolItem) override;

    bool ToggleClick() { return m_bClick = !m_bClick; }

  private:
    void AsyncEvent(const DP::Event::Data_t& Data);

    void ClickPoint(
      const Ui::Event::Click::Data_t& Data,
      POINT Point);

    void ClickWidget(const Ui::Event::Click::Data_t& Data);

    void SendChars(const Ui::Event::SendChars::Data_t& Data);

  private:
    Ui::Window::WithHandle_t& m_Window;
    bool m_bClick;
  }; // Handler_t
}// SsWindow::Acquire

#endif // Include_SSWINDOW_H
