////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// UiWindow.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UiWindow.h"
#include "PipelineManager.h"
#include "UiEvent.h"
#include "UiInput.h"
#include "DdUtil.h"
#include "Log.h"
#include "Macros.h"

namespace Ui
{
namespace Window
{

////////////////////////////////////////////////////////////////////////////////

Base_t::
Base_t(
    WindowId_t     WindowId,
    const wchar_t* pClassName,
    const wchar_t* pWindowName /* = nullptr */,
    Flag_t         Flags /* = 0 */)
    :
    m_WindowId(WindowId),
    m_ParentWindow(*this),         // a bit weird; parent of mainwindow is self
    m_hMainWindow(nullptr),
    m_strClassName((nullptr == pClassName) ? L"" : pClassName),
    m_strWindowName((nullptr == pWindowName) ? L"" : pWindowName),
    m_Flags(Flags),
    m_VertScrollPos(Scroll::Position::Unknown),
    m_HorzScrollPos(Scroll::Position::Unknown),
    m_pWidgets(nullptr),
    m_WidgetCount(0)
{
    LogError(L"FIXME: MainWindow hWnd initialization - move out of UiWindow.cpp");
    /*
    if ((nullptr != pClassName) || (nullptr != pWindowName))
    {
        HWND hWnd = ::FindWindow(pClassName, pWindowName);
        if (nullptr == hWnd)
        {
            LogError(L"Window not found: ClassName(%ls) WindowName(%ls)",
                     pClassName, pWindowName);
            throw invalid_argument("Ui::Window::Base_t()");
        }
        m_hMainWindow = hWnd;
    }
    */
}

////////////////////////////////////////////////////////////////////////////////
//
// Constructor for fake child 'windows' that have no associated hWnd.
//

Base_t::
Base_t(
    WindowId_t            WindowId,
    const Base_t&         ParentWindow,
    const wchar_t*        pWindowName,
    Flag_t                Flags       /*= 0*/,
    const Widget::Data_t* pWidgets    /*= nullptr*/,
    size_t                WidgetCount /*= 0*/)
:
    m_WindowId(WindowId),
    m_ParentWindow(ParentWindow),
    m_hMainWindow(ParentWindow.GetHwnd()),
    m_strWindowName((nullptr == pWindowName) ? L"Undefined" : pWindowName),
    m_Flags(Flags),
    m_pWidgets(pWidgets),
    m_WidgetCount(WidgetCount)
{
}

////////////////////////////////////////////////////////////////////////////////

Base_t::
~Base_t()
{
}

////////////////////////////////////////////////////////////////////////////////

HWND
Base_t::
GetSsWindowRect(
    RECT& Rect) const
{
    if (nullptr != m_hMainWindow)
    {
        if (::IsWindow(m_hMainWindow))
        {
            // !::IsWindowVisible(m_hMainWindow) seems overkill
            if (::GetForegroundWindow() != m_hMainWindow)
            {
                // don't screenshot if we're not foreground
                return nullptr;
            }
            ::GetClientRect(m_hMainWindow, &Rect);
        }
        else
        {
            // TODO: make m_hMainWindow mutable?
            const_cast<Base_t*>(this)->m_hMainWindow = nullptr;
        }
    }
    return m_hMainWindow;
}

////////////////////////////////////////////////////////////////////////////////

Base_t&
Base_t::
GetWindow(
    WindowId_t /*WindowId*/) const
{
    throw logic_error("Ui::Window_t::GetWindow() not implemented");
}

////////////////////////////////////////////////////////////////////////////////

WindowId_t
Base_t::
GetWindowId(
    const CSurface& /*Surface*/,
    const POINT*    /*pptHint*/) const
{
    return m_WindowId;
}

////////////////////////////////////////////////////////////////////////////////

bool
Base_t::
GetWidgetRect(
    Ui::WidgetId_t /*WidgetId*/,
          Rect_t&  /*WidgetRect*/) const
{
    // NOTE: we could support this, by using "windowrect" as default
    // but i don't need that functionality, and i want to be warned
    // when i haven't overridden this in a derived class, so it stays
    // like this.
    // TODO: should be a flag to select behavior
    throw logic_error("Ui::Window_t::GetWidgetRect() not implemented");
}

////////////////////////////////////////////////////////////////////////////////

bool
Base_t::
GetWidgetRect(
          Ui::WidgetId_t  WidgetId,
    const Rect_t&         RelativeRect,
          Rect_t&         WidgetRect,
    const Widget::Data_t* pWidgets /*= nullptr*/,
          size_t          widgetCount /*= 0*/) const
{
    if (nullptr == pWidgets)
    {
        pWidgets = m_pWidgets;
        widgetCount = m_WidgetCount;
    }
    if (nullptr != pWidgets)
    {
        for (size_t Widget = 0; Widget < widgetCount; ++Widget)
        {
            if (pWidgets[Widget].WidgetId == WidgetId)
            {
                RelativeRect_t Rect(pWidgets[Widget].RectData);
                WidgetRect = Rect.GetRelativeRect(RelativeRect);
                return true;
            }
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

bool
Base_t::
IsLocatedOn(
    const CSurface& surface,
          Flag_t    flags,
          POINT*    pptOrigin /*= nullptr*/) const
{
    const CSurface* pOriginSurface = GetOriginSurface();
    if (nullptr != pOriginSurface)
    {
        using namespace Ui::Window;
        if (flags.Test(Locate::CompareLastOrigin) &&
            CompareLastOrigin(surface, *pOriginSurface, pptOrigin))
        {
            return true;
        }
        if (flags.Test(Locate::Search) &&
            OriginSearch(surface, *pOriginSurface, pptOrigin))
        {
            return true;
        }
    }
    return false;
}
 
////////////////////////////////////////////////////////////////////////////////

void
Base_t::
GetOriginSearchRect(
    const CSurface& surface,
          Rect_t&   rect) const
{
    LogWarning(L"%s::GetOriginSearchRect() Using entire BltRect", GetWindowName());
    rect = surface.GetBltRect();
}

////////////////////////////////////////////////////////////////////////////////

bool
Base_t::
CompareLastOrigin(
    const CSurface& surface,
    const CSurface& image,
          POINT*    pptOrigin) const
{
    const POINT& pt = GetLastOrigin();
    if ((0 < pt.x) || (0 < pt.y))
    {
        if (surface.Compare(pt.x, pt.y, image))
        {
            if (nullptr != pptOrigin)
            {
                *pptOrigin = pt;
            }
            LogInfo(L"%s::CompareLastOrigin() Match", GetWindowName());
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

bool
Base_t::
OriginSearch(
    const CSurface& surface,
    const CSurface& image,
          POINT*    pptOrigin) const
{
    Rect_t searchRect;
    GetOriginSearchRect(surface, searchRect);
    // search for the supplied origin bitmap on the supplied surface
    POINT ptOrigin;
    if (surface.FindSurfaceInRect(image, searchRect, ptOrigin, nullptr))
    {
        LogInfo(L"%s::OriginSearch(): Found @ (%d, %d)", GetWindowName(),
                ptOrigin.x, ptOrigin.y);
        if (nullptr != pptOrigin)
        {
            *pptOrigin = ptOrigin;
        }
        const_cast<Window_t*>(this)->SetLastOrigin(ptOrigin);
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

bool
Base_t::
ClickWidget(
    WidgetId_t    WidgetId,
          bool    bDirect /*= false*/,
    const Rect_t* pRect /*= nullptr*/) const
{
    if (bDirect)
    {
        Rect_t Rect;
        if (nullptr == pRect)
        {
            if (!GetWidgetRect(WidgetId, Rect))
            {
                LogError(L"%ls::ClickWidget(direct): GetWidgetRect(%d) failed",
                         GetWindowName(), WidgetId);
                return false;
            }
            pRect = &Rect;
        }
        Ui::Input_t::Click(GetHwnd(), pRect->Center());
    }
    else
    {
        Ui::Event::Click::Data_t Click(GetWindowId(), WidgetId);
        GetPipelineManager().SendEvent(Click);
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
Base_t::
ClearWidgetText(
    WidgetId_t widgetId,
    size_t     count) const
{
    Rect_t Rect;
    if (!GetWidgetRect(widgetId, Rect))
    {
        LogError(L"%ls::ClearWidgetText(): GetWidgetRect(%d) failed",
                 GetWindowName(), widgetId);
        return false;
    }
    // click right side of rectangle
    Rect_t clickRect(Rect);
    clickRect.left = clickRect.right - 1;
    Input_t::Click(GetHwnd(), clickRect.Center());
    while (0 < count--)
    {
        Input_t::SendChar(VK_BACK);
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
Base_t::
SetWidgetText(
    WidgetId_t     widgetId,
    const wstring& text,
          bool     bDirect) const
{
    if (bDirect)
    {
        Rect_t Rect;
        if (!GetWidgetRect(widgetId, Rect))
        {
            LogError(L"%ls::SetWidgetText(direct): GetWidgetRect(%d) failed",
                     GetWindowName(), widgetId);
            return false;
        }
        Ui::Input_t Chars;
        Chars.SendChars(text.c_str());
    }
    else
    {
        Ui::Event::SendChars::Data_t sendChars(text.c_str(), GetWindowId(), widgetId);
        GetPipelineManager().SendEvent(sendChars);
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

#if 0
const Widget::Data_t&
Base_t::
GetWidgetData(
    WidgetId_t widgetId) const
{
    if (nullptr != m_pWidgets)
    {
        for (size_t Widget = 0; Widget < m_WidgetCount; ++Widget)
        {
            if (m_pWidgets[Widget].WidgetId == widgetId)
            {
                return m_pWidgets[Widget];
            }
        }
    }
    throw invalid_argument("Ui::Window_t::GetWidgetData()");
}
#endif

////////////////////////////////////////////////////////////////////////////////

bool
Base_t::
Scroll(
    Scroll::Direction_t Direction) const
{
    using namespace Scroll;
    Ui::WidgetId_t WidgetId = Ui::Widget::Id::Unknown;
    switch (Direction)
    {
    case Direction::Up: 
        {
            const Position_t VertPos = GetScrollPosition(Bar::Vertical);
            if ((Position::Top == VertPos) || (Position::Unknown == VertPos))
            {
                return false;
            }
            WidgetId = Ui::Widget::Id::VScrollUp;
        }
        break;
    case Direction::Down:
        {
            const Position_t VertPos = GetScrollPosition(Bar::Vertical);
            if ((Position::Bottom == VertPos) || (Position::Unknown == VertPos))
            {
                return false;
            }
            WidgetId = Ui::Widget::Id::VScrollDown;
        }
        break;
    case Direction::Left:
    case Direction::Right:
        return false;
    default:
        break;
    }
    if (Ui::Widget::Id::Unknown == WidgetId)
    {
        throw logic_error("UiWindow::Scroll(): Invalid widget id");
    }
    return ClickWidget(WidgetId);
}

////////////////////////////////////////////////////////////////////////////////

bool
Window_t::
ValidateBorders(
    const CSurface& Surface,
    const Rect_t&   Rect,
    const SIZE&     BorderSize,
    const COLORREF  LowColor,
    const COLORREF  HighColor) const
{
    Rect_t BorderRect = Rect;
    BorderRect.bottom = BorderRect.top + BorderSize.cy;
    if (!ValidateBorder(Surface, BorderRect, L"Top", LowColor, HighColor))
    {
        return false;
    }
    OffsetRect(&BorderRect, 0, Rect.Height() - BorderSize.cy);
    if (!ValidateBorder(Surface, BorderRect, L"Bottom", LowColor, HighColor))
    {
        return false;
    }

    BorderRect = Rect;
    BorderRect.right = BorderRect.left + BorderSize.cx,
    BorderRect.top += BorderSize.cy;
    BorderRect.bottom -= BorderSize.cy;
    if (!ValidateBorder(Surface, BorderRect, L"Left", LowColor, HighColor))
    {
        return false;
    }
    OffsetRect(&BorderRect, Rect.Width() - BorderSize.cx, 0);
    if (!ValidateBorder(Surface, BorderRect, L"Rignt", LowColor, HighColor))
    {
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
Window_t::
ValidateBorder(
    const CSurface& Surface,
    const Rect_t&   BorderRect,
    const wchar_t*  pBorderName,
    const COLORREF  LowColor,
    const COLORREF  HighColor) const
{
    if (!Surface.CompareColorRange(BorderRect, LowColor, HighColor))
    {
        LogWarning(L"%ls::ValidateBorder(): %ls border doesn't match @ (%d, %d)",
                   GetWindowName(), pBorderName, BorderRect.left, BorderRect.top);
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

void
Window_t::
GetWindowRect(
    Rect_t& Rect) const
{
    ::GetWindowRect(m_hMainWindow, &Rect);
    ::OffsetRect(&Rect, -Rect.left, -Rect.top);
}

////////////////////////////////////////////////////////////////////////////////

void
Window_t::
DumpWidgets(
    const CSurface& Surface,
    const Rect_t&   RelativeRect) const
{
    for (size_t Widget = 0; Widget < GetWidgetCount(); ++Widget)
    {
        RelativeRect_t Rect(m_pWidgets[Widget].RectData);
        Rect_t WidgetRect = Rect.GetRelativeRect(RelativeRect);
        wchar_t szBuf[255];
        swprintf_s(szBuf, L"Diag\\%ls_widget%d.bmp", GetWindowName(), Widget);
        Surface.WriteBMP(szBuf, WidgetRect);
    }
}

////////////////////////////////////////////////////////////////////////////////

/* static */
Ui::Scroll::Position_t 
Window_t::
GetVertScrollPos(
    const CSurface& Surface,
    const Rect_t&   VScrollUpRect,
    const Rect_t&   VScrollDownRect) //const
{
    extern bool g_bWriteBmps;

    static const size_t ScrollIntensity = 0xb0;
    static const size_t ActiveCount     = 20;

    if (g_bWriteBmps)
    {
        Surface.WriteBMP(L"Diag\\VScrollUpRect.bmp", VScrollUpRect);
        Surface.WriteBMP(L"Diag\\VScrollDownRect.bmp", VScrollDownRect);
    }
    bool bUpActive =   ActiveCount <= Surface.GetIntensityCount(VScrollUpRect, ScrollIntensity);
    bool bDownActive = ActiveCount <= Surface.GetIntensityCount(VScrollDownRect, ScrollIntensity);
    using namespace Ui::Scroll;
    Position_t Pos = Position::Unknown;
    if (!bUpActive && !bDownActive)
    {
        Pos = Position::Unknown;
    }
    else if (bUpActive && bDownActive)
    {
        Pos = Position::Middle;
    }
    else if (bUpActive)
    {
        Pos = Position::Bottom;
    }
    else if (bDownActive)
    {
        Pos = Position::Top;
    }
    else
    {
        throw std::logic_error("MainWindow_t::GetVertScrollPos(): Impossible!");
    }
    //LogAlways(L"MainWindow_t::GertVertScrollPos() ScrollRect = { %d, %d, %d, %d }",
    //          ScrollRect.left, ScrollRect.top, ScrollRect.right, ScrollRect.bottom);
    return Pos;
}

////////////////////////////////////////////////////////////////////////////////

} // Window
} // Ui

////////////////////////////////////////////////////////////////////////////////
