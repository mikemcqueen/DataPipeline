/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// SsTrades_t.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SsTrades_t.h"
#include "TrScrollThumb_t.h"
#include "LonWindow_t.h"
#include "PipelineManager.h"
#include "Macros.h"

/////////////////////////////////////////////////////////////////////////////

SsTrades_t::
SsTrades_t(
    CDisplay&          Display)
:
    Base_t(Display, 1280, 1024),
    m_bClick(true),
    m_VertBar(Lon::ScrollBar_t::Vertical),
    m_HorzBar(Lon::ScrollBar_t::Horizontal)
{
}

///////////////////////////////////////////////////////////////////////////////////

#pragma warning(disable:4063)

HRESULT
SsTrades_t::
EventHandler(
    DP::Event::Data_t& Data)
{
    HRESULT hr = Base_t::EventHandler(Data);
    if (S_FALSE != hr)
        return hr;

    ASSERT(DP::Message::Type::Event == Data.Type);

    using namespace Lon::Event;
    switch (Data.Id)
    {
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
        // Scroll events in collection - fall through to AsyncEvent
        [[fallthrough]];
    case Id::Scroll:
    case Id::Click:
    case Id::SendChars:
//    case DP::ScreenShot:
        AsyncEvent(Data);
        break;

    case Id::ThumbPosition:
        OnThumbPositionEvent(static_cast<LonWindow_t::EventThumbPosition_t::Data_t&>(Data));
        break;

    default:
        return S_FALSE;
    }
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

void
SsTrades_t::
AsyncEvent(
    const DP::Event::Data_t& Data)
{
    LogInfo(L"SsTrades_t::AsyncEvent(%d, %d)", Data.Id, Data.Size);

    if (!SetEventPending(true))
    {
        DP::Event::Data_t EventData;
        GetEventData(0, EventData, sizeof(EventData));

//        const DP::Event::Data_t& EventData = GetEventData(0);
//            *reinterpret_cast<const DP::Event::Data_t&>(GetEventData(0));
        LogError(L"SsTrades_t::AsyncEvent: Nested pending event"
                 L"Old = (%d,%d), New = (%d,%d)",
                 EventData.Id, EventData.Size, Data.Id, Data.Size);
        return;
    }
    SuspendAndFlush();

    using namespace Lon::Event;
    switch (Data.Id)
    {
//    case DP::ScreenShot:
    case Id::Scroll:
    case Id::Click:
    case Id::SendChars:
        SetEventData(Data);
        break;
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
    default:
        ASSERT(false);
        return;
    }
    SetAsyncEvent();
}

/////////////////////////////////////////////////////////////////////////////

void
SsTrades_t::
OnThumbPositionEvent(
    LonWindow_t::EventThumbPosition_t::Data_t& Data)
{
	/* TODO: NOTE: commented out so it will compile
	if (0 == (Data.Flags & DP::Event::Flags::Query))
    {
        SetThumbPosition(Data.WindowType, Data.ScrollBar);
    }
    else
		*/
    {
        Data.ScrollBar.Position = GetThumbPosition(Data.WindowType, Data.ScrollBar.Type);
    }
}

/////////////////////////////////////////////////////////////////////////////

Lon::ScrollBar_t::ThumbPosition_e
SsTrades_t::
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
SsTrades_t::
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

/////////////////////////////////////////////////////////////////////////////

HWND
SsTrades_t::
GetSsWindowRect(
    RECT& rcBounds) const
{
    return LonWindow_t::GetSsWindowRect(rcBounds);
}

/////////////////////////////////////////////////////////////////////////////

void
SsTrades_t::
ThreadProcessEvent()
{
    if (!IsEventPending())
    {
        LogError(L"SsTrades_t::ThreadProcessEvent(): No event pending.");
        return;
    }
    using namespace Lon::Event;
    for (size_t Event = 0; Event < GetEventCount(); ++Event)
    {
//        const DP::Event::Data_t& Data = GetEventData(Event);
//        const char* Data = GetEventData(Event);
// TODO: HACK extra copy here kinda bogus
        DP::Event::Data_t Data;
        GetEventData(Event, Data, sizeof(Data));
        switch (Data.Id)
        {
        case Id::Scroll:
            {
                LonWindow_t::EventScroll_t::Data_t ScrollData;
                GetEventData(Event, ScrollData, sizeof(ScrollData));
                ScrollWindow(ScrollData);
            }
            break;
        case Id::Click:
            {
                LonWindow_t::EventClick_t::Data_t ClickData;
                GetEventData(Event, ClickData, sizeof(ClickData));
                ClickWindow(ClickData);
            }
            break;
        case Id::SendChars:
            {
                LonWindow_t::EventSendChars_t::Data_t SendCharsData;
                GetEventData(Event, SendCharsData, sizeof(SendCharsData));
                SendChars(SendCharsData);
            }
            break;
/*
        case DP::ScreenShot:
            ASSERT(false);
            Shutter();
            break;
*/
        default:
            LogError(L"SsTrades_t: Unknown event (%d)", Data.Id);
            ASSERT(false);
            break;
        }
    }
    SetEventPending(false);
    Resume();
}

/////////////////////////////////////////////////////////////////////////////

void
SsTrades_t::
ScrollWindow(
    const LonWindow_t::EventScroll_t::Data_t& Data)
{
    LogInfo(L"SsTrades_t::ScrollWindow(%d, %d)",
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

///////////////////////////////////////////////////////////////////////////////////

void
SsTrades_t::
ClickWindow(
    const LonWindow_t::EventClick_t::Data_t& Data)
{
    LogInfo(L"SsTrades_t::ClickWindow(%d,%d)", Data.Point.x, Data.Point.y);

    if (Data.Size < sizeof(LonWindow_t::EventClick_t::Data_t))
    {
        LogError(L"Invalid click data size (%d,%d)",
                 Data.Size, sizeof(LonWindow_t::EventClick_t::Data_t));
        return;
    }
    const LonWindow_t::EventClick_t::Data_t&
        LonData = static_cast<const LonWindow_t::EventClick_t::Data_t&>(Data);
    DoClick(LonWindow_t::GetWindow(LonData.WindowType),
            MAKELONG(Data.Point.x, Data.Point.y),
            Data.Count,
            Data.Button,
            Data.bDoubleClick);
}

///////////////////////////////////////////////////////////////////////////////////

void
SsTrades_t::
DoClick(
    HWND   hWnd,
    LPARAM lParam,
    size_t Count,
    LonWindow_t::EventClick_t::Button_e Button,
    bool   bDouble)
{
    if (!m_bClick)
        return;
    if ((NULL == hWnd) || !IsWindowVisible(hWnd))
    {
        LogError(L"SsTrades::DoClick(): Handle is NULL or window is invisible (%d)", hWnd);
        return;
    }
    const LonWindow_t::Handle_t Top = LonWindow_t::GetTopWindow();
    if ((Top.hWnd != hWnd) && !util::IsParent(Top.hWnd, hWnd))
    {
        LogError(L"SsTrades::DoClick(): Click window is not top window (%d)", Top.Type);
        return;
    }
    while (0 < Count--)
    {
        LonWindow_t::Click(hWnd, lParam, Button, bDouble);
    }
}

////////////////////////////////////////////////////////////////////////////////

void
SsTrades_t::
SendChars(
    const DP::Event::Data_t& Data)
{
    if (Data.Size < sizeof(LonWindow_t::EventSendChars_t::Data_t))
    {
        LogError(L"Invalid SendChars data size (%d,%d)",
                 Data.Size, sizeof(LonWindow_t::EventSendChars_t::Data_t));
        return;
    }
    const LonWindow_t::EventSendChars_t::Data_t&
        LonData = static_cast<const LonWindow_t::EventSendChars_t::Data_t&>(Data);

    HWND hWnd = LonWindow_t::GetWindow(LonData.WindowType);
    if ((NULL == hWnd) || !IsWindowVisible(hWnd))
    {
        LogError(L"SsTrades::SendChars(): HWND is NULL or window is invisible (%d)", hWnd);
        return;
    }
    const LonWindow_t::Handle_t Top = LonWindow_t::GetTopWindow();
    if ((Top.hWnd != hWnd) && !util::IsParent(Top.hWnd, hWnd))
    {
        LogError(L"SsTrades::SendChars(): Click window is not top window (%d)", Top.Type);
        return;
    }
    LogInfo(L"SsTrades_t::SendChars(%ls)", LonData.Chars);
    LonWindow_t::SendCharsDirect(hWnd, LonData.Chars);
}

////////////////////////////////////////////////////////////////////////////////

void
SsTrades_t::
PostData(
    HWND               hWnd,
    SurfacePoolItem_t* pPoolItem)
{
    //
    // We have the hWnd, and from that we can infer what the window type
    // was on top before the screenshot.  Maybe it changed on us between
    // then and now, however.  We just take the current top window type
    // and if the supplied hWnd is a child we consider it good enough.
    // TODO: This could be better.
    //
    const LonWindow_t::Handle_t Handle = LonWindow_t::GetTopWindow();
    if ((Handle.hWnd != hWnd) && !util::IsParent(Handle.hWnd, hWnd))
        return;
//    if (!IsWindowTypeSupported(Handle.Type))
//        return;


#define PLACEMENT_NEW 
#ifndef PLACEMENT_NEW
    AcquireData_t* pData = reinterpret_cast<AcquireData_t*>
#else
    void *pData =
#endif
        (GetPipelineManager().Alloc(sizeof(AcquireData_t)));

    if (NULL == pData)
    {
        LogError(L"SsTrades_t::PostData(): Alloc callback data failed.");
    }
    else
    {
        // TODO: HACK? 
        pPoolItem->addref();
#ifndef PLACEMENT_NEW
        wcscpy_s(pData->Class, _countof(pData->Class), GetClass().c_str());
        pData->Stage      = DP::Stage::Acquire;
        pData->Type       = DP::Message::Message;
        pData->Id         = Lon::Message::ScreenShot;
        pData->WindowType = Handle.Type;
        pData->pPoolItem  = pPoolItem;
        HRESULT hr = GetPipelineManager().Callback(pData);
#else
        AcquireData_t* pSsData = new (pData)
            AcquireData_t(
                GetClass().c_str(),
                Handle.Type,
                pPoolItem);
        HRESULT hr = GetPipelineManager().Callback(pSsData);
#endif
        if (FAILED(hr))
            LogError(L"SsTrades_t::PostData(): PM.Callback() failed.");
    }
}		

///////////////////////////////////////////////////////////////////////////////////
