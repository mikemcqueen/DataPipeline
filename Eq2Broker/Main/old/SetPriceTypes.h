///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// SetPriceTypes.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_SETPRICETYPES_H
#define Include_SETPRICETYPES_H

#include "UiWindowManager.h"
#include "BrokerUi.h"

///////////////////////////////////////////////////////////////////////////////

namespace Broker
{
namespace SetPrice
{
    static const Ui::WindowId_t TopWindowId = Broker::Window::Id::BrokerSetPricePopup;

    namespace Translate
    {
        struct Data_t;
        class Handler_t;
    } // Translate

    namespace Interpret
    {
        struct Data_t;
        class Handler_t;
    } // Interpret

    class Window_t;
    namespace Window
    {
        typedef Ui::Window::Manager_t<
                    Window_t,
                    Translate::Handler_t,
                    Interpret::Handler_t> ManagerBase_t;
        class Manager_t;
    } // Window

} // SetPrice
} // Broker

///////////////////////////////////////////////////////////////////////////////

#endif // Include_SETPRICETYPES_H

///////////////////////////////////////////////////////////////////////////////
