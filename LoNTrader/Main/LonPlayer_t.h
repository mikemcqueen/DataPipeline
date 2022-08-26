///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// LonPlayer_t.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_LONPLAYER_T_H_
#define Include_LONPLAYER_T_H_

#include "DpHandler_t.h"
#include "TransactionManager.h"
//#include "DpTransaction.h"
#include "Trade_t.h"
#include "LonMessageTypes.h"
#include "Timer_t.h"

///////////////////////////////////////////////////////////////////////////////

class LonPlayer_t :
    public DP::Handler_t
{

public:

    struct EventGetYourCards_t
    {
        struct Data_t :
            public Lon::Transaction::Data_t
        {
            CardCollection_t YourCards;

            Data_t() :
               Lon::Transaction::Data_t(
                    Lon::Transaction::Id::GetYourCards,
                    Lon::Window::PostedTradesWindow,
                    sizeof(Data_t))
            { }
        } m_Data;
    };

private:

    std::wstring     m_strName;
    CardCollection_t m_YourCards;

public:

    mutable Timer_t  m_Timer;

    LonPlayer_t();

    //
    // DP::Handler_t override:
    //

    HRESULT
    MessageHandler(
        const DP::Message::Data_t* pData) override;

    HRESULT
    OnTransactionComplete(
        const DP::Transaction::Data_t& Data) override;

    // Accessors:

    const CardCollection_t& GetYourCards() const          { return m_YourCards; }

    const wchar_t*          GetName() const               { return m_strName.c_str(); }
    void                    SetName(const wchar_t* pName) { m_strName.assign(pName); }

    // Helpers:

    bool
    DoGetYourCards() const;

    bool
    ReadYourCards();

    bool
    WriteYourCards();

    void
    ShowYourCards(
        const Flag_t Flags,
              size_t Order = 0) const;

    bool
    CompareName(
        const wchar_t* pszName)
    {
        return 0 == m_strName.compare(pszName);
    }

private:

    void
    OnGetYourCardsComplete(
        const EventGetYourCards_t::Data_t& Data);

    void
    OnRemoveTrade(
        TradeId_t TradeId);

private:

    LonPlayer_t(const LonPlayer_t&) = delete;
    LonPlayer_t& operator=(const LonPlayer_t&) = delete;

};

///////////////////////////////////////////////////////////////////////////////

#endif // Include_LONPLAYER_T_H_

///////////////////////////////////////////////////////////////////////////////
