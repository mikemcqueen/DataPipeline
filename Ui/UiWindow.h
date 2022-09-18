////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// UiWindow.h
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_UIWINDOW_H
#define Include_UIWINDOW_H

#include "UiTypes.h"
#include "UiWindowId.h"
#include "Rect.h"

class CSurface;

////////////////////////////////////////////////////////////////////////////////

namespace Ui
{

typedef Window::Base_t Window_t;

namespace Window
{

    namespace Locate
    {
        static const unsigned CompareLastOrigin = 0x01;
        static const unsigned Search            = 0x02;
    }

    ////////////////////////////////////////////////////////////////////////////
    //
    // Ui::Window::Handle_t
    //
    ////////////////////////////////////////////////////////////////////////////

    struct Handle_t
    {
        HWND       hWnd;
        WindowId_t WindowId;

        Handle_t(
            HWND       InitHwnd       = nullptr,
            WindowId_t InitWindowId   = Id::Unknown)
        :
            hWnd(InitHwnd),
            WindowId(InitWindowId)
        { }
    };

    ////////////////////////////////////////////////////////////////////////////
    //
    // Ui::Window::Base_t
    //
    ////////////////////////////////////////////////////////////////////////////

    class Base_t
    {
    private:

        const Base_t&      m_ParentWindow;

        WindowId_t         m_WindowId;
        HWND               m_hMainWindow;
        std::wstring       m_strClassName;
        std::wstring       m_strWindowName;
        Flag_t             m_Flags;
        Scroll::Position_t m_VertScrollPos;
        Scroll::Position_t m_HorzScrollPos;
        POINT              m_ptLastOrigin;

        const Widget::Data_t* m_pWidgets;
        size_t                m_WidgetCount;

    public:

        explicit
        Base_t(
            WindowId_t     WindowId,
            const wchar_t* pClassName,
            const wchar_t* pWindowName = nullptr,
            Flag_t         Flags = 0);

        explicit
        Base_t(
            WindowId_t            WindowId,
            const Base_t&         ParentWindow,
            const wchar_t*        pWindowName,
            Flag_t                Flags = 0,
            const Widget::Data_t* pWidgetData = nullptr,
            size_t                WidgetCount = 0);

        virtual 
        ~Base_t();

        virtual
        HWND
        GetSsWindowRect(
            RECT& Rect) const;

        virtual
        HWND
        GetHwnd(
            WindowId_t /*Id*/) const
        {
            return m_hMainWindow;
        }

        virtual
        Handle_t
        GetTopWindow() const
        {
            return Handle_t(m_hMainWindow, m_WindowId);
        }

        virtual
        Ui::Window::Base_t&
        GetWindow(
            Ui::WindowId_t WindowId) const;

        // Determine the active window Id and rectangle by looking
        // at pixels of the supplied surface:
        virtual
        WindowId_t
        GetWindowId(
            const CSurface& Surface,
            const POINT*    pptHint) const;

        // Determine the rectangle of the supplied widget Id.
        // Return true if the widget was found, false otherwise.
        virtual
        bool
        GetWidgetRect(
            Ui::WidgetId_t  WidgetId,
                  Rect_t&   WidgetRect) const;

        virtual
        bool
        UpdateScrollPosition(
            Scroll::Bar_t   /*ScrollBar*/,
            const CSurface& /*Surface*/)
        {
            return false;
        }

        virtual
        bool
        IsLocatedOn(
            const CSurface& Surface,
                  Flag_t    flags,
                  POINT*    pptOrigin = nullptr) const;

        virtual
        const CSurface*
        GetOriginSurface() const
        {        
            return nullptr;
        }

        virtual
        void
        GetOriginSearchRect(
            const CSurface& surface,
                  Rect_t&   rect) const;

        //
        // Helper methods
        //

        bool
        ClickWidget(
            WidgetId_t WidgetId,
                  bool    bDirect = false,
            const Rect_t* pRect = nullptr) const;

        bool
        SetWidgetText(
                  WidgetId_t    WidgetId,
            const std::wstring& text,
                  bool          bDirect = false) const;

        bool
        ClearWidgetText(
            WidgetId_t widgetId,
            size_t     count) const;

        const Widget::Data_t&
        GetWidgetData(
            WidgetId_t widgetId) const;

        WindowId_t
        GetWindowId() const
        {
            return m_WindowId;
        }

        HWND
        GetHwnd() const
        {
            return m_hMainWindow;
        }

        const wchar_t*
        GetWindowName() const 
        {
            return m_strWindowName.c_str();
        }

        Flag_t
        GetFlags() const
        {
            return m_Flags;
        }

        Scroll::Position_t
        GetScrollPosition(
            Scroll::Bar_t ScrollBar) const
        {
            // TODO: Lock
            using namespace Scroll::Bar;
            return (Vertical == ScrollBar) ? m_VertScrollPos : m_HorzScrollPos;
        }

        void
        SetScrollPosition(
            Scroll::Bar_t      ScrollBar,
            Scroll::Position_t ScrollPos)
        {
            // TODO: Lock
            using namespace Scroll::Bar;
            if (Vertical == ScrollBar)
            {
                m_VertScrollPos = ScrollPos;
            }
            else
            {
                m_HorzScrollPos = ScrollPos;
            }
        }

        bool
        Scroll(
            Scroll::Direction_t Direction) const; // ,size_t Count)

        bool
        ValidateBorders(
            const CSurface& Surface,
            const Rect_t&   Rect,
            const SIZE&     BorderSize,
            const COLORREF  LowColor,
            const COLORREF  HighColor) const;

        bool
        ValidateBorder(
            const CSurface& Surface,
            const Rect_t&   BorderRect,
            const wchar_t*  pBorderName,
            const COLORREF  LowColor,
            const COLORREF  HighColor) const;

        size_t
        GetWidgetCount() const
        {
            return m_WidgetCount;
        }

        bool
        GetWidgetRect(
                  Ui::WidgetId_t  WidgetId,
            const Rect_t&         RelativeRect,
                  Rect_t&         WidgetRect,
            const Widget::Data_t* pWidgets = nullptr,
                  size_t          mwidgetCount = 0) const;

        void
        DumpWidgets(
            const CSurface& Surface,
            const Rect_t&   RelativeRect) const;

        void
        GetWindowRect(
            Rect_t& Rect) const;

        bool
        FindOrigin(
            const CSurface& Surface,
            const POINT&    ptHint,
                  POINT&    ptOrigin) const;

        void
        SetLastOrigin(
            POINT ptOrigin)
        {
            m_ptLastOrigin = ptOrigin;
        }

        const POINT&
        GetLastOrigin() const
        {
            return m_ptLastOrigin;
        }

        bool
        CompareLastOrigin(
            const CSurface& surface,
            const CSurface& image,
                  POINT*    pptOrigin) const;

        bool
        OriginSearch(
            const CSurface& surface,
            const CSurface& image,
                  POINT*    pptOrigin) const;

        const Base_t& GetParent() const { return m_ParentWindow; }

    public:

        static
        Ui::Scroll::Position_t 
        GetVertScrollPos(
            const CSurface& Surface,
            const Rect_t&   ScrollUpRect,
            const Rect_t&   ScrollDownRect);

    private:

        Base_t();
        Base_t(const Base_t&);
        const Base_t& operator=(const Base_t&);
    };

////////////////////////////////////////////////////////////////////////////////

} // Window
} // Ui

////////////////////////////////////////////////////////////////////////////////

#endif // Include_UIWINDOW_H

////////////////////////////////////////////////////////////////////////////////
