/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DcrTradeBuilder.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_DCRTRADEBUILDER_H
#define Include_DCRTRADEBUILDER_H

#include "TrWindow_t.h"
#include "DcrTrades_t.h"
#include "AutoCs.h"
#include "TradeBuilderTypes.h"

/////////////////////////////////////////////////////////////////////////////

namespace TradeBuilder
{
namespace Translate
{

    /////////////////////////////////////////////////////////////////////////////

    struct Data_t :
        public Lon::Message::Data_t
    {
        Table::Text_t /*::Data_t*/ Text;  // NOTE removed Data_t to make it compile
        RECT                  Rect;

        void
        Initialize(
            const Handler_t* pClass);

    private:

        Data_t();
    };

    /////////////////////////////////////////////////////////////////////////////

    class Handler_t;
    typedef OneDynamicTablePolicy_t<Handler_t> Policy_t;

    class Handler_t :
        public TrWindow_t<Policy_t>
    {
        friend struct Translate::Data_t;

    private:

        static const Lon::Window::Type_e DcrYourWindowType  = Lon::Window::TradeBuilderYourTableView;
        static const Lon::Window::Type_e DcrTheirWindowType = Lon::Window::TradeBuilderTheirTableView;

    private:

        Policy_t           m_TranslatePolicy;
        NoValidatePolicy_t m_ValidatePolicy;

        Lon::Window::Type_e m_DcrWindowType;
        DcrTrades_t        m_Dcr;
        Table::Text_t      m_Text;
        RECT               m_rcBounds;
        CAutoCritSec       m_csState;

    public:

        Handler_t();

        void
        SetCollection(
            const Collection_e WhichCollection);

        Lon::Window::Type_e
        GetDcrWindowType() const;

        //
        // TrSurface_t virtual:
        //

        virtual 
        void
        PostData(
            DWORD AcquireId);

    private:

        Handler_t(const Handler_t&);
        Handler_t& operator=(const Handler_t&);
    };

    /////////////////////////////////////////////////////////////////////////////

} // Translate
} // TradeBuilder

#endif // Include_DCRTRADEBUILDER_H

/////////////////////////////////////////////////////////////////////////////
