/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// PostedTradesPolicy.h
//
// Posted Trades window policies.
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_POSTEDTRADESPOLICY_H
#define Include_POSTEDTRADESPOLICY_H

#include "Trade_t.h"
#include "DpTransaction.h"

/////////////////////////////////////////////////////////////////////////////

namespace PostedTrades
{
namespace Interpret
{

class Handler_t;

/////////////////////////////////////////////////////////////////////////////

class Policy_t
{
private:
    Handler_t*        m_pHost;
    DP::MessageId_t m_TransactionId;

public:
    Policy_t(
        DP::MessageId_t TransactionId = DP::Message::Id::Unknown,
        Handler_t*        pHost = NULL)
        :
        m_TransactionId(TransactionId),
        m_pHost(pHost)
    {}

    Handler_t*        GetHost() const             { return m_pHost; }
    DP::MessageId_t   GetTransactionId() const    { return m_TransactionId; }

    virtual bool PreInterpret(const DP::Transaction::Data_t* pData) const = 0;
    virtual bool ShouldCompareText() const                              = 0;
    virtual bool CheckTrade(const Trade_t& Trade) const                 = 0;
    virtual bool ShouldSetText() const                                  = 0;
    virtual void TradeFound(TradeId_t TradeId, RECT& Rect) const        = 0;
    virtual void TradeNotFound(TradeId_t TradeId) const                 = 0;
    virtual void DoneGathering() const                                  = 0;
};

/////////////////////////////////////////////////////////////////////////////

class DefaultPolicy_t :
    public Policy_t
{
public:
    DefaultPolicy_t(
        DP::MessageId_t TransactionId = DP::Message::Id::Unknown,
        Handler_t*        pHost = NULL)
        :
        Policy_t(TransactionId, pHost)
    {}

    virtual bool PreInterpret(const DP::Transaction::Data_t* pData) const;
    virtual bool ShouldCompareText() const;
    virtual bool CheckTrade(const Trade_t& Trade) const;
    virtual bool ShouldSetText() const;
    virtual void TradeFound(TradeId_t TradeId, RECT& Rect) const;
    virtual void TradeNotFound(TradeId_t TradeId) const;
    virtual void DoneGathering() const;
};

/////////////////////////////////////////////////////////////////////////////

class BuyTradePolicy_t :
    public DefaultPolicy_t 
{
public:
    BuyTradePolicy_t() : DefaultPolicy_t(Lon::Transaction::Id::BuyTrade) {}

    virtual bool ShouldCompareText() const;
    virtual void TradeFound(TradeId_t TradeId, RECT& Rect) const;
    virtual void TradeNotFound(TradeId_t TradeId) const;
};

/////////////////////////////////////////////////////////////////////////////

class GetYourCardsPolicy_t :
    public DefaultPolicy_t
{
public:
    GetYourCardsPolicy_t() : DefaultPolicy_t(Lon::Transaction::Id::GetYourCards) {}

    virtual bool PreInterpret(const DP::Transaction::Data_t* pData) const;
};

/////////////////////////////////////////////////////////////////////////////

class PostTradePolicy_t :
    public DefaultPolicy_t
{
public:
    PostTradePolicy_t() : DefaultPolicy_t(Lon::Transaction::Id::PostTrade) {}

    virtual bool PreInterpret(const DP::Transaction::Data_t* pData) const;
    virtual bool CheckTrade(const Trade_t& Trade) const;
};

/////////////////////////////////////////////////////////////////////////////

class RemoveTradePolicy_t :
    public DefaultPolicy_t
{
public:
    RemoveTradePolicy_t() : DefaultPolicy_t(Lon::Transaction::Id::RemoveTrade) {}

    virtual bool ShouldCompareText() const;
    virtual void TradeFound(TradeId_t TradeId, RECT& Rect) const;
    virtual void TradeNotFound(TradeId_t TradeId) const;
};

class GatherTradesPolicy_t :
    public DefaultPolicy_t
{
public:
    GatherTradesPolicy_t() : DefaultPolicy_t(Lon::Transaction::Id::GatherTrades) {}

    virtual void DoneGathering() const;
};

/////////////////////////////////////////////////////////////////////////////

} // Interpret
} // PostedTrades

/////////////////////////////////////////////////////////////////////////////

#endif Include_POSTEDTRADESPOLICY_H

/////////////////////////////////////////////////////////////////////////////
