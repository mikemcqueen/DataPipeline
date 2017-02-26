/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TiConfirmTrade.h
//
// LoN Posted Trades Window text interpreter.
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TICONFIRMTRADE_H
#define Include_TICONFIRMTRADE_H

/////////////////////////////////////////////////////////////////////////////

#include "DpHandler_t.h"
#include "DpTransaction.h"
#include "TiBase_t.h" // TODO: TiTable_t.h
#include "TradePoster.h"
#include "AutoCs.h"
#include "ConfirmTradeTypes.h"
#include "LonWindow_t.h"
#include "LonTypes.h"

namespace ConfirmTrade
{
namespace Interpret
{

/////////////////////////////////////////////////////////////////////////////

class Handler_t : 
    public DP::Handler_t
{

private:

    typedef TiBase_t<
                Table::LineCount,
                Table::CharsPerLine,
                Table::ColumnCount,
                Table::LineHeight,
                Table::CharHeight>  TiTable_t;

    enum Column_e
    {
        Quantity,
        CardName
    };

private:

    ManagerBase_t& m_Manager;
    Trade_t      m_Trade;          // the trade we build using translated data
    Trade_t      m_TradeCompare;   // the trade sent to us to build a copy of 
    bool         m_bDone;
    TiTable_t    m_TiOffered;
    TiTable_t    m_TiWant;
    mutable
    CAutoCritSec m_csState;

public:

    Handler_t(
        ManagerBase_t& Manager);

    //
    // DP::Handler_t virtual:
    //

    virtual
    HRESULT
    MessageHandler(
        const DP::Message::Data_t* pData);

    virtual
    HRESULT
    ExecuteTransaction(
        const DP::Transaction::Data_t& Data);

    virtual
    HRESULT
    OnTransactionComplete(
        const DP::Transaction::Data_t& Data);


    // Helpers

    void
    ConfirmTrade(
       const TradePoster::EventPost_t::Data_t& Data);

    void
    InterpretSameText(
              Lon::Window::Type_e WindowType,
        const TiTable_t::Text_t&  Text);

    void
    InterpretNewText(
              Lon::Window::Type_e WindowType,
        const TiTable_t::Text_t&  Text,
              size_t              FirstNewLine);

private:

    bool
    CanScroll(
        Lon::Window::Type_e WindowType);

	bool
    InterpretLine(
        const wchar_t* pszText);

    void
    AddTradeLine(
              Trade_t& Trade,
        const wchar_t* pszText);

    bool
    AddTradeItem(
        CardCollection_t& Cards,
        const wchar_t*    pszText);

    const wchar_t*
    ParseItemName(
        const wchar_t* pszText);

    bool
    GetMoreData(
        Lon::Window::Type_e Type);

    bool
    ScrollToTrade(
        DWORD id);

    void
    Scroll(
        bool bScrollOffered,
        bool bScrollWant);

    void
    Scroll(
        Lon::Window::Type_e                WindowType,
        LonWindow_t::EventScroll_t::Type_e ScrollType,
        size_t                             Count = 1);

private:

    Handler_t();
    Handler_t(const Handler_t&);
    Handler_t& operator= (const Handler_t&);
};

} // Interpret
} // ConfirmTrade

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TICONFIRMTRADE_H

/////////////////////////////////////////////////////////////////////////////
