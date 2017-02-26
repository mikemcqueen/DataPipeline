///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// LonWindow_t.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LonWindow_t.h"
#include "Log.h"
#include "Macros.h"
#include "PipelineManager.h"

using namespace Lon;

///////////////////////////////////////////////////////////////////////////////

static const
struct TitleType_t
{
    Window::Type_e Type;
    const wchar_t* pszTitle;
} s_WindowTitles[] =
{
    Window::PostedTradesWindow,       L"postedTradesDialog",
    Window::PostedTradeDetailWindow,  L"postedTradeDetail",
    Window::TradeBuilderWindow,       L"postedTradeBuilderWrapper",
    Window::TradeBuilderExitPrompt,   L"exitTradePrompt",
    Window::SystemMessageWindow,      L"systemMessageDialog",
    Window::ConfirmTradeWindow,       L"confirmDialog",
    Window::AcceptTradeWindow,        L"requestAcceptDialog",
    Window::CannotTradeWindow,        L"Cannot Trade",
    Window::DeliveryWindow,           L"deliveryWindow",
    Window::NetworkStatusWindow,      L"Network Status",
    Window::JoinGuildWindow,          L"requestJoinGuild",
    Window::CancelTradeWindow,        L"requestCancelDialog",
};

///////////////////////////////////////////////////////////////////////////////

HWND LonWindow_t::m_hWnd = NULL;

LonWindow_t::
LonWindow_t() :
    m_pTrader(NULL)
{
    Initialize();
}

///////////////////////////////////////////////////////////////////////////////

