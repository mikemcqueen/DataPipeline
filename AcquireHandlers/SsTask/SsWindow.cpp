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

namespace SsWindow
{
namespace Acquire
{

/////////////////////////////////////////////////////////////////////////////

Handler_t::
Handler_t(
    CDisplay&           Display,
    Ui::Window::Base_t& Window)
:
    SsTask_t(Display, 1280, 1024),
    m_Window(Window),
    m_bClick(true)
{
}

///////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
EventHandler(
    DP::Event::Data_t& Data)
{
    HRESULT hr = SsTask_t::EventHandler(Data);
    if (S_FALSE != hr)
        return hr;

    ASSERT(DP::Message::Type::Event == Data.Type);

    using namespace Ui::Event;
    switch (Data.Id)
    {
#if 0
    case Id::Collection:
        {
            Lon::Event::Collection_t::Data_t&
                CollData = static_cast<Lon::Event::Collection_t::Data_t&>(Data);
            Lon::Event::Collection_t::EventVector_t::iterator
                it = CollData.Events.begin();
            bool bAnyScroll = false;
            for (; CollData.Events.end() != it; ++it)
            {
                // Only allow certain events in a collection. 
                switch ((*it)->Id)
                {
                case Id::ThumbPosition:
                    OnThumbPositionEvent(static_cast<LonWindow_t::EventThumbPosition_t::Data_t&>(**it));
                    break;
                case Id::Scroll:
                    bAnyScroll = true;
                    break;
                default:
                    return S_FALSE;
                }
            }
            // If no scroll events, return success
            if (!bAnyScroll)
                break;
        }
#endif
        // Scroll events in collection - fall through to AsyncEvent

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

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
AsyncEvent(
    const DP::Event::Data_t& Data)
{
    LogInfo(L"SsWindow::AsyncEvent(%d, %d)", Data.Id, Data.Size);

    if (!SetEventPending(true))
    {
        DP::Event::Data_t EventData;
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
/*
    case Id::Collection:
        {
            ClearEventData();
            const Lon::Event::Collection_t::Data_t& CollData =
                static_cast<const Lon::Event::Collection_t::Data_t&>(Data);
            Lon::Event::Collection_t::EventVector_t::const_iterator
                it = CollData.Events.begin();
            for (; CollData.Events.end() != it; ++it)
            {
                switch ((*it)->Id)
                {
                case Id::Scroll:
                    AddEventData(*it);
                    break;
                default:
                    break;
                }
            }
        }
        break;
*/
    default:
        ASSERT(false);
        return;
    }
    SetAsyncEvent();
}

/////////////////////////////////////////////////////////////////////////////

#if 0
void
Handler_t::
OnThumbPositionEvent(
    LonWindow_t::EventThumbPosition_t::Data_t& Data)
{
    if (0 == (Data.Flags & DP::Event::Flags::Query))
    {
        SetThumbPosition(Data.WindowType, Data.ScrollBar);
    }
    else
    {
        Data.ScrollBar.Position = GetThumbPosition(Data.WindowType, Data.ScrollBar.Type);
    }
}

/////////////////////////////////////////////////////////////////////////////

Lon::ScrollBar_t::ThumbPosition_e
Handler_t::
GetThumbPosition(
    Lon::Window::Type_e     WindowType,
    Lon::ScrollBar_t::Type_e ScrollType) const
{
    CLock lock(m_csThumbs);
    if ((Lon::ScrollBar_t::Vertical == ScrollType) &&
        (m_VertType == WindowType))
    {
        return m_VertBar.Position;
    }
    else if ((Lon::ScrollBar_t::Horizontal == ScrollType) &&
             (m_HorzType == WindowType))
    {
        return m_HorzBar.Position;
    }
    return Lon::ScrollBar_t::UnknownPosition;
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
SetThumbPosition(
          Lon::Window::Type_e    WindowType,
    const Lon::ScrollBar_t& ScrollBar)
{
    CLock lock(m_csThumbs);
    if (Lon::ScrollBar_t::Vertical == ScrollBar.Type)
    {
        m_VertType = WindowType;
        m_VertBar  = ScrollBar;
    }
    else
    {
        m_HorzType = WindowType;
        m_HorzBar  = ScrollBar;
    }
}
#endif

/////////////////////////////////////////////////////////////////////////////

HWND
Handler_t::
GetSsWindowRect(
    RECT& rcBounds) const
{
    return m_Window.GetSsWindowRect(rcBounds);
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
ThreadProcessEvent()
{
    if (!IsEventPending())
    {
        LogError(L"SsWindow::ThreadProcessEvent(): No event pending.");
        return;
    }
    using namespace Ui::Event;
    for (size_t Event = 0; Event < GetEventCount(); ++Event)
    {
        const DP::Event::Data_t& eventData = PeekEvent(Event);
        switch (eventData.Id)
        {
/*
        case Id::Scroll:
            {
                LonWindow_t::EventScroll_t::Data_t ScrollData;
                GetEventData(Event, ScrollData, sizeof(ScrollData));
                ScrollWindow(ScrollData);
            }
            break;
*/
        case Id::ClickPoint:
        case Id::ClickWidget:
            {
                Click::Data_t ClickData;
                GetEventData(Event, ClickData, sizeof(ClickData));
                if (Id::ClickPoint == ClickData.Id)
                {
                    ClickPoint(ClickData, ClickData.Destination.Point);
                }
                else
                {
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

/////////////////////////////////////////////////////////////////////////////

#if 0
void
Handler_t::
ScrollWindow(
    const LonWindow_t::EventScroll_t::Data_t& Data)
{
    LogInfo(L"SsWindow::ScrollWindow(%d, %d)",
            Data.ScrollType, Data.ScrollCount);

    if (Data.Size < sizeof(LonWindow_t::EventScroll_t::Data_t))
    {
        LogError(L"Invalid scroll data size (%d,%d)",
                 Data.Size, sizeof(LonWindow_t::EventScroll_t::Data_t));
        return;
    }
    // TODO: Scrollbar class/namespace
    static const int ScrollbarThumbHeight = 16;
    static const int ScrollbarThumbWidth  = 16;

    const LonWindow_t::EventScroll_t::Data_t&
        LonData = (LonWindow_t::EventScroll_t::Data_t&)Data;
    HWND hScroll = LonWindow_t::GetWindow(LonData.WindowType);
    if ((NULL == hScroll) || !IsWindowVisible(hScroll))
    {
        LogError(L"SsTrades::ScrollWindow(): Scroll hWnd is NULL or invisible (%d,%d)",
                 hScroll, LonData.WindowType);
        return;
    }
    RECT rc;
    ::GetClientRect(hScroll, &rc);

    LonWindow_t::EventScroll_t::Type_e Type = Data.ScrollType;
    bool bPageFirst = false;

    Lon::ScrollBar_t::ThumbPosition_e
        ThumbPos = GetThumbPosition(LonData.WindowType, LonData.ScrollBar);
    if (Lon::ScrollBar_t::UnknownPosition != ThumbPos)
    {
        switch (Type)
        {
        case LonWindow_t::EventScroll_t::PageDown:
            if (Lon::ScrollBar_t::Bottom == ThumbPos)
                Type = LonWindow_t::EventScroll_t::LineDown;
            else if (Lon::ScrollBar_t::Top == ThumbPos)
                bPageFirst = true;
            break;
        case LonWindow_t::EventScroll_t::PageUp:
            if (Lon::ScrollBar_t::Top == ThumbPos)
                Type = LonWindow_t::EventScroll_t::LineUp;
            break;
        default:
            break;
        }
    }

    #define Msg SendMessage
    #define MAKEVLINEUP(rc) MAKELONG(((rc).right / 2), (ScrollbarThumbHeight / 2))

    const HWND Window = hScroll;
    switch (Type)
    {
    case LonWindow_t::EventScroll_t::PageDown:
        {
            LPARAM pagedown;
            LPARAM lineup;
            if (Lon::ScrollBar_t::Vertical == Data.ScrollBar)
            {
                pagedown = MAKELONG(rc.right / 2, rc.bottom - ScrollbarThumbHeight - 1);
                lineup = MAKEVLINEUP(rc);
            }
            else
            {
                pagedown = MAKELONG(rc.right - ScrollbarThumbWidth - 1, rc.bottom / 2);
                lineup = MAKELONG(ScrollbarThumbWidth / 2, rc.bottom / 2);
            }
            if (bPageFirst)
                DoClick(Window, pagedown, Data.ScrollCount);
            DoClick(Window, lineup, Data.ScrollPageLines);
            if (!bPageFirst)
                DoClick(Window, pagedown, Data.ScrollCount);
        }
        break;
    case LonWindow_t::EventScroll_t::LineDown:
        {
            LPARAM linedown;
            if (Lon::ScrollBar_t::Vertical == Data.ScrollBar)
                linedown = MAKELONG(rc.right / 2, rc.bottom - (ScrollbarThumbHeight / 2));
            else
                linedown = MAKELONG(rc.right - (ScrollbarThumbWidth / 2), rc.bottom / 2);
            DoClick(Window, linedown, Data.ScrollCount);
        }
        break;
    case LonWindow_t::EventScroll_t::PageUp:
        {
            if (Lon::ScrollBar_t::Vertical == Data.ScrollBar)
            {
                LPARAM pageup = MAKELONG(rc.right / 2, ScrollbarThumbHeight + 1);
                DoClick(Window, pageup, Data.ScrollCount);
            }
            else
                ASSERT(false);
        }
        break;
    case LonWindow_t::EventScroll_t::LineUp:
        {
            LPARAM lineup;
            if (Lon::ScrollBar_t::Vertical == Data.ScrollBar)
                lineup = MAKEVLINEUP(rc);
            else
                lineup = MAKELONG(ScrollbarThumbWidth / 2, rc.bottom / 2);
            DoClick(Window, lineup, Data.ScrollCount);
        }
        break;
    default:
        ASSERT(false);
        break;
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////////

void
Handler_t::
ClickPoint(
    const Ui::Event::Click::Data_t& Data,
          POINT                     Point)
{
#if 0
    Point; Data;
    LogError(L"ClickPoint() not implemented");
    throw std::logic_error("ClickPoint() not implemented");
#else
    LogInfo(L"SsWindow::ClickPoint() (%d,%d)", Point.x, Point.y);
    if (m_bClick && ValidateEventData(Data, sizeof(Data)))
    {
        HWND hWnd = ValidateWindow(Data.WindowId);
        if (NULL != hWnd)
        {
            size_t Count = Data.Count;
            using namespace Ui::Event;
            while (0 < Count--)
            {
                switch (Data.Method)
                {
#if 0
                case Input::Method::SendMessage:
                    Ui::Window::ClickSendMessage(hWnd, MAKELONG(Point.x, Point.y),
                        Data.Button, Data.bDoubleClick);
                    break;
#endif
                case Input::Method::SendInput:
                    Ui::Input_t::Click(hWnd, Point);
                    break;
                default:
                    LogError(L"SsWindow::ClickPoint() invalid method (%d)", Data.Method);
                    throw std::invalid_argument("SsWindow::ClickPoint() method");
                }
            }
        }
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////////

void
Handler_t::
ClickWidget(
    const Ui::Event::Click::Data_t& Data)
{
    const Ui::WidgetId_t WidgetId = Data.Destination.WidgetId;
    LogInfo(L"SsWindow::ClickWidget(%d)", WidgetId);
    if (m_bClick && ValidateEventData(Data, sizeof(Data)))
    {    
        HWND hWnd = ValidateWindow(Data.WindowId);
        if (NULL != hWnd)
        {
            m_Window.GetWindow(Data.WindowId).ClickWidget(WidgetId, true);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
ValidateEventData(
    const DP::Event::Data_t& Data,
          size_t             Size)
{
    // this function is probably unnecessary as i do exact size 
    // validation in SsTask_t::GetEventData()
    if (Data.Size < Size)
    {
        LogError(L"SsWindow::ValidateEventData(%d) Invalid data size (%d) should be (%d)",
                 Data.Id, Size, Size);
        throw std::logic_error("SsWindow::ValidateEventData() Invalid data size");
    }
    else if (Data.Size > Size)
    {
       LogError(L"SsWindow::ValidateEventData(%d) Data.size (%d) > Size (%d)",
                 Data.Id, Size, Size);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////

HWND
Handler_t::
ValidateWindow(
    Ui::WindowId_t WindowId)
{
    // TODO: Shouldn't we use GetMainWindow() for the GetForegroundWindow() check?

    HWND hWnd = m_Window.GetHwnd(WindowId);
    if ((NULL == hWnd) || !IsWindowVisible(hWnd) || (hWnd != ::GetForegroundWindow()))
    {
        LogError(L"SsWindow::ValidateWindow() failed for hWnd (%08x)", hWnd);
        return NULL;
    }
    const Ui::Window::Handle_t Top = m_Window.GetTopWindow();
    // TODO : Window::IsParent
    if ((Top.hWnd != hWnd) && !util::IsParent(Top.hWnd, hWnd))
    {
        LogError(L"SsWindow::ValidateWindow(): WindowId (%d) is not top window"
                 L" or a child of top window (%d)", WindowId, Top.WindowId);
        return NULL;
    }
    return hWnd;
}

///////////////////////////////////////////////////////////////////////////////////

void
Handler_t::
SendChars(
    const Ui::Event::SendChars::Data_t& Data)
{
    LogInfo(L"SsWindow::SendChars(%ls)", Data.Chars);
    HWND hWnd = ValidateWindow(Data.WindowId);
    if (NULL == hWnd)
    {
        return;
    }
    const Ui::WidgetId_t WidgetId = Data.Destination.WidgetId;
    if (Ui::Widget::Id::Unknown != WidgetId)
    {
        Rect_t Rect;
        if (!m_Window.GetWindow(Data.WindowId).GetWidgetRect(WidgetId, Rect))
        {
            LogError(L"SsWindow::SendChars(): GetWidgetRect(%d) failed", WidgetId);
            return;
        }
        Ui::Input_t::Click(hWnd, Rect.Center());
    }
    Ui::Input_t keys;
    keys.SendChars(Data.Chars);
}

////////////////////////////////////////////////////////////////////////////////

void
Handler_t::
PostData(
    HWND               hWnd,
    SurfacePoolItem_t* pPoolItem)
{
    LogInfo(L"SsWindow::PostData()");
    using namespace Ui::Window;
    Handle_t Top = m_Window.GetTopWindow();
    Rect_t SurfaceRect;
    // TODO: if (!m_Window.HasChildHwnds())
    if (Id::MainWindow == Top.WindowId)
    {
        // The top window is the main app window. This means we're actually
        // a single-window app with no child windows, and need to infer the
        // actual window id by looking at surface bits.

        // TODO: Probably should support a "windowed" flag in Window::Base_t
        // and go off that.
        // TrWindowType translate handler will look for messages with unknown
        // window type and determine actual window type.
        Top.WindowId = Id::Unknown;
    }
    else
    {
        // Handle the legacy (and LoN) case (windowed app).
        //
        // We have the hWnd, and from that we can infer what the window type
        // was on top before the screenshot.  Maybe it changed on us between
        // then and now, however.  We just take the current top window type
        // and if the supplied hWnd is a child we consider it good enough.
        // TODO: This could be better.
        // TODO: Ui::Window::IsParent()
        if ((Top.hWnd != hWnd) && !util::IsParent(Top.hWnd, hWnd))
        {
            return;
        }
        if (Id::Unknown == Top.WindowId)
        {
            LogWarning(L"SsWindow::PostData() TopWindowId is unknown.");
            return;
        }
    }
    void *pBuffer = GetPipelineManager().Alloc(sizeof(Data_t));
    if (NULL == pBuffer)
    {
        LogError(L"SsWindow::PostData(): Alloc callback data failed.");
    }
    else
    {
        pPoolItem->addref(); // Haxington Heights
        Data_t* pData = new (pBuffer)
            Data_t(
                GetClass().c_str(),
                Top.WindowId,
                SurfaceRect,
                pPoolItem);
        HRESULT hr = GetPipelineManager().Callback(pData);
        if (FAILED(hr))
        {
            LogError(L"SsWindow::PostData(): PM.Callback() failed (%08x)", hr);
        }
    }
}		

///////////////////////////////////////////////////////////////////////////////////

} // Acquire
} // SsWindow

///////////////////////////////////////////////////////////////////////////////////

