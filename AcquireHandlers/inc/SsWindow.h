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
  struct Data_t : SsTask::Acquire::Data_t {
    Ui::WindowId_t WindowId;

    Data_t(
      const wchar_t* pClass,
      Ui::WindowId_t InitWindowId,
      pool<CSurface>::item_t* pPoolItem,
      size_t Size = sizeof(Data_t)) :
      SsTask::Acquire::Data_t(pClass, pPoolItem, Size),
      WindowId(InitWindowId)
    {}
  };

  class Handler_t :public SsTask_t {
  public:
    Handler_t(CDisplay& Display, Ui::Window::WithHandle_t& Window);

    // DP::Handler_t virtual:
    HRESULT EventHandler(DP::Event::Data_t& Event) override;

    // SSTask virtual:
    //HWND GetSsWindowRect(RECT& rcBounds) const override;

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

/*
    bool ValidateEventData(
      const DP::Event::Data_t& Data,
      size_t Size);
*/

//    HWND ValidateWindow(Ui::WindowId_t WindowId);

  private:
    Ui::Window::WithHandle_t& m_Window;
    bool m_bClick;
  }; // Handler_t
}// SsWindow::Acquire

#endif // Include_SSWINDOW_H
