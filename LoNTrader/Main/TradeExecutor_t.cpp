///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TradeExecutor_t.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TradeExecutor_t.h"
#include "TransactionManager.h"
#include "Log.h"
#include "Services.h"
#include "TradePoster.h"
#include "PcapTrades_t.h"

///////////////////////////////////////////////////////////////////////////////

TradeExecutor_t::
TradeExecutor_t()
{
}

///////////////////////////////////////////////////////////////////////////////

TradeExecutor_t::
~TradeExecutor_t()
{
}

///////////////////////////////////////////////////////////////////////////////

HRESULT
TradeExecutor_t::
MessageHandler(
    const DP::Message::Data_t* pMessage)
{
    using namespace Lon::Message;
    if (Id::RemoveTrade == pMessage->Id)
    {
        const PcapTrades_t::AcquireData_t&
            PcapData = static_cast<const PcapTrades_t::AcquireData_t&>(*pMessage);
        OnRemoveTrade(PcapData.TradeId);
        return S_OK;
    }
    return S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT
TradeExecutor_t::
ExecuteTransaction(
    const DP::Transaction::Data_t& Data)
{
Data;
    // TODO
    return S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT
TradeExecutor_t::
OnTransactionComplete(
    const DP::Transaction::Data_t& Data)
{
    using namespace Lon::Transaction;
    const DWORD Error = Data.Error;
    switch (Data.Id)
    {
    case Id::BuyTrade:
        OnBuyTradeComplete(Error);
        break;
    case Id::RemoveTrade:
        OnRemoveTradeComplete(Error);
        break;
    case Id::PostTrade:
        OnPostTradeComplete(Error);
        break;
    default:
        return S_FALSE;
    }
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

void
TradeExecutor_t::
LogOnComplete(
    const wchar_t* pszText,
          DWORD    Error) const
{
    using namespace Lon::Transaction;
    switch (Error)
    {
    case Error::None:
        LogAlways(L"%ls complete - success", pszText);
        break;
    case Error::TestSucceeded:
        LogAlways(L"%ls complete - test succeeded", pszText);
        break;
    default:
        LogError(L"%ls complete - failed (%d)", pszText, Error);
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool
TradeExecutor_t::
BuyTrade(
    TradeId_t TradeId,
    bool      bTest) const
{
    LogInfo(L"TradeExecutor::BuyTrade(%d)", TradeId);
    EventBuy_t::Data_t *pData =
        new EventBuy_t::Data_t(TradeId, bTest);
    GetTransactionManager().ExecuteTransaction(pData);
    // Now we wait for a OnBuyTradeComplete.
    return true;
}

//////////////////////////////////////////////////////////////////////////////

void
TradeExecutor_t::
OnBuyTradeComplete(
    DWORD Error) const
{
    LogOnComplete(L"BuyTrade", Error);
}

//////////////////////////////////////////////////////////////////////////////

bool
TradeExecutor_t::
PostTrade(
    const Trade_t&    Trade,
    TradePoster::Id_t TradePosterId,
    size_t            Value,
    bool              bTestPost) const
{
    LogAlways(L"TradeExecutor::PostTrade(%d,%d)", Trade.GetId(), bTestPost);
    // TODO: Flags on constructor
    TradePoster::EventPost_t::Data_t *pData = new
        TradePoster::EventPost_t::Data_t(
            Trade,
            TradePosterId,
            Value,
            bTestPost,
            true);
    GetTransactionManager().ExecuteTransaction(pData);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void
TradeExecutor_t::
OnPostTradeComplete(
    DWORD Error) const
{
    LogOnComplete(L"PostTrade", Error);
}

///////////////////////////////////////////////////////////////////////////////

bool
TradeExecutor_t::
RemoveTrades(
    const TradeIdSet_t& TradeIds,
          bool          bTest) const
{
    TradeIdSet_t::const_iterator it = TradeIds.begin();
#if 1
    for (; TradeIds.end() != it; ++it)
    {
        RemoveTrade(*it, bTest);
    }
#else
    std::for_each(TradeIds.begin(), TradeIds.end(),
        boost::bind(&TradeExecutor_t::RemoveTrade, _1, bTest)); 
#endif
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
TradeExecutor_t::
RemoveTrade(
    TradeId_t TradeId,
    bool      bTest) const
{
    LogInfo(L"TradeExecutor::RemoveTrade(%d)", TradeId);
    EventRemove_t::Data_t *pData = new EventRemove_t::Data_t(TradeId, bTest);
    GetTransactionManager().ExecuteTransaction(pData);
    // Now we wait for a OnRemoveTrade().
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void
TradeExecutor_t::
OnRemoveTrade(
    TradeId_t TradeId) const
{
    DP::Transaction::Data_t* pData = GetTransactionManager().Acquire();
    DP::TransactionManager_t::AutoRelease_t ar(pData);
    if ((NULL != pData) &&
        (Lon::Transaction::Id::RemoveTrade == pData->Id) &&
        (DP::Transaction::State::Pending == pData->GetState()))
    {
        const EventRemove_t::Data_t&
            Data = static_cast<const EventRemove_t::Data_t&>(*pData);
        if (Data.TradeId == TradeId)
        {
            // This generates a call to OnRemoveTradeComplete().
            GetTransactionManager().CompleteTransaction(pData->Id);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void
TradeExecutor_t::
OnRemoveTradeComplete(
    DWORD Error) const
{
    LogOnComplete(L"RemoveTrade", Error);
}

///////////////////////////////////////////////////////////////////////////////
