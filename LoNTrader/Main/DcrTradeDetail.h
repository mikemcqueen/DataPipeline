/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DcrTradeDetail.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_DCRTRADEDETAIL_H
#define Include_DCRTRADEDETAIL_H

#include "TrWindow_t.h"
#include "TradeDetailTypes.h"
#include "DcrTrades_t.h"

/////////////////////////////////////////////////////////////////////////////

namespace TradeDetail
{
namespace Translate
{

    /////////////////////////////////////////////////////////////////////////

    struct Data_t :
        public Lon::Message::Data_t
    {
        Table::Text_t/*::Data_t*/ WantText;   // NOTE removed ::Data_t in both of these to get it to compile
        Table::Text_t/*::Data_t*/ OfferedText;

        void
        Initialize(
            const wchar_t* pszClass,
            const Table::Text_t& Offered,
            const Table::Text_t& Want);

    private:

        Data_t();
    };

    /////////////////////////////////////////////////////////////////////////

    typedef TwoTablePolicy_t       TranslatePolicy_t;
    typedef ValidateWindowPolicy_t ValidatePolicy_t;

    class Handler_t :
        public TrWindow_t<
            TranslatePolicy_t,
            ValidatePolicy_t>
    {

    private:

        static const Lon::Window::Type_e DcrOfferedWindowType = Lon::Window::PostedTradeDetailOfferedView;
        static const Lon::Window::Type_e DcrWantWindowType    = Lon::Window::PostedTradeDetailWantView;

    private:

        TranslatePolicy_t m_TranslatePolicy;
        ValidatePolicy_t  m_ValidatePolicy;

        DcrTrades_t   m_DcrOffered;
        Table::Text_t m_OfferedText;

        DcrTrades_t   m_DcrWant;
        Table::Text_t m_WantText;

    public:

        Handler_t();

        //
        // TrSurface_t virtual:
        //

        virtual 
        void
        PostData(
            DWORD AcquireId);

    };

} // Translate
} // TradeDetail

/////////////////////////////////////////////////////////////////////////////

#endif // Include_DCRTRADEDETAIL_T_H

/////////////////////////////////////////////////////////////////////////////
