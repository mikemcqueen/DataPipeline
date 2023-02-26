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

namespace Ui
{

using EventId_t = DP::EventId_t;

namespace Event
{

    typedef std::vector<INPUT> InputVector_t;

    constexpr auto MakeId(const unsigned id) {
        //const unsigned max = 0x10000;
        //if consteval {
        //    static_assert(id < max);
        //}
        const auto first = static_cast<unsigned>(DP::Event::Id::Ui_First);
        return EventId_t(first + id);
    }

    namespace Id
    {
        static const auto ClickPoint = EventId_t(MakeId(0));
        static const auto ClickWidget = EventId_t(MakeId(1));
        static const auto SendChars = EventId_t(MakeId(2));
    }

    struct Data_t :
        DP::Event::Data_t
    {
        WindowId_t WindowId;

        Data_t(
            DP::Stage_t       stage        = DP::Stage_t::Any,
            DP::EventId_t     eventId      = DP::Event::Id::Unknown,
            WindowId_t        _windowId    = Window::Id::Unknown,
            DP::Event::Flag_t flags        = 0,
            size_t            size         = sizeof(Data_t))
        :
            DP::Event::Data_t(
                stage,
                eventId,
                size,
                flags),
            WindowId(_windowId)
        { }        
    };

    namespace Input
    {
        typedef unsigned Method_t;
        namespace Method
        {
            enum : Method_t
            {
                SendMessage,
                SendInput,
            };
        }
        union Destination_t
        {
            WidgetId_t WidgetId;
            POINT      Point;
        };

        struct Data_t :
            public Event::Data_t
        {
            Method_t        Method;
            Destination_t   Destination;

            // POINT constructor:
            Data_t(
                DP::EventId_t eventId,
                WindowId_t    windowId,
                POINT         point,
                Method_t      initMethod,
                size_t        size = sizeof(Data_t))
            :
                Event::Data_t(
                    DP::Stage_t::Acquire,
                    eventId,
                    windowId,
                    0,
                    size),
                Method(initMethod)
            {
                Destination.Point = point;
            }

            // WidgetId_t constructor:
            Data_t(
                DP::EventId_t eventId,
                WindowId_t    windowId,
                WidgetId_t    widgetId,
                Method_t      initMethod,
                size_t        size = sizeof(Data_t))
            :
                Event::Data_t(
                    DP::Stage_t::Acquire,
                    eventId,
                    windowId,
                    0,
                    size),
                Method(initMethod)
            {
                Destination.WidgetId = widgetId;
            }

    #if 1 // for GetEventData();
        Data_t()
        {
            SecureZeroMemory(this, sizeof(Data_t));
        }
    #endif

        };
    } // Input

    namespace Click
    {
        struct Data_t :
            public Input::Data_t
        {
            Mouse::Button_t Button;
            size_t          Count;
            bool            bDoubleClick;

            // POINT constructor:
            Data_t(
                WindowId_t      windowId,
                POINT           point,
                Input::Method_t method      = Input::Method::SendInput,
                Mouse::Button_t button      = Mouse::Button::Left,
                size_t          count       = 1,
                bool            doubleClick = false)
            :
                Input::Data_t(
                    Id::ClickPoint,
                    windowId,
                    point,
                    method,
                    sizeof(Data_t)),
                Button(button),
                Count(count),
                bDoubleClick(doubleClick)
            {
            }

            // WidgetId_t constructor:
            Data_t(
                WindowId_t      windowId,
                WidgetId_t      widgetId,
                Input::Method_t method          = Input::Method::SendInput,
                Mouse::Button_t InitButton      = Mouse::Button::Left,
                size_t          InitCount       = 1,
                bool            InitDoubleClick = false)
            :
                Input::Data_t(
                    Id::ClickWidget,
                    windowId,
                    widgetId,
                    method,
                    sizeof(Data_t)),
                Button(InitButton),
                Count(InitCount),
                bDoubleClick(InitDoubleClick)
            {
            }

    #if 1 // for GetEventData();
        Data_t()
        {
            SecureZeroMemory(this, sizeof(Data_t));
        }
    #endif

        };
    } // Click

    namespace SendChars
    {
        struct Data_t :
            public Input::Data_t
        {
            static const size_t MaxChars = 64;

            wchar_t Chars[MaxChars];

            // WidgetId_t constructor:
            Data_t(
                const wchar_t*  pText,
                WindowId_t      windowId = Ui::Window::Id::MainWindow,
                WidgetId_t      widgetId = Ui::Widget::Id::Unknown,
                Input::Method_t method   = Input::Method::SendInput)
            :
                Input::Data_t(
                    Id::SendChars,
                    windowId,
                    widgetId,
                    method,
                    sizeof(Data_t))
            {
                wcscpy_s(Chars, pText);
            }

            Data_t()
            {
                SecureZeroMemory(this, sizeof(Data_t));
            }
        };
    } // SendChars

} // Event
} // Ui

/////////////////////////////////////////////////////////////////////////////

#endif // Include_UIEVENT_H

/////////////////////////////////////////////////////////////////////////////
