////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// Eq2LoginWindow.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UiWindow.h"
#include "BrokerUi.h"
#include "DdUtil.h"

namespace Broker
{
namespace Eq2Login
{

////////////////////////////////////////////////////////////////////////////////

    class Window_t :
        public Ui::Window_t
    {
    private:

        CSurface  m_connectButton;

    public:

        Window_t(
        const Ui::Window_t& parent);

        //
        // Ui::Window_t virtual:
        //
 
        bool
        GetWidgetRect(
            Ui::WidgetId_t  WidgetId,
                  Rect_t&   WidgetRect) const override;

        const CSurface*
        GetOriginSurface() const override;

        void
        GetOriginSearchRect(
            const CSurface& surface,
                  Rect_t&   rect) const override;

    private:

        void
        loadSurfaces();

    private:

        Window_t();
        Window_t(const Window_t&);
        const Window_t& operator=(const Window_t&);
    };

////////////////////////////////////////////////////////////////////////////////

} // Eq2Login
} // Broker

////////////////////////////////////////////////////////////////////////////////
