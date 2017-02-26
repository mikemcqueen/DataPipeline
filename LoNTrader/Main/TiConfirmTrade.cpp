/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TiConfirmTrade.cpp
//
// Text interpreter for Lon Posted Trade Builder Confirm Trade window.
// This is the window you see when you attempt to submit a new trade 
// via the Trade Builder.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TiConfirmTrade.h"
#include "DcrConfirmTrade.h"
#include "PipelineManager.h"
#include "LonCardSet_t.h"
#include "LonWindow_t.h"
#include "LonTrader_t.h"
#include "Services.h"
#include "TransactionManager.h"

using namespace Lon;

namespace ConfirmTrade
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
    m_bDone(true)
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
    case Transaction::Id::PostTrade:
        ConfirmTrade(static_cast<const TradePoster::EventPost_t::Data_t&>(Data));
        break;
    default:
        return S_FALSE;
    }
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
OnTransactionComplete(
    const DP::Transaction::Data_t& Data)
{
    switch (Data.Id)
    {
    case Transaction::Id::PostTrade:
        m_bDone = true;
        break;
    default:
        return S_FALSE;
    }
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
ConfirmTrade(
    const TradePoster::EventPost_t::Data_t& Data)
{
    CLock lock(m_csState);

    // Tricky. This class receives the "new transaction" event,
    // and sets bDone = false.  No guarantee we get this far tho.
    // maybe "bDone" isn't enough.
    ASSERT(m_bDone); 

    m_TiOffered.ClearText();
    m_TiWant.ClearText();
    m_Trade.Clear();
    m_Trade.SetId(Data.pTrade->GetId());
    m_Trade.SetUser(L"Built");
    m_bDone = false;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
MessageHandler(
    const DP::Message::Data_t* pMessage)
{
    ASSERT(NULL != pMessage);
    if (Message::Id::TextTable != pMessage->Id)
        return S_FALSE;

    const Translate::Data_t*
        pConfirmData = static_cast<const Translate::Data_t*>(pMessage);
    if (TopWindowType != pConfirmData->WindowType)
        return S_FALSE;
    CLock lock(m_csState);
    if (m_bDone)
        return S_FALSE;

    DP::Transaction::Data_t* pData = GetTransactionManager().Acquire();
    DP::TransactionManager_t::AutoRelease_t ar(pData);
    if (NULL == pData) 
        return S_FALSE;
    if (Transaction::Id::PostTrade != pData->Id)
        return S_FALSE;
    TradePoster::EventPost_t::Data_t& PostData =
        static_cast<TradePoster::EventPost_t::Data_t&>(*pData);

    LogInfo(L"Handler_t::Interpret(%d)", pConfirmData->Id);

    size_t FirstNewLine;
    // Interpret offered items window.
    bool bNewOffered = m_TiOffered.CompareText(pConfirmData->OfferedText, FirstNewLine);
    if (bNewOffered)
    {
        InterpretNewText(Window::ConfirmTradeTheyGetView, pConfirmData->OfferedText, FirstNewLine);
        m_TiOffered.SetText(pConfirmData->OfferedText);
    }
    else
    {
        InterpretSameText(Window::ConfirmTradeTheyGetView, pConfirmData->OfferedText);
    }

    // Interpret wanted items window.
    bool bNewWant = m_TiWant.CompareText(pConfirmData->WantText, FirstNewLine);
    if (bNewWant)
    {
        InterpretNewText(Window::ConfirmTradeYouGetView, pConfirmData->WantText, FirstNewLine);
        m_TiWant.SetText(pConfirmData->WantText);
    }
    else
    {
        InterpretSameText(Window::ConfirmTradeYouGetView, pConfirmData->WantText);
    }

    if (m_TiOffered.IsTextEmpty() && m_TiWant.IsTextEmpty())
        return S_OK; // TODO: possible endless loop here, should add a counter to bail out
    if (m_TiOffered.IsTextEmpty() && (5 > m_TiOffered.GetSameTextCount()))
        return S_OK;
    if (m_TiWant.IsTextEmpty() && (5 > m_TiWant.GetSameTextCount()))
        return S_OK;

    // Determine if any windows need scrolling.
    bool bScrollOffered = bNewOffered && CanScroll(Window::ConfirmTradeTheyGetView);
    bool bScrollWant    = bNewWant    && CanScroll(Window::ConfirmTradeYouGetView);

    if (bScrollOffered || bScrollWant)
    {
        Scroll(bScrollOffered, bScrollWant);
        return S_OK;
    }

    m_bDone = true;
    m_Trade.Show();
    PostData.pTrade->Show();

    using namespace Lon::Transaction;
    Error_e Error = Error::None;
    if (m_Trade.Compare(*PostData.pTrade))
    {
        LogAlways(L"Confirm cards match!");
        if (!PostData.bTestPost)
        {
            LonWindow_t::Click(Window::ConfirmTradeConfirm);
            PostData.SetState(DP::Transaction::State::Pending);
            // Trade successfully posted. The transaction is not
            // complete until we verify the Trade Id came through
            // in TiPostedTrades.
        }
        else
            Error = Error::TestSucceeded;
    }
    else
    {
        const wchar_t* pszText = L"<empty>";
        if (!PostData.pTrade->GetWantCards().empty())
            pszText = PostData.pTrade->GetWantCards().begin()->pCard->GetName();
        LogError(L"Confirm cards don't match! '%ls'", pszText);
        Error = Error::ConfirmTrade;
    }
    if (Error::None != Error)
    {
        LonWindow_t::Click(Window::ConfirmTradeCancel);
        PostData.Error = Error;
        // If an error occurred we'll be stuck in the trade builder
        // window, so we need to keep the transaction executing until we
        // return back to the trade builder window interpreter, where we 
        // will discover the error, and complete the transaction.
    }
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
    case Window::ConfirmTradeTheyGetView:
        ScrollType = Window::ConfirmTradeTheyGetVScroll;
        break;
    case Window::ConfirmTradeYouGetView:
        ScrollType = Window::ConfirmTradeYouGetVScroll;
        break;
    default:
        ASSERT(false);
        return false;
    }
    ASSERT(Window::Unknown != ScrollType);
    HWND hScroll = LonWindow_t::GetWindow(ScrollType);
    if ((NULL == hScroll) || !IsWindow(hScroll) || !IsWindowVisible(hScroll))
        return false;
    return true;
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
InterpretSameText(
          Window::Type_e     WindowType,
    const TiTable_t::Text_t& /*Text*/)
{
    LogInfo(L"Confirm::InterpretSameText()");
WindowType;
/*
    switch (WindowType)
    {
    case LonWindow_t::PostedTradeDetailOfferedView:
        ++m_SameOfferCount;
        break;
    case LonWindow_t::PostedTradeDetailWantView:
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
           Window::Type_e      WindowType,
     const TiTable_t::Text_t&  Text,
           size_t              FirstNewLine)
{
    LogInfo(L"Confirm::InterpretNewText(%d, %d)", FirstNewLine, WindowType);

    // TODO: never accept new text with firstnewline = 0 unless the trade is empty

    CardCollection_t* pCards = NULL;
    switch (WindowType)
    {
    case Window::ConfirmTradeTheyGetView:
        pCards = &m_Trade.GetOfferCards();
        break;
    case Window::ConfirmTradeYouGetView:
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
            Text.DumpRow(Line,
                          (Window::ConfirmTradeTheyGetView == WindowType)
                          ? L"They get" : L" You get");
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
    // TODO: HACK: it happens to work because both views have same column widths
    size_t Pos = m_TiOffered.GetText().GetColumnOffset(Quantity);
    if (L'\0' == pszText[Pos])
    {
        LogError(L"Confirm: Empty quantity string");
        return false;
    }
    int Count = _wtoi(&pszText[Pos]);
    if (0 >= Count)
    {
        LogError(L"Confirm: Invalid quantity (%d)", Count);
        return false;
    }
    ASSERT(50 > Count);

    // Find the card name, and lookup the card.
    Pos = m_TiOffered.GetText().GetColumnOffset(CardName);
    const wchar_t* pszName = ParseItemName(&pszText[Pos]);
    if (L'\0' == pszName[0])
    {
        LogError(L"Confirm: Empty name");
        return false;
    }
    const Card_t* pCard = Services::GetCardSet().Lookup(pszName);
    if (NULL == pCard)
    {
        LogError(L"Confirm: Card not found '%ls'", pszName);
        return false;
    }
    CardQuantity_t cq(pCard, Count);
    if (!Cards.insert(cq).second)
    {
        LogError(L"Confirm: Insert failed '%ls'", pCard->GetName());
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
    static const size_t                    ScrollCount = 5;

    if (bScrollOffered && bScrollWant)
    {
        LonWindow_t::EventScroll_t OfferedEvent(Window::ConfirmTradeTheyGetVScroll,
                                                ScrollBar_t::Vertical, ScrollType, ScrollCount);
        LonWindow_t::EventScroll_t WantEvent   (Window::ConfirmTradeYouGetVScroll,
                                                ScrollBar_t::Vertical, ScrollType, ScrollCount);
        Event::Collection_t Event(DP::Stage::Acquire);
        Event.Add(&OfferedEvent.m_Data);
        Event.Add(&WantEvent.m_Data);
        GetPipelineManager().SendEvent(Event.m_Data);
    }
    else if (bScrollOffered)
        Scroll(Window::ConfirmTradeTheyGetVScroll, ScrollType, ScrollCount);
    else
        Scroll(Window::ConfirmTradeYouGetVScroll, ScrollType, ScrollCount);
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
Scroll(
    Window::Type_e       WindowType,
    LonWindow_t::EventScroll_t::Type_e ScrollType,
    size_t                    Count)
{
    LonWindow_t::Scroll(WindowType, ScrollBar_t::Vertical, ScrollType, Count);
}

/////////////////////////////////////////////////////////////////////////////

} // Interpret
} // ConfirmTrade

/////////////////////////////////////////////////////////////////////////////
