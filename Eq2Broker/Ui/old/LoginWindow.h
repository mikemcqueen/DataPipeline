////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// LoginWindow.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UiWindow.h"
#include "BrokerUi.h"
#include "DdUtil.h"

namespace Broker
{
namespace Login
{

////////////////////////////////////////////////////////////////////////////////

    class Window_t :
        public Ui::Window_t
    {
    private:

        CSurface  m_loginCaption;

    public:

//        explicit
        Window_t(
            const Ui::Window_t& parent);

        //
        // Ui::Window_t virtual:
        //
        virtual
        bool
        IsLocatedOn(
            const CSurface& Surface,
                  Flag_t    flags,
                  POINT*    pptOrigin = NULL) const override;

    private:

        void
        loadSurfaces();

    private:

        Window_t();
        Window_t(const Window_t&);
        const Window_t& operator=(const Window_t&);
    };

////////////////////////////////////////////////////////////////////////////////

} // Login
} // Broker

////////////////////////////////////////////////////////////////////////////////
