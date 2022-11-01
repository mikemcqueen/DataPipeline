////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// BrokerBuyWindow.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef INCLUDE_BROKERBUYWINDOW_H
#define INCLUDE_BROKERBUYWINDOW_H

#include "TabWindow.h"
#include "BrokerUi.h"

namespace Broker::Buy {

class Window_t :
    public TableWindow_t
{
public:
    explicit
    Window_t(
        const Ui::Window_t& parent);

    static
    const Ui::Widget::Data_t&
    GetWidgetData(
        Ui::WidgetId_t widgetId);

    bool
    GetWidgetRect(
        Ui::WidgetId_t widgetId,
        Rect_t* pRect) const override;

    void
    SetLayout(Frame::Layout_t layout);

private:

    MainWindow_t&
    GetMainWindow() const;

    Window_t();
    Window_t(const Window_t&);
    Window_t& operator=(const Window_t&);
};

} // Broker::Buy

#endif // INCLUDE_BROKERBUYWINDOW_H
