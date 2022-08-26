/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// DcrBrokerSell.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_DCRBROKERSELL_H
#define Include_DCRBROKERSELL_H

#include "DcrWindow.h"
#include "DcrBase_t.h"
#include "BrokerSellTypes.h"
#include "Macros.h"
#include "Rect.h"
#include "BrokerId.h"
#include "BrokerUi.h"
#include "UiTypes.h"

namespace Broker
{
namespace Sell
{
namespace Translate
{

////////////////////////////////////////////////////////////////////////////////

    struct Data_t :
        public DP::Message::Data_t
    {
        typedef Sell::Text_t Text_t;
        Text_t                 Text;
        size_t                 selectedRow;
        Ui::Scroll::Position_t VScrollPos;

        Data_t(
            const wchar_t*         pClass,
            const TextTable_t&     TextTable,
                  size_t           initSelectedRow,
            Ui::Scroll::Position_t InitVScrollPos)
        :
            DP::Message::Data_t(
                DP::Stage::Translate,
                Message::Id::Sell,
                sizeof(Data_t),
                pClass),
            Text(TextTable.GetData()),
            selectedRow(initSelectedRow),
            VScrollPos(InitVScrollPos)
        { }

    private:

        Data_t();
    };

////////////////////////////////////////////////////////////////////////////////

    typedef DcrWindow::Policy::TranslateMany_t    TranslatePolicy_t;
    typedef DcrWindow::Policy::NoValidate_t       ValidatePolicy_t;

    typedef DcrWindow::Translate::Handler_t<
                TranslatePolicy_t, ValidatePolicy_t> HandlerBase_t;

    class Handler_t :
        public HandlerBase_t
    {
        friend struct Translate::Data_t; 

    public:

        static const ScreenTable_t          s_ScreenTable;
 
    private:

        Window::ManagerBase_t& m_Manager;
        TranslatePolicy_t      m_TranslatePolicy;
        ValidatePolicy_t       m_ValidatePolicy;
        DcrVector_t            m_DcrVector;
        DcrBase_t              m_DcrTable;
        TextTable_t            m_TextTable;

    public:

        Handler_t(
            Window::ManagerBase_t& Manager);

        //
        // Handler_t virtual:
        //

        virtual 
        void
        PostData(
            DWORD /*Unused*/) override;

    public:

        const Text_t&             GetText() const  { return m_TextTable.GetData(); }

        const DcrBase_t&          GetDcr() const   { return m_DcrTable; }
        DcrBase_t&                GetDcr()         { return m_DcrTable; }

    private:

        Window::ManagerBase_t& GetManager() const { return m_Manager; }

    private:

        Handler_t();
        Handler_t(const Handler_t&);
        const Handler_t& operator=(const Handler_t&);
    };

/////////////////////////////////////////////////////////////////////////////

} // Translate
} // Sell
} // Broker

#endif // INCLUDE_DCRBROKERSELL_H

/////////////////////////////////////////////////////////////////////////////
