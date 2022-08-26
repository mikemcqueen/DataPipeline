/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// SsTrades_t.h
//
// LoN Posted Trades screen shots.
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_SSTRADES_T_H
#define Include_SSTRADES_T_H

#include "SsTask_t.h"
#include "LonWindow_t.h"
#include "AutoCs.h"

/////////////////////////////////////////////////////////////////////////////

class SsTrades_t :
    public SsTask_t
{
    typedef SsTask_t Base_t;

public:

    struct ScreenShotData_t :
        public Lon::Message::Data_t
    {
        SurfacePoolItem_t* pPoolItem;

        ScreenShotData_t(
            const wchar_t*            Class,
                  Lon::Window::Type_e WindowType,
                  SurfacePoolItem_t*  InitPoolItem,
                  size_t              Size = sizeof(ScreenShotData_t))
        :
            Lon::Message::Data_t(
                Class,
                DP::Stage::Acquire,
                DP::Message::Type::Message,
                Lon::Message::Id::ScreenShot,
                WindowType,
                Size),
            pPoolItem(InitPoolItem)
        { }

        virtual
        ~ScreenShotData_t()
        {
            pPoolItem->release();
        }

    private:

        ScreenShotData_t();
    };
    typedef ScreenShotData_t AcquireData_t;

private:

    mutable
    CAutoCritSec     m_csThumbs;

    Lon::Window::Type_e m_HorzType;
    Lon::ScrollBar_t  m_HorzBar;

    Lon::Window::Type_e m_VertType;
    Lon::ScrollBar_t  m_VertBar;

    bool             m_bClick;

public:

    SsTrades_t(
        CDisplay& Display);

    bool
    ToggleClick()
    {
        return m_bClick = !m_bClick;
    }

    // 
    // DP::Handler_t virtual:
    //

    HRESULT
    EventHandler(
        DP::Event::Data_t& Event) override;

    // 
    // SSTask virtual:
    //

    HWND
    GetSsWindowRect(
        RECT& rcBounds) const override;

    void
    ThreadProcessEvent() override;

    void
    PostData(
        HWND               hWnd,
        SurfacePoolItem_t* pPoolItem) override;

private:

    void
    AsyncEvent(
        const DP::Event::Data_t& Data);

    bool
    IsWindowTypeSupported(
        Lon::Window::Type_e Type) const;

    void
    OnThumbPositionEvent(
        LonWindow_t::EventThumbPosition_t::Data_t& Data);

    Lon::ScrollBar_t::ThumbPosition_e
    GetThumbPosition(
        Lon::Window::Type_e     WindowType,
        Lon::ScrollBar_t::Type_e ScrollType) const;

    void
    SetThumbPosition(
              Lon::Window::Type_e WindowType,
        const Lon::ScrollBar_t&    ScrollBar);

    void
    ScrollWindow(
        const LonWindow_t::EventScroll_t::Data_t& Data);

    void
    ClickWindow(
        const LonWindow_t::EventClick_t::Data_t& Data);

    void
    DoClick(
        HWND   hWnd,
        LPARAM lParam,
        size_t Count,
        LonWindow_t::EventClick_t::Button_e Button = LonWindow_t::EventClick_t::LeftButton,
        bool   bDouble = false);

    void
    SendChars(
        const DP::Event::Data_t& Data);

private:

    // Explicitly disabled:

    SsTrades_t() = delete;
    SsTrades_t(const SsTrades_t&) = delete;
    SsTrades_t& operator=(const SsTrades_t&) = delete;
};

/////////////////////////////////////////////////////////////////////////////

#endif //Include_SSTRADES_T_H

/////////////////////////////////////////////////////////////////////////////
