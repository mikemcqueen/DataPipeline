///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TiPostedTrades.h
//
// Text Interpreter for LoN Posted Trades window.
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TIPOSTEDTRADES_H
#define Include_TIPOSTEDTRADES_H

#include "DpHandler_t.h"
#include "DpTransaction.h"
#include "TiBase_t.h" // TODO: TiTable_t.h
#include "Trade_t.h"
#include "EventTypes.h"
#include "LonWindow_t.h"
#include "PostedTradesPolicy.h"
#include "PostedTradesTypes.h"
#include "TradeMap_t.h"

///////////////////////////////////////////////////////////////////////////////

class Trade_t;

namespace PostedTrades
{
namespace Interpret
{

///////////////////////////////////////////////////////////////////////////////

class Handler_t :
    public DP::Handler_t
{

private:

        typedef TiBase_t<
                    Table::LineCount,
                    Table::CharsPerLine,
                    Table::ColumnCount,
                    Table::LineHeight,
                    Table::CharHeight>  TiTrades_t;

    enum State_e
    {
        FindNewTrade,  // find id > m_idLastTrade
        BuildTrade,    // build m_Trade.id
        FindTrade,     // find m_Trade.id
    };

    enum Column_e
    {
        Id,
        User,
        Offer,
        Want
    };

    static const size_t PageLines = 4;

private:

    DefaultPolicy_t      m_DefaultPolicy;
    BuyTradePolicy_t     m_BuyTradePolicy;
    GetYourCardsPolicy_t m_GetYourCardsPolicy;
    PostTradePolicy_t    m_PostTradePolicy;
    RemoveTradePolicy_t  m_RemoveTradePolicy;
    GatherTradesPolicy_t m_GatherTradesPolicy;

    ManagerBase_t&       m_Manager;

    TiTrades_t           m_TiTrades;
    Trade_t              m_Trade;

    State_e              m_State;
    mutable
    CAutoCritSec         m_csState;

    TradeId_t            m_idLastTrade;
    TradeId_t            m_idFindTrade;
    TradeIdSet_t         m_MissedTrades;

    mutable
    LonWindow_t::EventScroll_t::Type_e m_LastScrollType;
    LonWindow_t::EventScroll_t::Type_e m_ScrollType;

public:

    Handler_t(
        ManagerBase_t& Manager);

protected:

    //
    // DP::Handler_t virtual:
    //

    HRESULT
    MessageHandler(
        const DP::Message::Data_t* pData) override;

    HRESULT
    ExecuteTransaction(
        const DP::Transaction::Data_t& Data) override;
 
public:

    void
    InterpretSameText(
              Lon::Window::Type_e WindowType,
        const TiTrades_t::Text_t& Text);

    void
    InterpretNewText(
              Lon::Window::Type_e WindowType,
        const TiTrades_t::Text_t& Text,
              size_t              FirstNewLine);

    // Helpers

    const ManagerBase_t& GetManager() const  { return m_Manager; }
    ManagerBase_t&       GetManager()        { return m_Manager; }

    void
    ShowMissedTradeNumbers() const;

    void
    ShowState(
        const wchar_t* pszText = NULL,
              bool     bAlways = false) const;

private:

    const Policy_t&
    GetPolicy() const;

    const Policy_t&
    GetPolicy(
       const DP::Transaction::Data_t* pData) const;

    void
    SetState(
        State_e State);

	bool
    InterpretNewLine(
        const wchar_t* pszText);

    TradeId_t
    GetTradeId(
        const wchar_t* pszText) const;

    void
    MaybeSkipTrade(
        size_t FirstNewLine);

    bool
    StartNewTrade(
              Trade_t& trade,
        const wchar_t* pszText);

    void
    AddTrade(
        Trade_t& trade);

    bool
    AddTradeLine(
              Trade_t& trade,
        const wchar_t* pszText);

    bool
    AddTradeItem(
              CardCollection_t& Cards,
        const wchar_t*          pszText,
              size_t            TextColumn);

    const wchar_t*
    ParseItemName(
        const wchar_t* pszText,
              int&     Count);

    void
    GetMoreData(
        const RECT& rcBounds);

    bool
    FindTradeInText(
              TradeId_t           TradeId,
        const TiTrades_t::Text_t& Text,
        const RECT&               rcBounds,
              RECT&               rcTrade) const;

    bool
    ScrollToMissedTrade();

    bool
    ScrollToTrade(
        TradeId_t TradeId) const;

    bool
    Scroll(
        LonWindow_t::EventScroll_t::Type_e Type,
        size_t                             Count = 1) const;

    void
    RemoveMissedTrade(
        TradeId_t TradeId);

    TradeId_t
    GetFirstTradeId(
        const TiTrades_t::Text_t& Text) const;

    TradeId_t
    GetLastTradeId(
        const TiTrades_t::Text_t& Text) const;

private:

    Handler_t();
    Handler_t(const Handler_t&);
    Handler_t& operator= (const Handler_t&);
};

} // Interpret
} // PostedTrades

///////////////////////////////////////////////////////////////////////////////

#endif // Include_TIPOSTEDTRADES_H

///////////////////////////////////////////////////////////////////////////////
