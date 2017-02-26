/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// PostedTradesPolicy.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TiPostedTrades.h"
#include "PostedTradesPolicy.h"
#include "LonWindow_t.h"
#include "TradePoster.h"
#include "Macros.h"
#include "TransactionManager.h"

using namespace Lon;

extern Window::Type_e g_TradeBuilderVScroll;
extern Window::Type_e g_TradeBuilderHScroll;

namespace PostedTrades
{
namespace Interpret
{

/////////////////////////////////////////////////////////////////////////////
//
// DefaultPolicy_t
//
/////////////////////////////////////////////////////////////////////////////

bool
DefaultPolicy_t::
PreInterpret(
    const DP::Transaction::Data_t* /*pData*/) const
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool
DefaultPolicy_t::
ShouldCompareText() const
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool
DefaultPolicy_t::
CheckTrade(
    const Trade_t& /*Trade*/) const
{
    return false;
}

/////////////////////////////////////////////////////////////////////////////

bool
DefaultPolicy_t::
ShouldSetText() const
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////

void
DefaultPolicy_t::
TradeFound(
    TradeId_t /*TradeId*/,
    RECT&     /*Rect*/) const
{
}

/////////////////////////////////////////////////////////////////////////////

void
DefaultPolicy_t::
TradeNotFound(
    TradeId_t /*TradeId*/) const
{
}

/////////////////////////////////////////////////////////////////////////////

void
DefaultPolicy_t::
DoneGathering() const
{
}

/////////////////////////////////////////////////////////////////////////////
//
// BuyTradePolicy_t
//
/////////////////////////////////////////////////////////////////////////////

bool
BuyTradePolicy_t::
ShouldCompareText() const
{
    return false;
}

/////////////////////////////////////////////////////////////////////////////

void
BuyTradePolicy_t::
TradeFound(
    TradeId_t TradeId,
    RECT&     Rect) const
{
    LogInfo(L"BuyTradePolicy_t::TradeFound(%d)", TradeId);

    // TODO: LonWindow_t::EventClick has WindowType
    // ASSERT(TradeId == pTransactionData->TradeId);

    // Rect is relative to PostedTradesList,
    // we want it relative to PostedTradesView.
    // TODO: if (!) failure
    LonWindow_t::ConvertRect(Window::PostedTradesList, Rect, Window::PostedTradesView);
    // Double-click
    LonWindow_t::Click(Window::PostedTradesView, &Rect, LonWindow_t::EventClick_t::LeftButton, 1, true);
    // TODO: Loophole for the transaction processing.. 
}

/////////////////////////////////////////////////////////////////////////////

void
BuyTradePolicy_t::
TradeNotFound(
    TradeId_t TradeId) const
{
    LogInfo(L"BuyTradePolicy_t::TradeNotFound(%d)", TradeId);
    //  ASSERT(TradeId == GetCurrentTransaction().TradeId);
    GetTransactionManager().CompleteTransaction(
        GetTransactionId(),
        Transaction::Error::TradeNotFound);
}

/////////////////////////////////////////////////////////////////////////////
//
// GetYourCardsPolicy_t
//
/////////////////////////////////////////////////////////////////////////////

bool
GetYourCardsPolicy_t::
PreInterpret(
    const DP::Transaction::Data_t* pData) const
{
    ASSERT(DP::Transaction::State::New == pData->GetState());
    if (DP::Transaction::State::New == pData->GetState())
    {
        g_TradeBuilderVScroll = Window::TradeBuilderTheirTableVScroll;
        g_TradeBuilderHScroll = Window::TradeBuilderTheirTableHScroll;
        LonWindow_t::Click(Window::PostedTradesCreateButton);
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////
//
// PostTradePolicy_t
//
/////////////////////////////////////////////////////////////////////////////

bool
PostTradePolicy_t::
PreInterpret(
    const DP::Transaction::Data_t* pData) const
{
    using namespace DP::Transaction;
    if (State::New == pData->GetState())
    {
        g_TradeBuilderVScroll = Window::TradeBuilderTheirTableVScroll;
        g_TradeBuilderHScroll = Window::TradeBuilderTheirTableHScroll;
        LonWindow_t::Click(Window::PostedTradesCreateButton);
        return false;
    }
    // TODO: I think this is happening at "complete" state sometimes too.
    //       I don't think there are any negative side effects.
    ASSERT(State::Pending == pData->GetState());
    if (State::Pending != pData->GetState())
        LogError(L"State Expected(%d) Actual(%d)", State::Pending, pData->GetState());
    const TradePoster::EventPost_t::Data_t&
        PostData = static_cast<const TradePoster::EventPost_t::Data_t&>(*pData);
    ASSERT(!PostData.bTestPost);
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool
PostTradePolicy_t::
CheckTrade(
    const Trade_t& Trade) const
{
    LogAlways(L"CheckTrade(%d)", Trade.GetId());

    DP::Transaction::Data_t* pData = GetTransactionManager().Acquire();
    DP::TransactionManager_t::AutoRelease_t ar(pData);
    ASSERT(NULL != pData);
    ASSERT(Transaction::Id::PostTrade == pData->Id);
    if ((NULL != pData) &&
        (Transaction::Id::PostTrade == pData->Id) &&
        (DP::Transaction::State::Pending == pData->GetState()))
    {
        TradePoster::EventPost_t::Data_t&
            PostData = static_cast<TradePoster::EventPost_t::Data_t&>(*pData);
        if (PostData.pTrade->Compare(Trade, true))
        {
            PostData.TradeId = Trade.GetId();
            GetTransactionManager().CompleteTransaction(GetTransactionId());
            return true;
        }
    }
    return false;
}
  
/////////////////////////////////////////////////////////////////////////////
//
// RemoveTradePolicy_t
//
/////////////////////////////////////////////////////////////////////////////

bool
RemoveTradePolicy_t::
ShouldCompareText() const
{
    return false;
}

/////////////////////////////////////////////////////////////////////////////

void
RemoveTradePolicy_t::
TradeFound(
    TradeId_t TradeId,
    RECT&     Rect) const
{
    LogAlways(L"RemoveTradePolicy_t::TradeFound(%d)", TradeId);

    LonWindow_t::ConvertRect(Window::PostedTradesList, Rect, Window::PostedTradesView);
    // Direct right-click
    LonWindow_t::Click(Window::PostedTradesView,
                       &Rect,
                       LonWindow_t::EventClick_t::RightButton,
                       1,
                       false,
                       true); 
    // That was a direct, synchronous click. Popup window should be up now.
    HWND hPopup = FindWindow(L"QPopup", L"LegendsOfNorrath");
    if (NULL == hPopup)
    {
        LogError(L"TradeFound(): Popup window error");
    }
    else
    {
        ASSERT(FALSE != IsWindowVisible(hPopup));
        RECT rcPopup;
        GetClientRect(hPopup, &rcPopup);
        int Third = RECTHEIGHT(rcPopup) / 3;
        rcPopup.top = Third / 2;
        rcPopup.bottom = rcPopup.top + Third;
        const POINT pt = { rcPopup.left + RECTWIDTH (rcPopup) / 2,
                           rcPopup.top  + RECTHEIGHT(rcPopup) / 2 };
        const LPARAM lParam = MAKELONG(pt.x, pt.y);

        DP::Transaction::Data_t* pData = GetTransactionManager().Acquire();
        DP::TransactionManager_t::AutoRelease_t ar(pData);
        if ((NULL == pData) ||
            (Transaction::Id::RemoveTrade != pData->Id) ||
            (DP::Transaction::State::New != pData->GetState()))
        {
            if (NULL == pData)
                LogError(L"TradeFound(): NULL == pData");
            else
                LogError(L"TradeFound(): Type (%d) State(%d)",
                         pData->Id, pData->GetState());
            throw std::logic_error("TradeFound(): Invalid state");
        }
        pData->SetState(DP::Transaction::State::Pending);
        // Another direct, synch click
        LonWindow_t::Click(hPopup, lParam);
    }
}

/////////////////////////////////////////////////////////////////////////////

void
RemoveTradePolicy_t::
TradeNotFound(
    TradeId_t TradeId) const
{
    LogInfo(L"RemoveTradePolicy_t::TradeNotFound(%d)", TradeId);
    //  ASSERT(TradeId == GetCurrentTransaction().TradeId);
    GetTransactionManager().CompleteTransaction(
        GetTransactionId(),
        Transaction::Error::TradeNotFound);
}

/////////////////////////////////////////////////////////////////////////////
//
// GatherTradesPolicy_t
//
/////////////////////////////////////////////////////////////////////////////

void
GatherTradesPolicy_t::
DoneGathering() const
{
    GetTransactionManager().CompleteTransaction(GetTransactionId());
}

/////////////////////////////////////////////////////////////////////////////

} // Interpet
} // PostedTrades

/////////////////////////////////////////////////////////////////////////////
