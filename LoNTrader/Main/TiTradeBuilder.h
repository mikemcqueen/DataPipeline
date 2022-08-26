///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TiTradeBuilder.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TITRADEBUILDER_H
#define Include_TITRADEBUILDER_H

#include "DpHandler_t.h"
#include "DpTransaction.h"
#include "TiBase_t.h" // TODO: TiTable_t.h
#include "LonCard_t.h"
#include "TradePoster.h"
#include "LonPlayer_t.h"
#include "TradeBuilderTypes.h"

///////////////////////////////////////////////////////////////////////////////

namespace TradeBuilder
{
namespace Interpret
{

///////////////////////////////////////////////////////////////////////////////

class Handler_t :
    public DP::Handler_t
{

private:

    enum State_e
    {
        Ready              = 0,
        ClearTrade,
        SelectCollection,
        ClearCardName,
        EnterCardName,
        ScrollTable,       // 5
        AddCard,
        SubmitTrade,
        Waiting,
        Error,
        Done,              // 10
    };

    typedef TiBase_t<
                Table::LineCount,
                Table::CharsPerLine,
                Table::MaxColumnCount,
                Table::LineHeightPixels,
                Table::CharHeightPixels> TiTable_t;

    struct Their
    {
        enum Column_e
        {
            TheyHave,
            YouHave,
            YouWant,
            Title
        };
    };

    struct Your
    {
        static const size_t ColumnOffset = 2;
        enum Column_e
        {
/*
            Quantity,
            Want,
*/
            Trade,
            Need,
            Title,
            Type,
            Set,
            Number,
            Rarity
        };
    };

    static const size_t CollectionTabDivisor = 3;
    static const size_t ScrollPageLines      = 2;

private:

    ManagerBase_t& m_Manager;

    TiTable_t     m_TiTable;

    mutable
    Trade_t       m_Trade;                 // for PostTrade, elements from this are removed
                                           // as the trade is built in UI
    size_t        m_StateTimeout;
    State_e       m_State;
    mutable
    CAutoCritSec  m_csState;

    RECT          m_rcBounds;

    Collection_e  m_WhichCollection;

    bool          m_bExitBuilder;         // Force builder exit; test setting
    bool          m_bSwitched;            // We've switched from "Theirs" to "Yours"
    mutable
    bool          m_bScrolledYours;       // "Your cards" collection has been horz scrolled

public:

    Handler_t(
        ManagerBase_t& Manager);

protected:

    //
    // DP::Handler_t override:
    //

    HRESULT
    MessageHandler(
        const DP::Message::Data_t* pData) override;

    HRESULT
    ExecuteTransaction(
        const DP::Transaction::Data_t& Data) override;

    HRESULT
    OnTransactionComplete(
        const DP::Transaction::Data_t& Data) override;

    const ManagerBase_t& GetManager() const  { return m_Manager; }
    ManagerBase_t&       GetManager()        { return m_Manager; }

private:

    void
    SetState(State_e State);

    bool
    GetYourCards();

    bool
    PostNewTrade(
        const TradePoster::EventPost_t::Data_t& Data);

    void
    InterpretGetYourCards(
        const Translate::Data_t*                  pBuilderData,
        LonPlayer_t::EventGetYourCards_t::Data_t& TransactionData);

    void
    InterpretPostTrade(
        const Translate::Data_t*           pBuilderData,
        TradePoster::EventPost_t::Data_t& TransactionData);

    void
    AddCardsToCollection(
        const TiTable_t::Text_t& Text,
              size_t             FirstNewLine);

    void
    OnPostTradeError(
        DWORD Error) const;

    bool
    HScrollTable() const;

    void
    VScrollTable() const;

    Lon::Window::Type_e
    GetVScrollType() const;

    bool
    DoClearTrade();

    CardCollection_t&
    GetCardCollection(
        Collection_e CollectionType);

    bool
    SwitchCollection();

    bool
    DoClickCollection(
        Collection_e CollectionType);

    bool
    ClickCollection(
        Collection_e CollectionType);

    bool
    DoClearCardName();

    bool
    DoEnterCardName();

    bool
    DoAddCardToTrade(
        const RECT& rcTable,
              bool  bForce = false);

    bool
    DoSubmitTrade();

    TradeId_t
    GetTradeId(
        const wchar_t* pszText) const;

    void
    NameFixup(
        std::wstring&       strName,
        LonCard_t::Number_t Number) const;

    bool
    FindCardInText(
        const LonCard_t*         pLonCard,
        const TiTable_t::Text_t& Text,
              size_t&            Line) const;

    size_t
    GetTitleColumnOffset(
        const TiTable_t::Text_t& Text) const;

private:

    Handler_t();
    Handler_t(const Handler_t&);
    Handler_t& operator=(const Handler_t&);
};

} // Interpret
} // TradeBuilder

///////////////////////////////////////////////////////////////////////////////

#endif // Include_TITRADEBUILDER_H

///////////////////////////////////////////////////////////////////////////////
