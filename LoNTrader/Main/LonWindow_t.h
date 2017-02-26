/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// LonWindow_t.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_LONWINDOW_T_H
#define Include_LONWINDOW_T_H

#include "LonTypes.h"
#include "EventTypes.h"

/////////////////////////////////////////////////////////////////////////////

class LonTrader_t;

class LonWindow_t
{
public:

    struct Handle_t
    {
        HWND           hWnd;
        Lon::Window::Type_e Type;

        Handle_t() :
            hWnd(NULL),
                Type(Lon::Window::Unknown)
        { }
    };

    class EventScroll_t
    {
    public:

        enum Type_e
        {
            PageUp,
            LineUp,
            PageDown,
            LineDown
        };

        struct Data_t :
            public Lon::Event::Data_t
        {
            Lon::ScrollBar_t::Type_e ScrollBar;
            Type_e              ScrollType;
            size_t              ScrollCount;
            size_t              ScrollPageLines;

            Data_t(
                Lon::Window::Type_e           WindowType,
                Lon::ScrollBar_t::Type_e InitScrollBar,
                Type_e                   InitScrollType,
                size_t                   InitScrollCount,
                size_t                   InitScrollPageLines)
            :
                Lon::Event::Data_t(
                    DP::Stage::Acquire,
                    Lon::Event::Id::Scroll,
                    WindowType,
                    0,
                    sizeof(Data_t)),
                ScrollBar(InitScrollBar),
                ScrollType(InitScrollType),
                ScrollCount(InitScrollCount),
                ScrollPageLines(InitScrollPageLines)
            { }

            Data_t()
            {
                SecureZeroMemory(this, sizeof(Data_t));
            }
        } m_Data;

        EventScroll_t(
            Lon::Window::Type_e            WindowType,
            Lon::ScrollBar_t::Type_e  ScrollBar,
            Type_e                    ScrollType,
            size_t                    ScrollCount = 1,
            size_t                    ScrollPageLines = 0)
        :
            m_Data(
                WindowType,
                ScrollBar,
                ScrollType,
                ScrollCount,
                ScrollPageLines)
        { }
    };

    class EventThumbPosition_t
    {
    public:

        struct Data_t :
            public Lon::Event::Data_t
        {
            Lon::ScrollBar_t ScrollBar;

            Data_t(
                Lon::Window::Type_e   WindowType,
                Lon::ScrollBar_t& InitScrollBar)
            :
                Lon::Event::Data_t(
                    DP::Stage::Acquire,
                    Lon::Event::Id::ThumbPosition,
                    WindowType,
                    0,
                    sizeof(Data_t)),
                ScrollBar(InitScrollBar)
            { }

            Data_t()
            {
                SecureZeroMemory(this, sizeof(Data_t));
            }
        } m_Data;

        EventThumbPosition_t(
            Lon::Window::Type_e   WindowType,
            Lon::ScrollBar_t& ScrollBar)
        :
            m_Data(WindowType, ScrollBar)
        { }
    };

    class EventClick_t
    {
    public:

        enum Button_e
        {
            LeftButton,
            MiddleButton,
            RightButton
        };

        struct Data_t :
            public Lon::Event::Data_t
        {
            Button_e Button;
            POINT    Point;
            size_t   Count;
            bool     bDoubleClick;

            Data_t(
                Lon::Window::Type_e WindowType,
                Button_e       InitButton,
                POINT          InitPoint,
                size_t         InitCount,
                bool           InitDoubleClick)
            :
                Lon::Event::Data_t(
                    DP::Stage::Acquire,
                    Lon::Event::Id::Click,
                    WindowType,
                    0,
                    sizeof(Data_t)),
                Button(InitButton),
                Point(InitPoint),
                Count(InitCount),
                bDoubleClick(InitDoubleClick)
            { }

            Data_t()
            {
                SecureZeroMemory(this, sizeof(Data_t));
            }
        } m_Data;

        EventClick_t(
            Lon::Window::Type_e WindowType,
            Button_e       Button,
            POINT          Point,
            size_t         Count,
            bool           bDoubleClick = false)
        :
            m_Data(WindowType, Button, Point, Count, bDoubleClick)
        { }
    };

    class EventSendChars_t
    {
    public:

        static const size_t MaxChars = 64;

        struct Data_t :
            public Lon::Event::Data_t
        {
            wchar_t Chars[MaxChars];

            Data_t(
                Lon::Window::Type_e WindowType,
                const wchar_t* Text)
            :
                Lon::Event::Data_t(
                    DP::Stage::Acquire,
                    Lon::Event::Id::SendChars,
                    WindowType,
                    0,
                    sizeof(Data_t))
            {
                wcscpy_s(Chars, Text);
            }

            Data_t()
            {
                SecureZeroMemory(this, sizeof(Data_t));
            }
        } m_Data;