bool
LonWindow_t::
Initialize()
{
    GetMainWindow();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

LonWindow_t::Handle_t
LonWindow_t::
GetTopWindow() //const
{
    Handle_t Handle;
    HWND hWnd = GetMainWindow();
    if (NULL != hWnd)
    {
        hWnd = ::GetTopWindow(hWnd);
        Handle.Type = GetWindowType(hWnd);
        Handle.hWnd = hWnd;
    }
    return Handle;
}

///////////////////////////////////////////////////////////////////////////////

Window::Type_e
LonWindow_t::
GetWindowType(
    HWND hWnd) //const
{
    if ((NULL == hWnd) || !IsWindow(hWnd))
        return Window::Unknown;
    wchar_t szBuf[256] = { 0 };
    if (!GetWindowText(hWnd, szBuf, _countof(szBuf)))
    {
    #ifdef EXTRALOG
        LogInfo(L"GetWindowText(0x%X) failed");
        ::GetWindowText(hWnd, szBuf, _countof(szBuf));
        LogInfo(L"GetWindowText = %ls", szBuf);
    #endif
        return Window::Unknown;
    }
    return GetWindowType(szBuf);
}

///////////////////////////////////////////////////////////////////////////////

Window::Type_e
LonWindow_t::
GetWindowType(
    const wchar_t* pszTitle) //const
{
    for (size_t Pos = 0; Pos < _countof(s_WindowTitles); ++Pos)
    {
        const TitleType_t& tt = s_WindowTitles[Pos];
        if (0 == wcsncmp(tt.pszTitle, pszTitle, wcslen(tt.pszTitle)))
            return tt.Type;
    }
    LogWarning(L"GetWindowType(%ls) unknown", pszTitle);
    return Window::Unknown;
}

///////////////////////////////////////////////////////////////////////////////

HWND
LonWindow_t::
GetChildWindow(
          HWND     hWnd,
    const wchar_t* pszTitle) //const
{
    ASSERT(NULL != pszTitle);
    hWnd = ::GetWindow(hWnd, GW_CHILD);
    while (NULL != hWnd)
    {
        WCHAR szBuf[256] = { 0 };
        if (GetWindowText(hWnd, szBuf, _countof(szBuf)) &&
            (0 == wcscmp(pszTitle, szBuf)))
                break;
        hWnd = ::GetWindow(hWnd, GW_HWNDNEXT);
    }
    return hWnd;
}
///////////////////////////////////////////////////////////////////////////////

HWND
LonWindow_t::
GetWindow(
    Window::Type_e Type)
{
    return FindWindow(Type);
}

///////////////////////////////////////////////////////////////////////////////

HWND
LonWindow_t::
GetMainWindow()
{
    if (NULL != m_hWnd)
    {
        // ValidateMainWindow()
        // TODO: lock, or interlocked?
        if (IsWindow(m_hWnd))
            return m_hWnd;

        m_hWnd = NULL;
    }
    m_hWnd = FindWindow(Window::MainWindow);
    return m_hWnd;
}

///////////////////////////////////////////////////////////////////////////////

bool
LonWindow_t::
GetWindowRect(
    Window::Type_e Type,
    RECT&          Rect,
    Window::Type_e RelativeType) //const
{
    SetRectEmpty(&Rect);
    HWND hRelative = GetWindow(RelativeType);
    if ((NULL == hRelative) || !IsWindowVisible(hRelative))
        return false;
    HWND hWnd = GetWindow(Type);
    if (NULL == hWnd)
        return false;
    ::GetClientRect(hWnd, &Rect);
    ConvertRect(hWnd, Rect, hRelative);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
LonWindow_t::
ConvertRect(
    Window::Type_e Type,
    RECT&  rc,
    Window::Type_e RelativeType)
{
    HWND hRelative = GetWindow(RelativeType);
    if (NULL == hRelative)
        return false;
    HWND hWnd = GetWindow(Type);
    if (NULL == hWnd)
        return false;
    ConvertRect(hWnd, rc, hRelative);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void
LonWindow_t::
ConvertRect(
    HWND  hWnd,
    RECT& rc,
    HWND  hRelative)
{
    if (hWnd == hRelative)
        return;
    ClientToScreen(hWnd, (POINT*)&rc);
    ClientToScreen(hWnd, (POINT*)&rc + 1);
    ScreenToClient(hRelative, (POINT*)&rc);
    ScreenToClient(hRelative, (POINT*)&rc + 1);
}

///////////////////////////////////////////////////////////////////////////////

HWND
LonWindow_t::
GetSsWindowRect(
    RECT& rcBounds)// const
{
    Handle_t Top(GetTopWindow());
    if ((NULL == Top.hWnd) || (Window::Unknown == Top.Type))
        return NULL;

    Window::Type_e WindowType = GetSsWindowType(Top.Type);
    if (Window::Unknown == WindowType)
        return NULL;

    HWND hWnd = GetWindow(WindowType);
    if (NULL != hWnd)
        ::GetClientRect(hWnd, &rcBounds);
    return hWnd;
}

///////////////////////////////////////////////////////////////////////////////

Window::Type_e
LonWindow_t::
GetSsWindowType(
    Window::Type_e TopWindowType) 
{
    using namespace Window;
    switch (TopWindowType)
    {
    case PostedTradesWindow:
        return PostedTradesList;
    case TradeBuilderWindow:
        return TradeBuilderCollectionFrame;

    case PostedTradeDetailWindow:
    case ConfirmTradeWindow:
    case TradeBuilderExitPrompt: 
    case SystemMessageWindow:
    case NetworkStatusWindow:
    case AcceptTradeWindow:
    case DeliveryWindow:
    case JoinGuildWindow:
    case CancelTradeWindow:
        return TopWindowType;

    case CannotTradeWindow:
        break;

    case Unknown:
    default:
        ASSERT(false);
        break;
    }
    return Unknown;
}

////////////////////////////////////////////////////////////////////////////////

bool
LonWindow_t::
CanScroll(
    Window::Type_e ScrollType)
{
    ASSERT(Window::Unknown != ScrollType);
    HWND hScroll = GetWindow(ScrollType);
    if ((NULL == hScroll) || !IsWindow(hScroll) || !IsWindowVisible(hScroll))
        return false;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
LonWindow_t::
Scroll(
   Window::Type_e        WindowType,
   ScrollBar_t::Type_e   ScrollBar,
   EventScroll_t::Type_e ScrollType,
   size_t                Count,
   size_t                PageLines)
{
    if (NULL == ValidateWindow(WindowType))
        return false;
    EventScroll_t ScrollEvent(WindowType, ScrollBar, ScrollType, Count, PageLines);
    return 0 < GetPipelineManager().SendEvent(ScrollEvent.m_Data);
}

///////////////////////////////////////////////////////////////////////////////

ScrollBar_t::ThumbPosition_e
LonWindow_t::
GetThumbPosition(
   Window::Type_e      WindowType,
   ScrollBar_t::Type_e ScrollBarType)
{
    ScrollBar_t Scrollbar(ScrollBarType);
    EventThumbPosition_t ThumbEvent(WindowType, Scrollbar);
//    ThumbEvent.m_Data.Flags |= DP::Event::Flags::Query; // NOTE commented out to make it compile
    size_t HandlerCount = GetPipelineManager().SendEvent(ThumbEvent.m_Data);
    if (1 == HandlerCount)
    {
        ASSERT(ScrollBar_t::UnknownPosition != ThumbEvent.m_Data.ScrollBar.Position);
        return ThumbEvent.m_Data.ScrollBar.Position;
    }
    LogError(L"LonWindow_t::GetThumbPosition(): Handlers (%d)", HandlerCount);
    return ScrollBar_t::UnknownPosition;
}

///////////////////////////////////////////////////////////////////////////////

bool
LonWindow_t::
Click(
    Window::Type_e         WindowType,
    const RECT*            pRect,
    EventClick_t::Button_e Button,
          size_t           Count,
          bool             bDoubleClick,
          bool             bDirect)
{
    HWND hWnd = ValidateWindow(WindowType);
    if (NULL == hWnd)
        return false;
    RECT rc = { 0 };
    if (NULL == pRect)
    {
        ::GetClientRect(hWnd, &rc);
        pRect = &rc;
    }
    POINT pt = { pRect->left + RECTWIDTH(*pRect) / 2,
                 pRect->top + RECTHEIGHT(*pRect) / 2 };
    if (bDirect)
    {
        LPARAM lParam = MAKELONG(pt.x, pt.y);
        while (0 < Count--)
            Click(hWnd, lParam, Button, bDoubleClick);
    }
    else
    {
        EventClick_t ClickEvent(WindowType, Button, pt, Count, bDoubleClick);
        if (0 == GetPipelineManager().SendEvent(ClickEvent.m_Data))
            return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void
LonWindow_t::
Click(
    HWND                   hWnd, 
    LPARAM                 lParam,
    EventClick_t::Button_e Button,
    bool                   bDoubleClick)
{
    static const
    struct ButtonMessageInfo
    {
        LonWindow_t::EventClick_t::Button_e Button;
        DWORD  Up;
        DWORD  Down;
        WPARAM wParam;
    } ButtonMessageMap[] = 
    {
        LonWindow_t::EventClick_t::LeftButton,  WM_LBUTTONUP, WM_LBUTTONDOWN, MK_LBUTTON,
        LonWindow_t::EventClick_t::RightButton, WM_RBUTTONUP, WM_RBUTTONDOWN, MK_RBUTTON
    };

    for (size_t Index = 0; Index < _countof(ButtonMessageMap); ++Index)
    {
        if (ButtonMessageMap[Index].Button == Button)
        {
            const ButtonMessageInfo& bmi = ButtonMessageMap[Index];
#define Msg SendMessage
            Msg(hWnd, bmi.Down, bmi.wParam, lParam);
            Msg(hWnd, bmi.Up,   bmi.wParam, lParam);
            if (bDoubleClick)
            {
                Msg(hWnd, WM_LBUTTONDBLCLK, MK_LBUTTON, lParam);
                Msg(hWnd, WM_LBUTTONUP,     MK_LBUTTON, lParam);
            }
            return;
        }
    }
    LogError(L"Click(): Unsupported button type (%d)", Button);
}

///////////////////////////////////////////////////////////////////////////////

bool
LonWindow_t::
SendChars(
    Window::Type_e WindowType,
    const wchar_t* pszText)
{
    HWND hWnd = ValidateWindow(WindowType);
    if (NULL == hWnd)
        return false;
    EventSendChars_t Event(WindowType, pszText);
    return 0 < GetPipelineManager().SendEvent(Event.m_Data);
}

///////////////////////////////////////////////////////////////////////////////

void
LonWindow_t::
SendCharsDirect(
          HWND     hWnd,
    const wchar_t* pszText)
{
    RECT rc = { 0 };
    ::GetClientRect(hWnd, &rc);
    POINT pt = { rc.left + RECTWIDTH(rc) / 2,
                 rc.top + RECTHEIGHT(rc) / 2 };
    LPARAM lParam = MAKELONG(pt.x, pt.y);
    Click(hWnd, lParam);
//    SetFocus(hWnd);
    for(; L'\0' != *pszText; ++pszText)
        SendKey(hWnd, *pszText);
}

///////////////////////////////////////////////////////////////////////////////

void
LonWindow_t::
SendKey(
    HWND    hWnd, 
    wchar_t Key)
{
    SHORT VirtKey = VkKeyScan(Key);
    ASSERT(0xffff != VirtKey);
    VirtKey &= 0xff;
    UINT ScanCode = MapVirtualKey(VirtKey, 0); // MAPVK_VK_TO_VSC);
    ASSERT(0 != ScanCode);
    ScanCode = (ScanCode << 16) & 0x00ff0000;
    SendMessage(hWnd, WM_CHAR, Key, 0x00000001 | ScanCode);
}

///////////////////////////////////////////////////////////////////////////////

HWND
LonWindow_t::
ValidateWindow(
    Window::Type_e WindowType)
{
    HWND hWnd = GetWindow(WindowType);
    if ((NULL == hWnd) || !IsWindowVisible(hWnd))
    {
        // LogInfo(L"ValidateWindow(): HWND is NULL or window is not visible (%d)", hWnd);
        return NULL;
    }
    const LonWindow_t::Handle_t Top(GetTopWindow());
    if ((Top.hWnd != hWnd) && !util::IsParent(Top.hWnd, hWnd))
    {
        LogError(L"Validate(): Destination window (%d) is not top window (%d)",
                 WindowType, Top.Type);
        return NULL;
    }
    return hWnd;
}

///////////////////////////////////////////////////////////////////////////////

size_t
LonWindow_t::
GetWindowText(
    HWND     hWnd,
    wchar_t* pBuffer,
    size_t   BufferLen)
{
    DWORD dwLen = (DWORD)SendMessage(hWnd, WM_GETTEXT, BufferLen, (LPARAM)pBuffer);
    if (0 == dwLen)
    {
        LRESULT lResult =
            SendMessageTimeout(hWnd, WM_GETTEXT, BufferLen, (LPARAM)pBuffer,
                               SMTO_BLOCK, 15000, &dwLen);
        if ((0 == lResult) || (0 == dwLen))
            return 0;
    }
    return dwLen;
}

///////////////////////////////////////////////////////////////////////////////

const wchar_t*
LonWindow_t::
GetWindowTitle(
    const Window::Type_e Type)
{
    for (size_t Pos = 0; _countof(s_WindowTitles) > Pos; ++Pos)
    {
        if (Type == s_WindowTitles[Pos].Type)
            return s_WindowTitles[Pos].pszTitle;
    }

    static wchar_t szNumericBuffer[16];
    wsprintf(szNumericBuffer, L"%d", Type);
    return szNumericBuffer;
}

///////////////////////////////////////////////////////////////////////////////
