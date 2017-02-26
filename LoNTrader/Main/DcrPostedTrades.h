/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DcrPostedTrades.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_DCRPOSTEDTRADES_H
#define Include_DCRPOSTEDTRADES_H

/////////////////////////////////////////////////////////////////////////////

#include "TrWindow_t.h"
#include "DcrTrades_t.h"
#include "TextTable_t.h"
#include "LonTypes.h"
#include "PostedTradesTypes.h"

namespace PostedTrades
{
namespace Translate
{

/////////////////////////////////////////////////////////////////////////////

    class Handler_t :
        public TrWindow_t<OneTablePolicy_t>
    {
        friend struct Translate::Data_t;

        static const Lon::Window::Type_e TopWindowType = Lon::Window::PostedTradesWindow;
        static const Lon::Window::Type_e DcrWindowType = Lon::Window::PostedTradesView;

    private:

        OneTablePolicy_t   m_TranslatePolicy;
        NoValidatePolicy_t m_ValidatePolicy;

        DcrTrades_t   m_Dcr;
        Table::Text_t m_Text;
        RECT          m_rcBounds;

    public:

        Handler_t();

        //
        // TrBase_t virtual:
        //

        virtual 
        void
        PostData(
            DWORD /*Unused*/);

    public:

        const Table::TextData_t&  GetText() const  { return m_Text.GetData(); }

        const DcrTrades_t&        GetDcr() const   { return m_Dcr; }
        DcrTrades_t&              GetDcr()         { return m_Dcr; }

    };

/////////////////////////////////////////////////////////////////////////////

} // Translate
} // PostedTrades

#endif // Include_DCRPOSTEDTRADES_H

/////////////////////////////////////////////////////////////////////////////