        EventSendChars_t(
            Lon::Window::Type_e WindowType,
            const wchar_t* Text)
        :
            m_Data(WindowType, Text)
        { }
    };

    static 
    const wchar_t*
    GetWindowTitle(
        const Lon::Window::Type_e Type);

private:

    static 
    HWND m_hWnd;

    LonTrader_t *m_pTrader;

public:

    LonWindow_t();

    bool
    Initialize();

    static
    Handle_t
    GetTopWindow();

    static
    HWND
    GetSsWindowRect(
        RECT& rcBounds);// const;

    static
    Lon::Window::Type_e
    GetSsWindowType(
        Lon::Window::Type_e TopWindowType);

    static
    HWND
    GetWindow(
        Lon::Window::Type_e Type);

    static
    Lon::Window::Type_e
    GetWindowType(
        HWND hWnd);

    void
    GetClientRect(
        Lon::Window::Type_e Type,
        RECT&               Rect) const;

    static
    size_t
    GetWindowText(
        HWND     hWnd,
        wchar_t* pBuffer,
        size_t   BufferLen);

    static
    bool
    GetWindowRect(
        Lon::Window::Type_e Type,
        RECT&          Rect,
        Lon::Window::Type_e RelativeType);

    static
    bool
    ConvertRect(
        Lon::Window::Type_e Type,
        RECT&          Rect,
        Lon::Window::Type_e RelativeType);

    static
    void
    ConvertRect(
        HWND  hWnd,
        RECT& Rect,
        HWND  hRelative);

    static
    bool
    Click(
        Lon::Window::Type_e    WindowType,
        const RECT*            pRect        = NULL,
        EventClick_t::Button_e Button       = EventClick_t::LeftButton,
        size_t                 Count        = 1,
        bool                   bDoubleClick = false,
        bool                   bDirect      = false);


    static
    void
    Click(
        HWND                   hWnd, 
        LPARAM                 lParam,
        EventClick_t::Button_e Button       = EventClick_t::LeftButton,
        bool                   bDoubleClick = false);

    static
    bool
    SendChars(
        Lon::Window::Type_e WindowType,
        const wchar_t*      pszText);

    static
    void
    SendCharsDirect(
                HWND     hWnd,
          const wchar_t* pszText);

    static
    void
    SendKey(
        HWND    hWnd, 
        wchar_t Key);

    static 
    bool
    CanScroll(
        Lon::Window::Type_e ScrollType);

    static
    bool
    Scroll(
        Lon::Window::Type_e      WindowType,
        Lon::ScrollBar_t::Type_e ScrollBar,
        EventScroll_t::Type_e    ScrollType,
        size_t                   Count = 1,
        size_t                   PageLines = 0);

    static
    Lon::ScrollBar_t::ThumbPosition_e
    GetThumbPosition(
        Lon::Window::Type_e          WindowType,
        Lon::ScrollBar_t::Type_e ScrollBar);

    static
    HWND
    ValidateWindow(
        Lon::Window::Type_e WIndowType);

private:

    static
    HWND
    FindPromptWindow(
        const Lon::Window::Type_e WindowType);

    static
    HWND
    FindChildWindow(
              Lon::Window::Type_e   TopWindowType,
        const wchar_t* ChildWindows[] = NULL,
              size_t   Count          = 0,
        const wchar_t* WindowName     = NULL);

    static
    HWND
    WalkChildWindowTitles(
              HWND hWnd,
        const wchar_t* ChildWindowTitles[],
        const size_t   Count);

    static
    HWND
    FindPostedTradesWindow(
        Lon::Window::Type_e WindowType);

    static
    HWND
    FindPostedTradeDetailWindow(
        Lon::Window::Type_e WindowType);

    static
    HWND
    FindTradeBuilderWindow(
        Lon::Window::Type_e WindowType);

    static
    HWND
    FindConfirmTradeWindow(
        Lon::Window::Type_e WindowType);

    static
    Lon::Window::Type_e
    GetWindowType(
        const wchar_t* pszTitle);

    void
    GetClientRect(
        HWND  hWnd,
        RECT& Rect) const;

    static
    HWND
    GetMainWindow();

    static
    HWND
    FindWindow(
        Lon::Window::Type_e Type);

    static
    HWND
    GetChildWindow(
              HWND     hWnd,
        const wchar_t* pszTitle);
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_LONWINDOW_T_H

/////////////////////////////////////////////////////////////////////////////
