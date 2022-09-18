////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// LoginWindow.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LoginWindow.h"
#include "BrokerUi.h"
#include "DdUtil.h"
#include "Log.h"

////////////////////////////////////////////////////////////////////////////////

extern CDisplay* g_pDisplay;

namespace Broker
{
namespace Login
{

////////////////////////////////////////////////////////////////////////////////

static const Flag_t s_windowFlags;

static const SIZE ButtonSize = {  64, 32 };
static const SIZE EditSize   = { 156, 16 };

static const Ui::Widget::Data_t s_widgets[] =
{
    { Widget::Id::UsernameLabel, { RelativeRect_t::LeftTop,       11, -178,  9,  9 } },
    { Widget::Id::UsernameEdit,  { RelativeRect_t::LeftBottom,    -42,   43, EditSize.cx, EditSize.cy } },
    { Widget::Id::PasswordLabel, { RelativeRect_t::LeftBottom,   140,   35, ButtonSize.cx, ButtonSize.cy } },
    { Widget::Id::PasswordEdit,  { RelativeRect_t::LeftBottom,   -42,   77, EditSize.cx, EditSize.cy } },
    { Widget::Id::LoginButton,   { RelativeRect_t::RightBottom, -115,   35, ButtonSize.cx, ButtonSize.cy } },
    { Widget::Id::ExitButton,    { RelativeRect_t::RightBottom, -115,   35, ButtonSize.cx, ButtonSize.cy } },
};

////////////////////////////////////////////////////////////////////////////////

Window_t::
Window_t(
         const Ui::Window_t& parent)
:
    Ui::Window_t(
        Broker::Window::Id::Login,
        parent,
        L"Login",
        s_windowFlags,
        s_widgets,
        _countof(s_widgets))
{
    loadSurfaces();
}

////////////////////////////////////////////////////////////////////////////////

void
Window_t::
loadSurfaces()
{
    extern CDisplay* g_pDisplay;
    HRESULT hr = S_OK;
    hr = g_pDisplay->CreateSurfaceFromBitmap(&m_loginCaption, MAKEINTRESOURCE(IDB_LOGIN_CAPTION));
    if (FAILED(hr))
    {
        throw runtime_error("Create LoginCaption surface");
    }
}

////////////////////////////////////////////////////////////////////////////////

bool
Window_t::
IsLocatedOn(
    const CSurface& surface,
          Flag_t    flags,
          POINT*    pptOrigin) const
{
    using namespace Ui::Window;
    if (flags.Test(Locate::CompareLastOrigin) &&
        CompareLastOrigin(surface, m_loginCaption, pptOrigin))
    {
        return true;
    }
    if (flags.Test(Locate::Search) &&
        OriginSearch(surface, m_loginCaption, pptOrigin))
    {
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

} // Login
} // Broker

////////////////////////////////////////////////////////////////////////////////
