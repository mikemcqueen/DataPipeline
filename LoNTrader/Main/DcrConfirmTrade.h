/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DcrConfirmTrade.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_DCRCONFIRMTRADE_H
#define Include_DCRCONFIRMTRADE_H

#include "TrWindow_t.h"
#include "DcrTrades_t.h"
#include "ConfirmTradeTypes.h"

/////////////////////////////////////////////////////////////////////////////

namespace ConfirmTrade
{
namespace Translate
{

    /////////////////////////////////////////////////////////////////////////

    struct Data_t :
        public Lon::Message::Data_t
    {
        Table::Text_t/*::Data_t*/ OfferedText; // NOTE removed ::Data_t from these two lines to get it compiling
        Table::Text_t/*::Data_t*/ WantText;

        // TODO: constructor takes params, operator new (offset) 
        void
        Initialize(
            const wchar_t* pszClass,
            const Table::Text_t& Offered,
            const Table::Text_t& Want);

        //    private:
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

        static const Lon::Window::Type_e DcrOfferedWindowType = Lon::Window::ConfirmTradeTheyGetView;
        static const Lon::Window::Type_e DcrWantWindowType    = Lon::Window::ConfirmTradeYouGetView;

    private:

        TranslatePolicy_t m_TranslatePolicy;
        ValidatePolicy_t  m_ValidatePolicy;

        DcrTrades_t       m_DcrOffered;
        Table::Text_t     m_OfferedText;

        DcrTrades_t       m_DcrWant;
        Table::Text_t     m_WantText;

        // TODO: hacky:
        size_t            m_OfferedRequired;
        size_t            m_WantRequired;

    public:

        Handler_t();

        void
        SetRequired(
            size_t Offered,
            size_t Want)
        {
            m_OfferedRequired = min(Offered, Table::LineCount);
            m_WantRequired = min(Want, Table::LineCount);
        }

        // TODO: ExecuteTransaction() ? to call SetRequired (or otherwise
        // support "we need this many lines of text visible" startup case.

        //
        // TrSurface_t virtual:
        //

        virtual 
        void
        PostData(
            DWORD AcquireId);

    private:

        Handler_t(const Handler_t&);
        Handler_t& operator= (const Handler_t&);
    };

} // Translate
} // ConfirmTrade

/////////////////////////////////////////////////////////////////////////////

#endif // Include_DCRCONFIRMTRADE_H

/////////////////////////////////////////////////////////////////////////////
