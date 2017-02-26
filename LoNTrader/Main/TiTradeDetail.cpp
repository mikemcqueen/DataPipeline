///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TiTradeDetail.cpp
//
// Text interpreter for LoN Posted Trade Detail window.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TiTradeDetail.h"
#include "DcrTradeDetail.h"
#include "TradeDetailTypes.h"
#include "PipelineManager.h"
#include "LonCardSet_t.h"
#include "LonWindow_t.h"
#include "LonTrader_t.h"
#include "TradeManager_t.h"
#include "Services.h"
#include "TradeExecutor_t.h"

using namespace Lon;

/////////////////////////////////////////////////////////////////////////////

namespace TradeDetail
{
namespace Interpret
{

static const bool EXTRALOG = true;

/////////////////////////////////////////////////////////////////////////////

Handler_t::
Handler_t(
    ManagerBase_t& Manager)
    :
    m_Manager(Manager),
    m_TiOffered(Table::CharColumnWidths, Table::ColumnCount),
    m_TiWant   (Table::CharColumnWidths, Table::ColumnCount),
    m_bDone    (true)
{
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
ExecuteTransaction(
    const DP::Transaction::Data_t& Data)
{
    switch (Data.Id)
    {
    case Transaction::Id::BuyTrade:
        BuyTrade(static_cast<const TradeExecutor_t::EventBuy_t::Data_t&>(Data).TradeId);
        break;
    default:
        return S_FALSE;
    }
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
BuyTrade(
    TradeId_t TradeId)
{
    ASSERT(0 != TradeId);

    CLock lock(m_csState);
    ASSERT(m_bDone);
    m_bDone = false;
    m_TiOffered.ClearText();
    m_TiWant.ClearText();
    m_Trade.Clear();
    m_Trade.SetId(TradeId);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
MessageHandler(
    const DP::Message::Data_t* pMessage)
{
    if (Message::Id::TextTable != pMessage->Id)
        return S_FALSE;
    const Translate::Data_t*
        pDetailData = (Translate::Data_t*)pMessage;
    if (Window::PostedTradeDetailWindow != pDetailData->WindowType)
        return S_FALSE;

    CLock lock(m_csState);
    if (m_bDone)
        return S_FALSE;

    DP::Transaction::Data_t* pData = GetTransactionManager().Acquire();
    DP::TransactionManager_t::AutoRelease_t ar(pData);
    if (NULL == pData)
        return S_FALSE;
    if (Transaction::Id::BuyTrade != pData->Id)
        return S_FALSE;
    TradeExecutor_t::EventBuy_t::Data_t& Data =
        static_cast<TradeExecutor_t::EventBuy_t::Data_t&>(*pData);

    LogInfo(L"TradeDetail::Interpret::Handler_t::MessageHandler(%d)", pMessage->Id);

    size_t FirstNewLine;
    // Interpret offered items window.
    bool bNewOffered = m_TiOffered.CompareText(pDetailData->OfferedText, FirstNewLine);
    if (bNewOffered)
    {
        InterpretNewText(Window::PostedTradeDetailOfferedView, pDetailData->OfferedText.GetData(), FirstNewLine);
        m_TiOffered.SetText(pDetailData->OfferedText);
    }
    else
    {
        InterpretSameText(Window::PostedTradeDetailOfferedView, pDetailData->OfferedText.GetData());
    }

    // Interpret wanted items window.
    bool bNewWant = m_TiWant.CompareText(pDetailData->WantText, FirstNewLine);
    if (bNewWant)
    {
        InterpretNewText(Window::PostedTradeDetailWantView, pDetailData->WantText.GetData(), FirstNewLine);
        m_TiWant.SetText(pDetailData->WantText);
    }
    else
    {
        InterpretSameText(Window::PostedTradeDetailWantView, pDetailData->WantText.GetData());
    }

    if (m_TiOffered.IsTextEmpty() && m_TiWant.IsTextEmpty())
        return S_OK; // TODO: possible endless loop here, should add a counter to bail out
    if (m_TiOffered.IsTextEmpty() && (5 > m_TiOffered.GetSameTextCount()))
        return S_OK;
    if (m_TiWant.IsTextEmpty() && (5 > m_TiWant.GetSameTextCount()))
        return S_OK;

    // Determine if any windows need scrolling.
    bool bScrollOffered = bNewOffered && CanScroll(Window::PostedTradeDetailOfferedView);
    bool bScrollWant    = bNewWant    && CanScroll(Window::PostedTradeDetailWantView);

    if (bScrollOffered || bScrollWant)
    {
        Scroll(bScrollOffered, bScrollWant);
        return S_OK;
    }

    m_bDone = true;

    m_Trade.Show();
    // HACK: Testing BuyTrade
    if (0 == m_Trade.GetId())
        return S_OK;
    Services::GetTradeManager().ShowTrade(m_Trade.GetId());

    // TODO:
    // now what?
    // Post our interpreted trade back to PM?
    //   Or just call TradeManager.  maybe we should get trade manager
    //   registered as an analyzer.
    // Close the detail window. -- after we validate 
    //  that this is the trade to buy.  maybe TM should have sent us a 
    //  copy of the trade it expects us to find.
    using namespace Transaction;
    Error_e Error = Error::None;
    bool bMatch = Services::GetTradeManager().CompareTrade(m_Trade.GetId(), m_Trade);
    if (bMatch)
    {
        LogInfo(L"Trades match!");
        if (!Data.bTestPost)
            LonWindow_t::Click(Window::PostedTradeDetailAccept);
        else
            Error = Error::TestSucceeded;
    }
    else
    {
        LogError(L"Trades don't match!");
        Error = Error::TradesDontMatch;
    }
    if (Error::None != Error)
        LonWindow_t::Click(Window::PostedTradeDetailCancel);
    GetTransactionManager().CompleteTransaction(pData->Id);
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
CanScroll(
    Window::Type_e WindowType)
{
    Window::Type_e ScrollType = Window::Unknown;
    switch (WindowType)
    {
    case Window::PostedTradeDetailOfferedView:
        ScrollType = Window::PostedTradeDetailOfferedVScroll;
        break;
    case Window::PostedTradeDetailWantView:
        ScrollType = Window::PostedTradeDetailWantVScroll;
        break;
    default:
        ASSERT(false);
        return false;
    }
    return LonWindow_t::CanScroll(ScrollType);
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
InterpretSameText(
          Window::Type_e          WindowType,
//    const TiTable_t::Text_t& /*Text*/)
const Table::Text_t::Data_t& /*Text*/)
{
    LogInfo(L"Detail::InterpretSameText()");
WindowType;
/*
    switch (WindowType)
    {
    case Window::PostedTradeDetailOfferedView:
        ++m_SameOfferCount;
        break;
    case Window::PostedTradeDetailWantView:
        ++m_SameWantCount;
        break;
    default:
        ASSERT(false);
        return;
    }
*/
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
InterpretNewText(
          Window::Type_e   WindowType,
    const Table::Text_t::Data_t& Text,
          size_t           FirstNewLine)
{
    LogInfo(L"Detail::InterpretNewText(%d, %d)", FirstNewLine, WindowType);

    // TODO: never accept new text with firstnewline = 0 unless the trade is empty

    CardCollection_t* pCards = NULL;
    switch (WindowType)
    {
    case Window::PostedTradeDetailOfferedView:
        pCards = &m_Trade.GetOfferCards();
        break;
    case Window::PostedTradeDetailWantView:
        pCards = &m_Trade.GetWantCards();
        break;
    default:
        ASSERT(false);
        return;
    }
    ASSERT(NULL != pCards);
    ASSERT(Text.GetEndRow() > FirstNewLine);
    for (size_t Line = FirstNewLine; Line < Text.GetEndRow(); ++Line)
    {
        if (EXTRALOG)
        {
            Text.DumpRow(
                Line,
                (Window::PostedTradeDetailOfferedView == WindowType)
                ? L"Offer" : L"Want ");
        }
        AddTradeItem(*pCards, Text.GetRow(Line));
    }
}

/////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
InterpretLine(
    const wchar_t* pszText)
{
pszText;
    return true;
}

/////////////////////////////////////////////////////////////////////////////

// TODO:  should take TranslatedText_t&, size_t Line
bool
Handler_t::
AddTradeItem(
          CardCollection_t& Cards,
    const wchar_t*          pszText)
{
    // Find the quantity.
    // TODO: HACK: it happens to work because both view have same column widths
    size_t Pos = m_TiOffered.GetText().GetColumnOffset(Quantity);
    if (L'\0' == pszText[Pos])
    {
        LogError(L"Detail: Empty quantity string");
        return false;
    }
    int Count = _wtoi(&pszText[Pos]);
    if (0 >= Count)
    {
        LogError(L"Detail: Invalid quantity (%d)", Count);
        return false;
    }
    ASSERT(50 > Count);

    // Find the card name, and lookup the card.
    Pos = m_TiOffered.GetText().GetColumnOffset(CardName);
    const wchar_t* pszName = ParseItemName(&pszText[Pos]);
    if (L'\0' == pszName[0])
    {
        LogError(L"Detail: Empty name");
        return false;
    }
    const Card_t* pCard = Services::GetCardSet().Lookup(pszName);
    if (NULL == pCard)
    {
        LogError(L"Detail: Card not found '%ls'", pszName);
        return false;
    }
    CardQuantity_t item(pCard, Count);
    if (!Cards.insert(item).second)
    {
        LogError(L"Detail: Insert failed '%ls'", pCard->GetName());
        return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////

const wchar_t*
Handler_t::
ParseItemName(
    const wchar_t* pszText)
{
    while (iswspace(*pszText)) ++pszText;
    return pszText;
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
Scroll(
    bool bScrollOffered,
    bool bScrollWant)
{
    ASSERT(bScrollOffered || bScrollWant);

    // TODO:
    // Should be able to pagedown here. The PageDown->LineDown conversion code
    // in SSTrades is all fucked up because it depends on TrThumbPosition, which
    // is all fucked up. Need a better scrollbar manager/abstraction..

    static const LonWindow_t::EventScroll_t::Type_e ScrollType  = LonWindow_t::EventScroll_t::LineDown;
    static const size_t                             ScrollCount = 5;

    if (bScrollOffered && bScrollWant)
    {
        LonWindow_t::EventScroll_t
            OfferedEvent(Window::PostedTradeDetailOfferedVScroll,
                         ScrollBar_t::Vertical, ScrollType, ScrollCount);
        LonWindow_t::EventScroll_t
            WantEvent   (Window::PostedTradeDetailWantVScroll,
                         ScrollBar_t::Vertical, ScrollType, ScrollCount);
        Event::Collection_t Event(DP::Stage::Acquire);
        Event.Add(&OfferedEvent.m_Data);
        Event.Add(&WantEvent.m_Data);
        GetPipelineManager().SendEvent(Event.m_Data);
    }
    else
    {
        const Window::Type_e WindowType =
            (bScrollOffered) ? Window::PostedTradeDetailOfferedVScroll :
                               Window::PostedTradeDetailWantVScroll;
        LonWindow_t::Scroll(WindowType, ScrollBar_t::Vertical, ScrollType, ScrollCount);
    }
}

/////////////////////////////////////////////////////////////////////////////

} // Interpret
} // TradeDetail

/////////////////////////////////////////////////////////////////////////////
