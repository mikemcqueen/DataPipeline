////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// BrokerWindow.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UiWindow.h"
#include "BrokerUi.h"
#include "DdUtil.h"

class TabWindow_t;

namespace Broker
{
//TODO: namespace Frame {
////////////////////////////////////////////////////////////////////////////////

    class Window_t :
        public Ui::Window_t
    {
    private:

        // Broker window
        CSurface  m_brokerCaption;
        CSurface  m_buyTabActive;
        CSurface  m_buyTabInactive;

        // Market window
        CSurface  m_marketCaption;
        CSurface  m_browseTabActive;
        CSurface  m_browseTabInactive;

        // Broker + Market windows
        CSurface  m_sellTabActive;
        CSurface  m_sellTabInactive;
        CSurface  m_salesLogTabActive;
        CSurface  m_salesLogTabInactive;

        mutable Frame::Layout_t m_layout;

    public:

        static
        bool
        IsTabWindow(
            Ui::WindowId_t windowId);

        static
        Ui::WidgetId_t
        GetTabWidgetId(
            Ui::WindowId_t windowId);

       // Constructor
        explicit
        Window_t(
            const Ui::Window_t& parent);

        //
        // Ui::Window_t virtual:
        //
        virtual
        Ui::Window::Base_t&
        GetWindow(
            Ui::WindowId_t windowId) const override;

        virtual
        Ui::WindowId_t
        GetWindowId(
            const CSurface& Surface,
            const POINT*    pptHint = NULL) const override;

        virtual
        bool
        IsLocatedOn(
            const CSurface& Surface,
                  Flag_t    flags,
                  POINT*    pptOrigin = NULL) const override;

        virtual
        void
        GetOriginSearchRect(
            const CSurface& surface,
                  Rect_t&   rect) const override;

        // Misc functions:
        Tab_t
        FindActiveTab(
            const CSurface& Surface,
            const POINT&    ptOrigin,
                  POINT&    ptTab) const;

        const Rect_t&
        GetTabAreaRect(
            const POINT& ptOrigin) const;

        TabWindow_t&
        GetTabWindow(
            Tab_t Tab) const;

        Frame::Layout_t GetLayout() const { return m_layout; }

    private:

        void
        loadSurfaces();

        MainWindow_t&
        GetMainWindow() const;

        void
        SetLayout(
            Frame::Layout_t layout) const;

    private:

        Window_t();
        Window_t(const Window_t&);
        Window_t& operator=(const Window_t&);
    };

////////////////////////////////////////////////////////////////////////////////

} // Broker

////////////////////////////////////////////////////////////////////////////////
