////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// Eq2LoginWindow.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Eq2LoginWindow.h"
#include "BrokerUi.h"
#include "DdUtil.h"
#include "Log.h"
#include "Resource.h"

////////////////////////////////////////////////////////////////////////////////

//extern CDisplay* g_pDisplay;

namespace Broker
{
namespace Eq2Login
{

////////////////////////////////////////////////////////////////////////////////

static const Flag_t s_windowFlags;

static const SIZE EditSize      = { 154, 20 };

// character buttons
static const SIZE ButtonSpacing = {   88,  34 }; // y unknown
static const SIZE ButtonSize    = {   75,  19 };
static const SIZE fcbo          = { -278, 133 }; // first character button offset

static const Ui::Widget::Data_t s_widgets[] =
{
    { Widget::Id::CharacterEdit,        { RelativeRect_t::LeftTop, -186,  47, EditSize.cx, EditSize.cy } },
    { Widget::Id::ServerEdit,           { RelativeRect_t::LeftTop, -186,  77, EditSize.cx, EditSize.cy } },
    { Widget::Id::ConnectButton,        { RelativeRect_t::LeftTop, 0, 0, 9, 9 } },
    { Widget::Id::FirstCharacterButton, { RelativeRect_t::LeftTop, fcbo.cx, fcbo.cy, ButtonSize.cx, ButtonSize.cy } },
};

////////////////////////////////////////////////////////////////////////////////

Window_t::
Window_t(
         const Ui::Window_t& parent)
:
    Ui::Window_t(
        Broker::Window::Id::Eq2Login,
        parent,
        L"Eq2Login",
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
    hr = g_pDisplay->CreateSurfaceFromBitmap(&m_connectButton, MAKEINTRESOURCE(IDB_EQ2LOGIN_CONNECT));
    if (FAILED(hr))
    {
        throw runtime_error("Create Eq2Login_Connect surface");
    }
}

////////////////////////////////////////////////////////////////////////////////

const CSurface*
Window_t::
GetOriginSurface() const
{
    return &m_connectButton;
}

////////////////////////////////////////////////////////////////////////////////

void
Window_t::
GetOriginSearchRect(
    const CSurface& surface,
          Rect_t&   rect) const
{
    rect = surface.GetBltRect();
    rect.right /= 3;
    rect.bottom /= 6;;
    ::OffsetRect(&rect, rect.Width(), 0);
}

////////////////////////////////////////////////////////////////////////////////

bool
Window_t::
GetWidgetRect(
    Ui::WidgetId_t WidgetId,
    Rect_t&        WidgetRect) const
{
    static const size_t ButtonsPerRow = 4;
    using namespace Widget;
    if ((Id::FirstCharacterButton < WidgetId) &&
        (Id::LastCharacterButton >= WidgetId))
    {
        // recursive call to get first character button rect
        if (GetWidgetRect(Id::FirstCharacterButton, WidgetRect))
        {
            size_t offset = WidgetId - Id::FirstCharacterButton;
            if (0 < offset)
            {
                ::OffsetRect(&WidgetRect, (offset % ButtonsPerRow) * ButtonSpacing.cx,
                                          (offset / ButtonsPerRow) * ButtonSpacing.cy);
            }
            return true;
        }
        throw logic_error("Eq2LoginWindow::GetWidgetRect()");
    }
    else
    {
        const POINT& ptOrigin = GetLastOrigin();
        Rect_t origin;
        ::SetRect(&origin, ptOrigin.x, ptOrigin.y, ptOrigin.x + 1, ptOrigin.y + 1);
        return Ui::Window_t::GetWidgetRect(WidgetId, origin, WidgetRect);
    }
}

////////////////////////////////////////////////////////////////////////////////

} // Eq2Login
} // Broker

////////////////////////////////////////////////////////////////////////////////
