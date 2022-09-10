////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// TabWindow.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UiWindow.h"
#include "UiTypes.h"
#include "Log.h"

class CSurface;

////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    //
    // TabWindow_t
    //
    ////////////////////////////////////////////////////////////////////////////

    class TabWindow_t :
        public Ui::Window::Base_t
    {
    private:

        mutable POINT m_TabOffset;

    public:

        TabWindow_t(
                  Ui::WindowId_t      WindowId,
            const Ui::Window_t&       parent,
            const wchar_t*            pWindowName,
                  Flag_t              Flags,
            const Ui::Widget::Data_t* pWidget,
                  size_t              WidgetCount,
                  POINT               TabOffset);

        bool
        FindTab(
            const CSurface& surface,
            const Rect_t&   tabAreaRect,
            const CSurface& tabSurface,
            const POINT&    ptOrigin,
                  POINT&    ptTabFound) const;

        const POINT&
        GetTabOffset() const
        {
            return m_TabOffset;
        }

        void
        SetTabOffset(POINT offset)
        {
            m_TabOffset = offset;
        }
    };

    ////////////////////////////////////////////////////////////////////////////
    //
    // TableWindow_t
    //
    ////////////////////////////////////////////////////////////////////////////

    class TableWindow_t :
        public TabWindow_t
    {
    private:

        static const int      MinTableWidth = 500; // wider than SearchEdit box

    private:

        const Rect_t    m_InnerTableRect;

        mutable POINT   m_TableOffset;
        mutable SIZE    m_TableSize;
        mutable Rect_t  m_TableRect;
        mutable Rect_t  m_ClientRect;

    public:

        TableWindow_t(
                  Ui::WindowId_t      WindowId,
            const Ui::Window_t&       parent,
            const wchar_t*            pWindowName,
                  Flag_t              Flags,
            const Ui::Widget::Data_t* pWidgets,
                  size_t              WidgetCount,
                  POINT               TableOffset,
            const RECT&               InnerTableRect,
                  POINT               TabOffset);

        //
        // Ui::Window_t virtual:
        //

        Ui::WindowId_t
        GetWindowId(
            const CSurface& Surface,
            const POINT*    pptHint) const override;

        bool
        GetWidgetRect(
            Ui::WidgetId_t  WidgetId,
                  Rect_t&   WidgetRect) const override;

        bool
        UpdateScrollPosition(
            Ui::Scroll::Bar_t ScrollBar,
            const CSurface&   Surface) override;

        //
        // TableWindow_t virtual:
        //

        void
        GetScrollOffsets(
            const CSurface& Surface,
            const Rect_t&   TableRect,
                  SIZE&     ScrollOffsets) const;

        //
        // Public methods:
        //

        const Rect_t&
        GetTableRect() const
        {
            return m_TableRect;
        }

        const Rect_t&
        GetClientRect() const
        {
            return m_ClientRect;
        }

        bool
        ClickRow(
            size_t Row) const;

        void
        SetTableOffset(POINT offset)
        {
            m_TableOffset = offset;
        }

        void
        SetOffsets(POINT tabOffset, POINT tableOffset)
        {
            SetTabOffset(tabOffset);
            SetTableOffset(tableOffset);
        }

    private:

        //
        // Private methods:
        //

        bool
        FindTable(
            const CSurface& Surface,
            const POINT&    ptTab,
                  Rect_t&   TableRect,
                  SIZE&     ScrollOffsets) const;

        void
        GetClientRect(
                  Rect_t& ClientRect,
            const Rect_t& TableRect,
            const SIZE&   ScrollOffsets) const;

        bool
        ValidateTable(
            const CSurface& Surface,
            const Rect_t&   TableRect,
                  SIZE&     ScrollOffsets) const;

        bool
        ValidateBorders(
            const CSurface& Surface,
            const Rect_t&   TableRect) const;

        bool
        ValidateBorder(
            const CSurface& Surface,
            const Rect_t&   BorderRect,
            const wchar_t*  pBorderName) const;

        bool
        ValidateClient(
            const CSurface& Surface,
            const Rect_t&   TableRect,
                  SIZE&     ScrollOffsets) const;
    };

////////////////////////////////////////////////////////////////////////////////

