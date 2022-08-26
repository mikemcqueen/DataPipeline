/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DcrSetPrice.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_DCRSETPRICE_H
#define Include_DCRSETPRICE_H

/////////////////////////////////////////////////////////////////////////////

#include "DcrWindow.h"

#include "SetPriceTypes.h"
#include "Macros.h"
#include "Rect.h"
#include "BrokerId.h"
#include "BrokerUi.h"
#include "DcrRect_t.h"

namespace Broker
{
namespace SetPrice
{
namespace Translate
{

////////////////////////////////////////////////////////////////////////////////

    struct Data_t :
        public DP::Message::Data_t
    {
        size_t Price;

        Data_t(
            const wchar_t* pClass,
            const size_t   InitPrice)
        :
            DP::Message::Data_t(
                DP::Stage::Translate,
                Message::Id::SetPrice,
                sizeof(Data_t),
                pClass),
            Price(InitPrice)
        { }

    private:

        Data_t();
    };

////////////////////////////////////////////////////////////////////////////////

    typedef SsWindow::Acquire::Data_t             AcquireData_t;
    typedef DcrWindow::Policy::TranslateMany_t    TranslatePolicy_t;
    typedef DcrWindow::Policy::NoValidate_t       ValidatePolicy_t;

    typedef DcrWindow::Translate::Handler_t<
                TranslatePolicy_t,
                ValidatePolicy_t>                 HandlerBase_t;

    class Handler_t :
        public HandlerBase_t
    {
        friend struct Translate::Data_t; 

    public:

        //static const Ui::WindowId_t         DcrWindowId      = Broker::Window::Id::SetPrice_Main;

    private:

        Window::ManagerBase_t&  m_Manager;

        TranslatePolicy_t m_TranslatePolicy;
        ValidatePolicy_t  m_ValidatePolicy;

        DcrVector_t       m_DcrVector;
        DcrRect_t         m_DcrPrice;

    public:

        Handler_t(
            Window::ManagerBase_t& Manager);

        //
        // DcrWindow virtual:
        //

        virtual 
        void
        PostData(
            DWORD /*Unused*/) override;

    private:

        Handler_t();
        Handler_t(const Handler_t&);
        const Handler_t& operator=(const Handler_t&);
    };


/////////////////////////////////////////////////////////////////////////////

} // Translate
} // SetPrice
} // Broker

#endif // Include_DCRSETPRICE_H

/////////////////////////////////////////////////////////////////////////////
