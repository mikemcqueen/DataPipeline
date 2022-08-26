/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TradeExecutor_t.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TRADEEXECUTOR_T_H
#define Include_TRADEEXECUTOR_T_H

#include "DpHandler_t.h"
#include "DpTransaction.h"
#include "TradePoster.h"
#include "TradeTypes.h"
#include "TradeMap_t.h"
#include "AutoCs.h"

/////////////////////////////////////////////////////////////////////////////
//
// TradeExecutor_t
//
/////////////////////////////////////////////////////////////////////////////

class TradeExecutor_t :
    public DP::Handler_t
{
friend class LonTrader_t;

public:

    class EventBuy_t
    {

    public:

        struct Data_t :
            public DP::Transaction::Data_t
        {
            TradeId_t TradeId;
            bool      bTestPost;     // don't accept post, cancel out

            Data_t(
                const TradeId_t InitTradeId,
                      bool      InitTestPost)
            :
                DP::Transaction::Data_t(
                    Lon::Transaction::Id::BuyTrade,
                    sizeof(Data_t)),
                TradeId(InitTradeId),
                bTestPost(InitTestPost)
            { }

        private:

            Data_t();

        } m_Data;

        EventBuy_t(
            const TradeId_t TradeId,
                  bool      bTestPost = false)
        :
            m_Data(TradeId, bTestPost)
        { }

    private:

        EventBuy_t();
    };

    class EventRemove_t
    {

    public:

        struct Data_t :
            public DP::Transaction::Data_t
        {
            TradeId_t TradeId;
            bool      bTestPost;

            Data_t(
                const TradeId_t InitTradeId,
                      bool      InitTestPost)
            :
                DP::Transaction::Data_t(
                    Lon::Transaction::Id::RemoveTrade,
                    sizeof(Data_t)),
                TradeId(InitTradeId),
                bTestPost(InitTestPost)
            { }

        private:

            Data_t();

        } m_Data;

        EventRemove_t(
            const TradeId_t TradeId,
                  bool      bTestPost = false)
        :
            m_Data(TradeId, bTestPost)
        { }

    private:

        EventRemove_t();
    };

public:

    TradeExecutor_t();
    ~TradeExecutor_t();

    //
    // DP::Handler_t virtual:
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

    // Helpers:

    bool
    BuyTrade(
        TradeId_t TradeId,
        bool      bTest = false) const;

    bool
    RemoveTrades(
        const TradeIdSet_t& TradeIds,
              bool          bTest = false) const;

    bool
    RemoveTrade(
        TradeId_t TradeId,
        bool      bTest = false) const;

    bool
    PostTrade(
        const Trade_t&          Trade_t,
              TradePoster::Id_t TradePosterId,
              size_t            Value,
              bool              bTestPost) const;

private:

    void
    LogOnComplete(
        const wchar_t*           pszText,
              DWORD Error) const;

    void
    OnBuyTradeComplete(
        DWORD Error) const;

    void
    OnRemoveTrade(
        TradeId_t TradeId) const;

    void
    OnRemoveTradeComplete(
        DWORD Error) const;

    void
    OnPostTradeComplete(
        DWORD Error) const;
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TRADEEXECUTOR_T_H

/////////////////////////////////////////////////////////////////////////////
