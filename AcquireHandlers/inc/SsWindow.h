////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// SsWindow.h
//
// Screenshot acquire handler with knowledge of Window behaviors such
// as clicking, scrolling, and entering text.
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_SSWINDOW_H
#define Include_SSWINDOW_H

#include "SsTask_t.h"
#include "UiWindow.h"
#include "UiEvent.h"
#include "UiWindowId.h"
#include "AutoCs.h"

namespace SsWindow
{
namespace Acquire
{

////////////////////////////////////////////////////////////////////////////////

    struct Data_t :
        public SsTask::Acquire::Data_t
    {
        Ui::WindowId_t WindowId;
//        Rect_t         SurfaceRect;

        Data_t(
            const wchar_t*     pClass,
            Ui::WindowId_t     InitWindowId,
            Rect_t&            InitSurfaceRect,
            SurfacePoolItem_t* pPoolItem,
            size_t             Size = sizeof(Data_t))
        :
            SsTask::Acquire::Data_t(
                pClass,
                pPoolItem,
                Size)
            ,WindowId(InitWindowId)
//            ,SurfaceRect(InitSurfaceRect)
        { InitSurfaceRect; }        
    };

////////////////////////////////////////////////////////////////////////////////

    class Handler_t:
        public SsTask_t
    {
    private:

        Ui::Window::Base_t& m_Window;
        bool                m_bClick;

    public:

        Handler_t(
            CDisplay&           Display,
            Ui::Window::Base_t& Window);

        // 
        // DP::Handler_t virtual:
        //

        virtual
        HRESULT
        EventHandler(
            DP::Event::Data_t& Event);

        // 
        // SSTask virtual:
        //

        virtual
        HWND
        GetSsWindowRect(
            RECT& rcBounds) const;

        virtual
        void
        ThreadProcessEvent();

        virtual
        void
        PostData(
            HWND              hWnd,
            SurfacePoolItem_t* pPoolItem);

        // Helper methods:

        bool
        ToggleClick()
        {
            return m_bClick = !m_bClick;
        }

    private:

        void
        AsyncEvent(
            const DP::Event::Data_t& Data);

        void
        ClickPoint(
            const Ui::Event::Click::Data_t& Data,
                  POINT                     Point);

        void
        ClickWidget(
            const Ui::Event::Click::Data_t& Data);

        void
        SendChars(
            const Ui::Event::SendChars::Data_t& Data);

        bool
        ValidateEventData(
            const DP::Event::Data_t& Data,
            size_t             Size);

        HWND
        ValidateWindow(
            Ui::WindowId_t WindowId);
    }; // Handler_t

////////////////////////////////////////////////////////////////////////////////

} // Acquire
} // SsWindow

#endif // Include_SSWINDOW_H

////////////////////////////////////////////////////////////////////////////////
