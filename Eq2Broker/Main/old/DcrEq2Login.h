/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// DcrEq2Login.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_DCREQ2LOGIN_H
#define Include_DCREQ2LOGIN_H

#include "DcrWindow.h"
#include "Eq2LoginTypes.h"
#include "Macros.h"
#include "DcrRect_t.h"
#include "BrokerId.h"
#include "BrokerUi.h"

namespace Broker
{
namespace Eq2Login
{
namespace Translate
{

////////////////////////////////////////////////////////////////////////////////

    struct Data_t :
        public DP::Message::Data_t
    {
        static const size_t kCharacterTextMax  = 30;
        static const size_t kServerTextMax     = 30;
        wchar_t characterName[kCharacterTextMax];
        wchar_t serverName[kServerTextMax];
        struct CharacterButton_t
        {
            wchar_t text[kCharacterTextMax];
        };
        CharacterButton_t characterButtons[kCharacterButtonCount];

        Data_t(
            const wchar_t*       pClass,
            const wstring&       initCharacterName,
            const wstring&       initServerName,
            CharacterButton_t*   pCharacterButtons)
        :
            DP::Message::Data_t(
                DP::Stage::Translate,
                Message::Id::Eq2Login,
                sizeof(Data_t),
                pClass)
        {
            wcscpy_s(characterName, initCharacterName.c_str());
            wcscpy_s(serverName, initServerName.c_str());
            memcpy(&characterButtons[0], pCharacterButtons, sizeof(characterButtons));
        }

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

        static const Ui::WindowId_t TopWindowId  = Broker::Window::Id::Eq2Login;

    private:

        Broker::Eq2Login::Window::ManagerBase_t& m_windowManager;

        TranslatePolicy_t m_TranslatePolicy;
        ValidatePolicy_t  m_ValidatePolicy;
        DcrVector_t       m_DcrVector;
        DcrRect_t         m_dcrCharacterEdit;
        DcrRect_t         m_dcrServerEdit;
        vector<shared_ptr<DcrRect_t>>  m_dcrCharacterButtons;

    public:

        Handler_t(
            Window::ManagerBase_t& windowManager);

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
        Handler_t& operator=(const Handler_t&);
    };

/////////////////////////////////////////////////////////////////////////////

} // Translate
} // Eq2Login
} // Broker

#endif // Include_DCREQ2LOGIN_H

/////////////////////////////////////////////////////////////////////////////
