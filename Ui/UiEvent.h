/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// UiEvent.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_UIEVENT_H
#define Include_UIEVENT_H

#include "UiTypes.h"
#include "UiWindowId.h"
#include "DpEvent.h"

namespace Ui {

using DP::EventId_t;

namespace Event {

  typedef std::vector<INPUT> InputVector_t;

  constexpr auto MakeId(const unsigned id) {
    //const unsigned max = 0x10000;
    //if consteval {
    //    static_assert(id < max);
    //}
    const auto first = static_cast<unsigned>(DP::Event::Id::Ui_First);
    return EventId_t(first + id);
  }

  namespace Id {
    constexpr auto ClickPoint = EventId_t(MakeId(0));
    constexpr auto ClickWidget = EventId_t(MakeId(1));
    constexpr auto SendChars = EventId_t(MakeId(2));
  }

  struct Data_t : DP::Event::Data_t {
    Data_t(
      DP::Stage_t       stage = DP::Stage_t::Any,
      DP::EventId_t     eventId = DP::Event::Id::Unknown,
      WindowId_t        windowId = Window::Id::Unknown,
      DP::Event::Flag_t flags = 0,
      size_t            size = sizeof(Data_t)) :
      DP::Event::Data_t(stage, eventId, size, flags),
      WindowId(windowId)
    {}

    WindowId_t WindowId;
  };

  namespace Input {
    enum class Method {
      SendMessage,
      SendInput,
    };
    union Destination_t {
      WidgetId_t WidgetId;
      POINT      Point;
    };

    struct Data_t : public Event::Data_t {
      // POINT constructor:
      Data_t(
        DP::EventId_t eventId,
        WindowId_t    windowId,
        POINT         point,
        DP::Event::Flag_t        flags,
        Method        method,
        size_t        size = sizeof(Data_t)) :
        Event::Data_t(DP::Stage_t::Acquire, eventId, windowId, flags, size),
        Method(method)
      {
        Destination.Point = point;
      }

      // WidgetId_t constructor:
      Data_t(
        DP::EventId_t eventId,
        WindowId_t    windowId,
        WidgetId_t    widgetId,
        DP::Event::Flag_t        flags,
        Method        method,
        size_t        size = sizeof(Data_t)) :
        Event::Data_t(DP::Stage_t::Acquire, eventId, windowId, flags, size),
        Method(method)
      {
        Destination.WidgetId = widgetId;
      }

      // TODO hack for GetEventData();
      Data_t() {
        SecureZeroMemory(this, sizeof(Data_t));
      }

      Method        Method;
      Destination_t Destination;
    };
  } // Input

  namespace Click {
    struct Data_t : public Input::Data_t {
      // POINT constructor:
      Data_t(
        WindowId_t      windowId,
        POINT           point,
        DP::Event::Flag_t flags,
        Input::Method   method = Input::Method::SendInput,
        Mouse::Button_t button = Mouse::Button::Left,
        size_t          count = 1,
        bool            doubleClick = false) :
        Input::Data_t(Id::ClickPoint, windowId, point, flags, method, sizeof(Data_t)),
        Button(button),
        Count(count),
        bDoubleClick(doubleClick)
      {
      }

      // WidgetId_t constructor:
      Data_t(
        WindowId_t      windowId,
        WidgetId_t      widgetId,
        DP::Event::Flag_t flags,
        Input::Method   method = Input::Method::SendInput,
        Mouse::Button_t InitButton = Mouse::Button::Left,
        size_t          InitCount = 1,
        bool            InitDoubleClick = false) :
        Input::Data_t(Id::ClickWidget, windowId, widgetId, flags, method, sizeof(Data_t)),
        Button(InitButton),
        Count(InitCount),
        bDoubleClick(InitDoubleClick)
      {
      }

      // hack for GetEventData();
      Data_t() {
        SecureZeroMemory(this, sizeof(Data_t));
      }

      Mouse::Button_t Button;
      size_t          Count;
      bool            bDoubleClick;
    };
  } // Click

  namespace SendChars {
    struct Data_t : public Input::Data_t {
      static constexpr auto kMaxChars = 64;
      std::array<char, kMaxChars> Chars;

      // WidgetId_t constructor:
      Data_t(
        std::string_view text,
        WindowId_t      windowId = Ui::Window::Id::MainWindow,
        WidgetId_t      widgetId = Ui::Widget::Id::Unknown,
        DP::Event::Flag_t flags = 0,
        Input::Method   method = Input::Method::SendInput) :
        Input::Data_t(Id::SendChars, windowId, widgetId, flags, method, sizeof(Data_t))
      {
        Chars.fill('\0');
        text.copy(Chars.data(), Chars.size() - 1);
      }

      Data_t() {
        SecureZeroMemory(this, sizeof(Data_t));
      }
    };
  } // SendChars
} // Event
} // Ui

#endif // Include_UIEVENT_H
