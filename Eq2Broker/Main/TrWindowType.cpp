////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TrWindowType_t.cpp
//
// Window type translate handler.
//
// Typically the first translate handler in the stack; determine the window
// type (Id) of the supplied acquire data (i.e. screenshot).
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TrWindowType.h"
#include "MainWindow_t.h"
#include "SsWindow.h"
#include "BrokerWindow.h"
#include "TabWindow.h"
#include "DdUtil.h"
#include "Log.h"
#include "BrokerUi.h"
#include "OtherWindows.h"

extern bool g_bWriteBmps;

namespace Broker
{

const TrWindowType_t::FnGetWindowId_t TrWindowType_t::s_brokerFunc   = &TrWindowType_t::GetBrokerWindowId;
const TrWindowType_t::FnGetWindowId_t TrWindowType_t::s_mainChatFunc = &TrWindowType_t::GetMainChatWindowId;

const TrWindowType_t::FnGetWindowId_t TrWindowType_t::s_windowIdFuncs[] = 
{
    &TrWindowType_t::GetBrokerWindowId,
    &TrWindowType_t::GetLogonWindowId,
    &TrWindowType_t::GetOtherWindowId,
};

////////////////////////////////////////////////////////////////////////////////

TrWindowType_t::
TrWindowType_t(
    MainWindow_t& mainWindow)
:
    m_mainWindow(mainWindow),
    m_lastWindowId(Ui::Window::Id::Unknown),
    m_lastWindowIdFunc(nullptr)
{
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
TrWindowType_t::
MessageHandler(
    const DP::Message::Data_t* pMessage)
{
    using namespace SsWindow;
    if (typeid(*pMessage) != typeid(Acquire::Data_t))
    {
        return S_FALSE;
    }
    const Acquire::Data_t& SsData = *static_cast<const Acquire::Data_t*>(pMessage);
    using namespace Ui::Window;
    // If the acquire handler didn't initialize the Window Id
    if (Id::Unknown == SsData.WindowId)
    {
        // Try to determine the Window Id by looking at the screenshot bits
        Ui::WindowId_t windowId = GetWindowId(*SsData.pPoolItem->get());
        // If we determined a valid Window Id, hack the id into the message
        if (Id::Unknown != windowId)
        {
            LogInfo(L"TrWindowType_t::MessageHandler() Found Id(%d) Name(%s)", windowId,
                    m_mainWindow.GetWindow(windowId).GetWindowName());
            const_cast<Acquire::Data_t&>(SsData).WindowId = windowId;
            return S_OK;
        }
        LogInfo(L"TrWindowType_t: Unknown WindowId");
        // Can't determine the Window Id, give up on translating this message
        return E_ABORT;
    }
    // Acquire handler set Window Id, we didn't handle this message
    return S_FALSE;
}

////////////////////////////////////////////////////////////////////////////////

Ui::WindowId_t
TrWindowType_t::
GetWindowId(
    const CSurface& surface) const
{
    using namespace Ui::Window;
    Ui::WindowId_t windowId = Id::Unknown;
    if (nullptr != m_lastWindowIdFunc)
    {
        windowId = (this->*m_lastWindowIdFunc)(surface, Locate::CompareLastOrigin);
    }
    if ((Id::Unknown == windowId) || (Window::Id::MainChat == windowId))
    {
        windowId = CompareAllLastOrigins(surface);
        if (Window::Id::MainChat == windowId)
        {
            Ui::WindowId_t loggedInWindowId = SearchLoggedInWindows(surface);
            if (Id::Unknown != loggedInWindowId)
            {
                windowId = loggedInWindowId;
            }
        }
        else
        {
            windowId = SearchAllWindows(surface);
        }
        if (Window::Id::MainChat == windowId)
        {
            m_lastWindowIdFunc = s_mainChatFunc;
        }
    }
    if (m_lastWindowId != windowId)
    {
        m_lastWindowId = windowId;
        LogAlways(L"Window(%s)", GetWindowName(windowId));
    }
    return windowId;
}

////////////////////////////////////////////////////////////////////////////////

Ui::WindowId_t
TrWindowType_t::
CompareAllLastOrigins(
    const CSurface& surface) const
{
    using namespace Ui::Window;
    Ui::WindowId_t windowId = Id::Unknown;
    // first, check 'last origin' of all windows for the supplied bitmap
    for (int func = 0; func < _countof(s_windowIdFuncs); ++func)
    {
        // TODO: optimization: if mainchat = windowid due to it being lastWindowIdFunc, 
        // only check 'logged in' window (currently only brokerwindow)
        if (s_windowIdFuncs[func] != m_lastWindowIdFunc)
        {
            windowId = (this->*s_windowIdFuncs[func])(surface, Locate::CompareLastOrigin);
            if (Id::Unknown != windowId)
            {
                m_lastWindowIdFunc = s_windowIdFuncs[func];
                break;
            }
        }
    }
    // if no match, check for the 'main chat' window at last origin
    if ((Id::Unknown == windowId) && (m_lastWindowIdFunc != s_mainChatFunc))
    {
        windowId = (this->*s_mainChatFunc)(surface, Locate::CompareLastOrigin);
    }
    return windowId;
}

////////////////////////////////////////////////////////////////////////////////

Ui::WindowId_t
TrWindowType_t::
SearchLoggedInWindows(
    const CSurface& surface) const
{
    // check all 'logged in' windows (currently only broker window)
    using namespace Ui::Window;
    Ui::WindowId_t windowId = (this->*s_brokerFunc)(surface, Locate::Search);
    if (Id::Unknown != windowId)
    {
        m_lastWindowIdFunc = s_brokerFunc;
    }
    return windowId;
}

////////////////////////////////////////////////////////////////////////////////

Ui::WindowId_t
TrWindowType_t::
SearchAllWindows(
    const CSurface& surface) const
{
    using namespace Ui::Window;
    Ui::WindowId_t windowId = Id::Unknown;
    for (int func = 0; func < _countof(s_windowIdFuncs); ++func)
    {
        windowId = (this->*s_windowIdFuncs[func])(surface, Locate::Search);
        if (Id::Unknown != windowId)
        {
            m_lastWindowIdFunc = s_windowIdFuncs[func];
            break;
        }
    }
    // all searches failed; search for 'main chat' as last resort
    if (Id::Unknown == windowId)
    {
        windowId = (this->*s_mainChatFunc)(surface, Locate::Search);
    }
    return windowId;
}

////////////////////////////////////////////////////////////////////////////////

const wchar_t*
TrWindowType_t::
GetWindowName(
    Ui::WindowId_t windowId) const
{    
    const wchar_t* pName = L"Error";
    switch (windowId)
    {
    case Ui::Window::Id::Unknown:
        pName = L"Unknown";
        break;
    default:
        pName = m_mainWindow.GetWindow(windowId).GetWindowName();
        break;
    }
    return pName;
}

////////////////////////////////////////////////////////////////////////////////

Ui::WindowId_t
TrWindowType_t::
GetLogonWindowId(
    const CSurface& surface,
    Flag_t flags) const
{
#ifdef OLD_LOGIN
    Ui::WindowId_t windowId = Window::Id::Login;
#else
    Ui::WindowId_t windowId = Window::Id::Eq2Login;
#endif
    return m_mainWindow.GetWindow(windowId).IsLocatedOn(surface, flags) ?
               windowId : Ui::Window::Id::Unknown;
}

////////////////////////////////////////////////////////////////////////////////

Ui::WindowId_t
TrWindowType_t::
GetOtherWindowId(
    const CSurface& surface,
          Flag_t    flags) const
{
    static const Ui::WindowId_t otherWindowIds[] = 
    {
        Window::Id::Eq2Loading,
        Window::Id::Zoning,
    };

    for (int index = 0; _countof(otherWindowIds) > index; ++index)
    {
        if (m_mainWindow.GetWindow(otherWindowIds[index]).IsLocatedOn(surface, flags))
        {
            return otherWindowIds[index];
        }
    }
    return Ui::Window::Id::Unknown;
}

////////////////////////////////////////////////////////////////////////////////

Ui::WindowId_t
TrWindowType_t::
GetBrokerWindowId(
    const CSurface& surface,
    Flag_t flags) const
{
    Ui::WindowId_t windowId = Ui::Window::Id::Unknown;
    POINT origin = { 0, 0 };
    if (m_mainWindow.GetBrokerWindow().IsLocatedOn(surface, flags, &origin))
    {
        windowId = m_mainWindow.GetBrokerWindow().GetWindowId(surface);
        if (Ui::Window::Id::Unknown == windowId)
        {
            windowId = Window::Id::BrokerFrame;
        }
    }
    return windowId;
}

////////////////////////////////////////////////////////////////////////////////

Ui::WindowId_t
TrWindowType_t::
GetMainChatWindowId(
    const CSurface& surface,
          Flag_t    flags) const
{
    if (m_mainWindow.GetWindow(Window::Id::MainChat).IsLocatedOn(surface, flags))
    {
        return Window::Id::MainChat;
    }
    return Ui::Window::Id::Unknown;
}

////////////////////////////////////////////////////////////////////////////////

} // Broker

////////////////////////////////////////////////////////////////////////////////
