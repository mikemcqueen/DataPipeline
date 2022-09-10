////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// BrokerBuyWindow.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "TabWindow.h"
#include "BrokerUi.h"

namespace Broker
{
namespace Buy
{

////////////////////////////////////////////////////////////////////////////////

    class Window_t :
        public TableWindow_t
    {
    public:

        static
        const Ui::Widget::Data_t&
        GetWidgetData(
            Ui::WidgetId_t widgetId);

    public:

        explicit
        Window_t(
            const Ui::Window_t& parent);

        bool
        GetWidgetRect(
            Ui::WidgetId_t widgetId,
            Rect_t&        rect) const override;

        void
        SetLayout(Frame::Layout_t layout);

    private:

        MainWindow_t&
        GetMainWindow() const;

    private:

        Window_t();
        Window_t(const Window_t&);
        Window_t& operator=(const Window_t&);
    };

////////////////////////////////////////////////////////////////////////////////

} // Buy
} // Broker

////////////////////////////////////////////////////////////////////////////////
