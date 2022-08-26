////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// SetPriceWindow.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UiWindow.h"
#include "BrokerUi.h"

namespace Broker
{
namespace SetPrice
{

////////////////////////////////////////////////////////////////////////////////

    class Window_t :
        public Ui::Window_t
    {
    public:

        explicit
        Window_t(
            const Ui::Window_t& parent);

        //
        // Ui::Window_t virtual:
        //
        virtual
        bool
        GetWidgetRect(
            Ui::WidgetId_t WidgetId,
            Rect_t&        WidgetRect) const override;

        bool
        FindBorder(
            const CSurface& Surface,
            const POINT&    ptTab,
                  Rect_t&   SurfaceRect) const;

    private:

        Window_t();
        Window_t(const Window_t&);
        Window_t& operator=(const Window_t&);
    };

////////////////////////////////////////////////////////////////////////////////

} // SetPrice
} // Broker

////////////////////////////////////////////////////////////////////////////////
