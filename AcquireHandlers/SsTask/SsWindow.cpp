/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// SsWindow.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SsWindow.h"
#include "PipelineManager.h"
#include "Macros.h"
#include "DdUtil.h"
#include "UiInput.h"

namespace SsWindow::Acquire {
  Handler_t::Handler_t(CDisplay& Display, Ui::Window::WithHandle_t& Window) :
    SsTask_t(Display, 1280, 1024,
      [&Window](Rect_t& rect) {
        Window.SyncHwnd();
        if (Window.IsVisibleTopWindow()) {
          Window.GetClientRect(rect);
          return Window.Hwnd();
        }
        return (HWND)nullptr;
      }),
    m_Window(Window),
    m_bClick(true)
  {
  }

#pragma warning(disable:4063) // invalid enum case

  HRESULT Handler_t::EventHandler(DP::Event::Data_t& Data) {
    HRESULT hr = SsTask_t::EventHandler(Data);
    if (S_FALSE != hr)
      return hr;

    assert(DP::Message::Type::Event == Data.Type);

    using namespace Ui::Event;
    switch (Data.Id) {

      //    case Id::Scroll:
    case Id::ClickPoint:
    case Id::ClickWidget:
    case Id::SendChars:
      AsyncEvent(Data);
      break;

      /*
          case Id::ThumbPosition:
              OnThumbPositionEvent(static_cast<LonWindow_t::EventThumbPosition_t::Data_t&>(Data));
              break;
      */

    default:
      return S_FALSE;
    }
    return S_OK;
  }

  void Handler_t::AsyncEvent(const DP::Event::Data_t& Data) {
    LogInfo(L"SsWindow::AsyncEvent(%d, %d)", Data.Id, Data.Size);

    if (!SetEventPending(true)) {
      DP::Event::Data_t EventData(DP::Stage_t::Any);
      GetEventData(0, EventData, sizeof(EventData));

      //        const DP::Event::Data_t& EventData = GetEventData(0);
      //            *reinterpret_cast<const DP::Event::Data_t&>(GetEventData(0));
      LogError(L"SsWindow::AsyncEvent: Nested pending event"
        L"Old = (%d,%d), New = (%d,%d)",
        EventData.Id, EventData.Size, Data.Id, Data.Size);
      return;
    }
    SuspendAndFlush();

    using namespace Ui::Event;
    switch (Data.Id)
    {
      //    case Id::Scroll:
    case Id::ClickPoint:
    case Id::ClickWidget:
    case Id::SendChars:
      SetEventData(Data);
      break;

    default:
      ASSERT(false);
      return;
    }
    SetAsyncEvent();
  }

  void Handler_t::ThreadProcessEvent() {
    if (!IsEventPending()) {
      LogError(L"SsWindow::ThreadProcessEvent(): No event pending.");
      return;
    }
    using namespace Ui::Event;
    for (size_t Event = 0; Event < GetEventCount(); ++Event) {
      const DP::Event::Data_t& eventData = PeekEvent(Event);
      switch (eventData.Id) {
      case Id::ClickPoint:
        [[fallthrough]];

      case Id::ClickWidget:
      {
        Click::Data_t ClickData;
        GetEventData(Event, ClickData, sizeof(ClickData));
        if (Id::ClickPoint == ClickData.Id) {
          ClickPoint(ClickData, ClickData.Destination.Point);
        }
        else {
          ClickWidget(ClickData);
        }
      }
      break;

      case Id::SendChars:
      {
        SendChars::Data_t sendCharsData;
        GetEventData(Event, sendCharsData, sizeof(sendCharsData));
        SendChars(sendCharsData);
      }
      break;
      /*
              case DP::ScreenShot:
                  ASSERT(false);
                  Shutter();
                  break;
      */
      default:
        LogError(L"SsWindow: Unknown event (%d)", eventData.Id);
        ASSERT(false);
        break;
      }
    }
    SetEventPending(false);
    Resume();
  }

  void Handler_t::ClickPoint(
    const Ui::Event::Click::Data_t& Data,
    POINT Point)
  {
    LogInfo(L"SsWindow::ClickPoint() (%d,%d)", Point.x, Point.y);
    if (!m_bClick || !m_Window.IsVisibleTopWindow()) {
      return;
    }
    size_t Count = Data.Count;
    using namespace Ui::Event;
    while (0 < Count--) {
      switch (Data.Method) {
#if 0
      case Input::Method::SendMessage:
        Ui::Window::ClickSendMessage(hWnd, MAKELONG(Point.x, Point.y),
          Data.Button, Data.bDoubleClick);
        break;
#endif
      case Input::Method::SendInput:
        Ui::Input_t::Click(m_Window.Hwnd(), Point);
        break;
      default:
        LogError(L"SsWindow::ClickPoint() invalid method (%d)", Data.Method);
        throw std::invalid_argument("SsWindow::ClickPoint() method");
      }
    }
  }

  void Handler_t::ClickWidget(const Ui::Event::Click::Data_t& Data) {
    const Ui::WidgetId_t WidgetId = Data.Destination.WidgetId;
    LogInfo(L"SsWindow::ClickWidget(%d)", WidgetId);
    if (m_bClick && m_Window.IsVisibleTopWindow()) {
      m_Window.ClickWidget(m_Window.GetWindow(Data.WindowId), WidgetId);
    }
  }

  void Handler_t::SendChars(const Ui::Event::SendChars::Data_t& Data) {
    LogInfo(L"SsWindow::SendChars(%ls)", Data.Chars);
    if (!m_bClick || !m_Window.IsVisibleTopWindow()) {
      return;
    }
    const Ui::WidgetId_t WidgetId = Data.Destination.WidgetId;
    if (Ui::Widget::Id::Unknown != WidgetId) {
      Rect_t rect;
      if (!m_Window.GetWindow(Data.WindowId).GetWidgetRect(WidgetId, &rect)) {
        LogError(L"SsWindow::SendChars(): GetWidgetRect(%d) failed", WidgetId);
        return;
      }
      Ui::Input_t::Click(m_Window.Hwnd(), rect.Center());
    }
    Ui::Input_t::SendChars(Data.Chars);
  }

  void Handler_t::PostData(
    HWND, //hWnd,
    pool<CSurface>::item_t* pPoolItem)
  {
    //using Legacy::Data_t;
    LogInfo(L"SsWindow::PostData()");
    void* pBuffer = GetPipelineManager().Alloc(sizeof(Data_t));
    if (nullptr == pBuffer) {
      LogError(L"SsWindow::PostData(): Alloc callback data failed.");
    }
    else {
      pPoolItem->addref(); // Haxington Heights
      Data_t* pData = new (pBuffer)
        Data_t(
          GetName().c_str(),
          Ui::Window::Id::Unknown,
          pPoolItem);

      LogInfo(L"SsWindow_t::PostData: data %S", typeid(*pData).name());

      HRESULT hr = GetPipelineManager().Callback(pData);
      if (FAILED(hr)) {
        LogError(L"SsWindow::PostData(): PM.Callback() failed (%08x)", hr);
      }
    }
  }
} // namespace SsWindow::Acquire
