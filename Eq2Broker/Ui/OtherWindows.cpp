////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// OtherWindows.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OtherWindows.h"
#include "DdUtil.h"
#include "Log.h"
#include "BrokerUi.h"
#include "Resource.h"

namespace Broker
{

////////////////////////////////////////////////////////////////////////////////
//
// Eq2LoadingWindow_t
//
////////////////////////////////////////////////////////////////////////////////

Eq2LoadingWindow_t::
Eq2LoadingWindow_t(
)//    const Ui::Window_t& parent)
:
    Ui::Window_t(
        Broker::Window::Id::Eq2Loading,
        *this,
        L"Eq2Loading")
{
    loadSurfaces();
}

////////////////////////////////////////////////////////////////////////////////

void
Eq2LoadingWindow_t::
loadSurfaces()
{
    extern CDisplay* g_pDisplay;
    #if 0 // TODO
    HRESULT hr = g_pDisplay->CreateSurfaceFromBitmap(&m_cancelButton,
        MAKEINTRESOURCE(IDB_EQ2LOADING_CANCEL));
    if (FAILED(hr))
    {
        throw runtime_error("Create Eq2Loading_Cancel surface");
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////

const CSurface*
Eq2LoadingWindow_t::
GetOriginSurface() const
{
    return &m_cancelButton;
}

////////////////////////////////////////////////////////////////////////////////

void
Eq2LoadingWindow_t::
GetOriginSearchRect(
    const CSurface& surface,
          Rect_t&   rect) const
{
    // a rectangle below the middle
    rect = surface.GetBltRect();
    size_t height = rect.Height();
    rect.top += height / 2;
    rect.bottom = rect.top + height / 6;
    size_t widthPart = rect.Width() / 6;
    rect.left += (rect.Width() - widthPart) / 2;
    rect.right = rect.left + widthPart;
}

////////////////////////////////////////////////////////////////////////////////
//
// TransitionWindow_t
//
////////////////////////////////////////////////////////////////////////////////

TransitionWindow_t::
TransitionWindow_t(
)//    const Ui::Window_t& parent)
:
    Ui::Window_t(
        Broker::Window::Id::Zoning,
        *this,
        L"Transition")
{
    loadSurfaces();
}

////////////////////////////////////////////////////////////////////////////////

void
TransitionWindow_t::
loadSurfaces()
{
#if 0 // TODO
    extern CDisplay* g_pDisplay;
    HRESULT hr = S_OK;
    hr = g_pDisplay->CreateSurfaceFromBitmap(&m_transitionImage, MAKEINTRESOURCE(IDB_EQ2_TRANSITION));
    if (FAILED(hr))
    {
        throw runtime_error("Create Eq2_Transition surface");
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////

const CSurface*
TransitionWindow_t::
GetOriginSurface() const
{
    return &m_transitionImage;
}

////////////////////////////////////////////////////////////////////////////////

void
TransitionWindow_t::
GetOriginSearchRect(
    const CSurface& surface,
          Rect_t&   rect) const
{
    // search the upper right sixth 1/8 horizontally, top 1/8 vertically 
    rect = surface.GetBltRect();
    rect.bottom = rect.top + rect.Height() / 8;
    size_t widthPart = rect.Width() / 8;
    rect.left = rect.Width() / 2 + widthPart;
    rect.right = rect.left + widthPart;
}

////////////////////////////////////////////////////////////////////////////////
//
// MainChatWindow_t
//
////////////////////////////////////////////////////////////////////////////////

MainChatWindow_t::
MainChatWindow_t(
)//    const Ui::Window_t& parent)
:
    Ui::Window_t(
        Broker::Window::Id::MainChat,
        *this,
        L"MainChat")
{
    loadSurfaces();
}

////////////////////////////////////////////////////////////////////////////////

void
MainChatWindow_t::
loadSurfaces()
{
#if 0
    extern CDisplay* g_pDisplay;
    HRESULT hr = g_pDisplay->CreateSurfaceFromBitmap(&m_mainChatCaption,
        MAKEINTRESOURCE(IDB_MAINCHAT_CAPTION));
    if (FAILED(hr))
    {
        throw runtime_error("Create MainChat_Caption surface");
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////

const CSurface*
MainChatWindow_t::
GetOriginSurface() const
{
    return &m_mainChatCaption;
}

////////////////////////////////////////////////////////////////////////////////

void
MainChatWindow_t::
GetOriginSearchRect(
    const CSurface& surface,
          Rect_t&   rect) const
{
    // bottom quarter veritcally middle half horizontally
    rect = surface.GetBltRect();
    rect.top = rect.bottom - rect.Height() / 4;
    size_t width = rect.Width();
    rect.left = width / 4;
    rect.right = rect.left + width / 2;
}

////////////////////////////////////////////////////////////////////////////////

} // Broker
