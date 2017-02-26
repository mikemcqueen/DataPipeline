///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TrScrollThumb_t.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TrScrollThumb_t.h"
#include "DdUtil.h"
#include "PipelineManager.h"
#include "LonWindow_t.h"
#include "SsTrades_t.h"

using namespace Lon;

extern Window::Type_e g_TradeBuilderVScroll;// = Window::TradeBuilderTheirTableVScroll;
extern Window::Type_e g_TradeBuilderHScroll;// = Window::TradeBuilderTheirTableHScroll;

///////////////////////////////////////////////////////////////////////////////

//#define EXTRALOG

///////////////////////////////////////////////////////////////////////////////

TrScrollThumb_t::
TrScrollThumb_t() :
    m_Horz(ScrollBar_t::Horizontal),
    m_Vert(ScrollBar_t::Vertical)
{
}

///////////////////////////////////////////////////////////////////////////////

TrScrollThumb_t::
~TrScrollThumb_t()
{
}

///////////////////////////////////////////////////////////////////////////////

bool
TrScrollThumb_t::
IsSupportedWindowType(
    Window::Type_e WindowType) const
{
    switch (WindowType)
    {
    case Window::PostedTradesWindow:
    case Window::TradeBuilderWindow:
        return true;
    default:
        break;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

Window::Type_e
TrScrollThumb_t::
GetScrollWindow(
    const Window::Type_e      WindowType,
    const ScrollBar_t::Type_e ScrollType) const
{
    Window::Type_e Scroll = Window::Unknown;
    switch (WindowType)
    {
    case Window::PostedTradesWindow:
        if (ScrollBar_t::Vertical == ScrollType)
            Scroll = Window::PostedTradesVScroll;
        break;
    case Window::TradeBuilderWindow:
        if (ScrollBar_t::Vertical == ScrollType)
            Scroll = g_TradeBuilderVScroll;
        else
            Scroll = g_TradeBuilderHScroll;
        break;
    default:
        ASSERT(false);
        break;
    }
    return Scroll;
}

///////////////////////////////////////////////////////////////////////////////
//
// TODO: move to TrLonWindow
//
HRESULT
TrScrollThumb_t::
MessageHandler(
    const DP::Message::Data_t* pData)
{
    if (!Validate(pData))
        return S_FALSE;

    // Discard screenshots if acquire window is not currently the top window
    Window::Type_e WindowType =
        static_cast<const SsTrades_t::AcquireData_t*>(pData)->WindowType;

    //TODO: get correct window types
    CSurface* pSurface = GetSurface(pData);
    InitScrollPosition(m_Vert, WindowType, pSurface);
    InitScrollPosition(m_Horz, WindowType, pSurface);
    m_WindowType = WindowType;
    PostData(pData->Id);
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

void
TrScrollThumb_t::
InitScrollPosition(
          ScrollBar_t&   ScrollBar,
          Window::Type_e WindowType,
    const CSurface*      pSurface)
{
    RECT rc;
    GetScrollRect(WindowType, ScrollBar.Type, rc);
    if (!IsRectEmpty(&rc))
        ScrollBar.Position = GetScrollPosition(ScrollBar.Type, pSurface, rc);
    else
        ScrollBar.Position = ScrollBar_t::UnknownPosition;
}

///////////////////////////////////////////////////////////////////////////////

void
TrScrollThumb_t::
GetScrollRect(
    Window::Type_e      WindowType,
    ScrollBar_t::Type_e ScrollType,
    RECT&               Rect) const
{
    SetRectEmpty(&Rect);
    Window::Type_e Scroll = GetScrollWindow(WindowType, ScrollType);
    if (Window::Unknown != Scroll)
    {
        LonWindow_t::GetWindowRect(Scroll,
                                   Rect,
                                   LonWindow_t::GetSsWindowType(WindowType));
    }
}

///////////////////////////////////////////////////////////////////////////////

bool
TrScrollThumb_t::
TranslateSurface(
    CSurface* pSurface,
    RECT&     rcSurface)
{
#if 0
    static int num = 0;
    if (0 == num++)
    {
        LogAlways(L"ScrollRect = { %d, %d, %d, %d }",
                  rcSurface.left, rcSurface.top, rcSurface.right, rcSurface.bottom);
        pSurface->WriteBMP(L"bmp\\scroll_thumb.bmp", rcSurface);
    }
#else
pSurface; rcSurface;
#endif

//    return Unknown != m_Position;
    return false;
}

///////////////////////////////////////////////////////////////////////////////

/*
LPARAM pagedown = MAKELONG(rc.right / 2, rc.bottom - ScrollbarThumbHeight - 1);
*/
static const int   ScrollbarThumbWidth   = 16;
static const int   ScrollbarThumbHeight  = 16;
static const DWORD ScrollPageColor       = 0x00444444;

///////////////////////////////////////////////////////////////////////////////

ScrollBar_t::ThumbPosition_e
TrScrollThumb_t::
GetScrollPosition(
          ScrollBar_t::Type_e ScrollType,
    const CSurface*           pSurface,
          RECT&               rcSurface) const
{
	DDSURFACEDESC2 ddsd;
    HRESULT hr;
	hr = pSurface->Lock(&ddsd);
	if (FAILED(hr))
        return ScrollBar_t::UnknownPosition;

    // TODO: CSurface::Unlock_t Unlock(pSurface);
    class Unlock_t
    {
        const CSurface* m_pSurface;
    public:
        Unlock_t(const CSurface* pSurface) : m_pSurface(pSurface) {}
        ~Unlock_t() { m_pSurface->Unlock(); }
    } Unlock(pSurface);

    size_t PixelCount;
    size_t PixelIncrement;
    if (ScrollBar_t::Vertical == ScrollType)
    {
        PixelCount = ScrollbarThumbWidth - 2;
        PixelIncrement = 1;
    }
    else
    {
        PixelCount = ScrollbarThumbHeight - 3;
        PixelIncrement = ddsd.lPitch / 4; // NOTE: 32bit
    }

    POINT pt;
    GetTopPoint(ScrollType, rcSurface, pt);
    size_t Count;
    Count = GetSameColorPixelCount(ddsd, pt, PixelCount, PixelIncrement);
    if (Count != PixelCount)
    {
#if defined(EXTRALOG)
        LogInfo(L"ScrollThumb(%d): TOP (%d,%d)", ScrollType, Count, PixelCount);
#endif
        return ScrollBar_t::Top;
    }
    GetBottomPoint(ScrollType, rcSurface, pt);
    Count = GetSameColorPixelCount(ddsd, pt, PixelCount, PixelIncrement);
    if (Count != PixelCount)
    {
#if defined(EXTRALOG)
        LogInfo(L"ScrollThumb(%d): BOTTOM (%d,%d)", ScrollType, Count, PixelCount);
#endif
        return ScrollBar_t::Bottom;
    }
    return ScrollBar_t::Middle;
}

///////////////////////////////////////////////////////////////////////////////

void
TrScrollThumb_t::
GetTopPoint(
          ScrollBar_t::Type_e ScrollType,
    const RECT&               Rect,
          POINT&              Point) const
{
    if (ScrollBar_t::Vertical == ScrollType)
    {
        Point.x = Rect.right - ScrollbarThumbWidth + 1;
        Point.y = Rect.top + ScrollbarThumbHeight + 1;
    }
    else
    {
        Point.x = Rect.left + ScrollbarThumbWidth + 1;
        Point.y = Rect.top + 1;
    }
}

///////////////////////////////////////////////////////////////////////////////

void
TrScrollThumb_t::
GetBottomPoint(
          ScrollBar_t::Type_e ScrollType,
    const RECT&               Rect,
          POINT&              Point) const
{
    if (ScrollBar_t::Vertical == ScrollType)
    {

//    POINT pagedown = { rcSurface.right - ScrollbarThumbWidth + 2,
//                       rcSurface.bottom - ScrollbarThumbHeight - 1 };
        Point.x = Rect.right - ScrollbarThumbWidth + 1;
        Point.y = Rect.bottom - ScrollbarThumbHeight - 1;
    }
    else
    {
        Point.x = Rect.right - ScrollbarThumbWidth - 1;
        Point.y = Rect.top + 1;
    }
}

///////////////////////////////////////////////////////////////////////////////

size_t
TrScrollThumb_t::
GetSameColorPixelCount(
    const DDSURFACEDESC2& ddsd,
          POINT           StartPoint,
          size_t          PixelCount,
          size_t          PixelIncrement) const
{
    size_t Count = 0;
    const DWORD *pBits = (DWORD*)GetBitsAt(&ddsd, StartPoint.x, StartPoint.y);
    const DWORD PixelData = (*pBits & 0x00ffffff);
    for (size_t Pixel = 0; Pixel < PixelCount; ++Pixel, ++Count)
    {
        // TODO: validate color range.  ScrollPageColor is wrong.
        // It can be 0x00686868 also.
        // if (ScrollPageColor != pixel)
        pBits += PixelIncrement;
        if (PixelData != (*pBits & 0x00ffffff))
            break;
    }
    return Count;
}

/////////////////////////////////////////////////////////////////////////////

void
TrScrollThumb_t::
PostData(
    DWORD AcquireId)
{
    AcquireId;
    LonWindow_t::EventThumbPosition_t
        HorzThumb(GetScrollWindow(m_WindowType, ScrollBar_t::Horizontal), m_Horz);
    LonWindow_t::EventThumbPosition_t
        VertThumb(GetScrollWindow(m_WindowType, ScrollBar_t::Vertical),   m_Vert);
    if ((Window::Unknown != HorzThumb.m_Data.WindowType) &&
        (Window::Unknown != VertThumb.m_Data.WindowType))
    {
        Event::Collection_t Event(DP::Stage::Acquire);
        Event.Add(&HorzThumb.m_Data);
        Event.Add(&VertThumb.m_Data);
        GetPipelineManager().SendEvent(Event.m_Data);
    }
    else if (Window::Unknown != HorzThumb.m_Data.WindowType)
    {
        GetPipelineManager().SendEvent(HorzThumb.m_Data);
    }
    else if (Window::Unknown != VertThumb.m_Data.WindowType)
    {
        GetPipelineManager().SendEvent(VertThumb.m_Data);
    }
    else
    {
        // No Scrollbars - shouldn't have made it this far?
        ASSERT(false);
    }
}

///////////////////////////////////////////////////////////////////////////////
