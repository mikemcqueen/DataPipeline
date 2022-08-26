////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// Eq2LoginTypes.h
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_EQ2LOGINTYPES_H
#define Include_EQ2LOGINTYPES_H

#include "UiWindowManager.h"
#include "BrokerUi.h"

////////////////////////////////////////////////////////////////////////////////

namespace Broker
{
namespace Eq2Login
{
    static const Ui::WindowId_t TopWindowId = Broker::Window::Id::Eq2Login;

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

    namespace Interpret
    {
        class Handler_t
        {
        public:
            Handler_t(Window::ManagerBase_t&) {}
        };
    } // Interpret

} // Eq2Login
} // Broker

////////////////////////////////////////////////////////////////////////////////

#endif // Include_EQ2LOGINTYPES_H

////////////////////////////////////////////////////////////////////////////////
