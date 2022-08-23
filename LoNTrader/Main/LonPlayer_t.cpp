///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// LonPlayer_t.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LonPlayer_t.h"
#include "PcapTrades_t.h"
#include "DbCardSet_t.h"
#include "LonTrader_t.h"
#include "LonCardSet_t.h"
#include "Services.h"
#include "TradeManager_t.h"
#include "Log.h"
#include "DpTransaction.h"
#include "LonMessageTypes.h"

/////////////////////////////////////////////////////////////////////////////

LonPlayer_t::
LonPlayer_t() :
    m_Timer(L"LonPlayer_t", false)
{
}

/////////////////////////////////////////////////////////////////////////////

#pragma warning(disable:4063)

HRESULT
LonPlayer_t::
OnTransactionComplete(
    const DP::Transaction::Data_t& Data)
{
    switch (Data.Id)
    {
    case Lon::Transaction::Id::GetYourCards:
        OnGetYourCardsComplete(static_cast<const EventGetYourCards_t::Data_t&>(Data));
        break;
    default:
        return S_FALSE;
    }
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
LonPlayer_t::
MessageHandler(
    const DP::Message::Data_t* pData)
{
    ASSERT(NULL != pData);
    if (Lon::Message::Id::RemoveTrade == pData->Id)
    {
        const PcapTrades_t::AcquireData_t&
            PcapData = static_cast<const PcapTrades_t::AcquireData_t&>(*pData);
        OnRemoveTrade(PcapData.TradeId);
        return S_OK;
    }
    return S_FALSE;
}

////////////////////////////////////////////////////////////////////////////

bool
LonPlayer_t::
DoGetYourCards() const
{
    LogInfo(L"LonPlayer_t::GetYourCards()");
    DP::Transaction::Data_t *pData = new EventGetYourCards_t::Data_t;
    GetTransactionManager().ExecuteTransaction(pData);
    m_Timer.Set();
    // Now we wait for a OnGetYourCardsComplete()
    return true;
}

////////////////////////////////////////////////////////////////////////////

void
LonPlayer_t::
OnGetYourCardsComplete(
    const EventGetYourCards_t::Data_t& Data)
{
    m_Timer.Show(L"GetYourCards");
    m_Timer.Set();
    if (0 == Data.Error)
    {
        LogAlways(L"  OnGetYourCardsComplete() success!");
        m_YourCards = Data.YourCards;
        LogAlways(L"  YourCards (%d, %d)",
                  m_YourCards.size(), m_YourCards.GetTotalQuantity());
    }
    else
        LogError(L"OnGetYourCardsComplete(%d) error", Data.Error);
}

////////////////////////////////////////////////////////////////////////////

void
LonPlayer_t::
OnRemoveTrade(
    TradeId_t TradeId)
{
    Trade_t Trade;
    if (!Services::GetTradeManager().GetTrade(TradeId, Trade))
        return;
    // TradeManager_t should have processed this event before us,
    // resulting in the Removed flag being set.
    ASSERT(Trade.TestFlags(Trade_t::Flag::Removed));

    if (!CompareName(Trade.GetUser()))
        return;

    const size_t Removed = m_YourCards.Remove(Trade.GetOfferCards());
    ASSERT(Removed == Trade.GetOfferCards().GetTotalQuantity());
    const size_t Added = m_YourCards.Add(Trade.GetWantCards());
    ASSERT(Added == Trade.GetWantCards().GetTotalQuantity());
    LogAlways(L"LonPlayer_t::OnRemoveTrade(%d) Removed(%d,%d) Added(%d,%d)",
              TradeId,
              Removed, Trade.GetOfferCards().GetTotalQuantity(),
              Added,   Trade.GetWantCards().GetTotalQuantity());
}

////////////////////////////////////////////////////////////////////////////

bool
LonPlayer_t::
WriteYourCards()
{
    CDatabase db;
    try
    {
        BOOL b;
        b = db.Open(NULL, FALSE, FALSE, LonTrader_t::GetDbConnectString(), FALSE);
        ASSERT(!!b);
        DbCardSet_t rs(&db);
        b = rs.Open(CRecordset::dynaset, NULL, CRecordset::appendOnly);
        ASSERT(!!b);
        if (!rs.CanAppend())
        {
            LogError(L"Can't append yourcards_t");
            return false;
        }
        CardCollection_t::const_iterator it = m_YourCards.begin();
        for (; m_YourCards.end() != it; ++it)
        {
            rs.AddNew();
            rs.m_cardid   = (long)it->CardId;
            rs.m_quantity = (long)it->Quantity;
            rs.Update();
        }
        rs.Close();
        db.Close();
        return true;
    }
    catch(CDBException *e)
    {
        LogError(L"WriteYourCards() exception: %ls", e->m_strError);
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////

bool
LonPlayer_t::
ReadYourCards()
{
    m_YourCards.clear();
    CDatabase db;
    try
    {
        BOOL b = db.Open(NULL, FALSE, FALSE, LonTrader_t::GetDbConnectString(), FALSE);
        DbCardSet_t rs(&db);
        static const wchar_t szSql[] = L"SELECT cardid, quantity FROM yourcards_t";
        b = rs.Open(CRecordset::forwardOnly, szSql, CRecordset::readOnly);
        while (!rs.IsEOF())
        {
            m_YourCards.insert(CardQuantity_t(rs.m_cardid, rs.m_quantity));
            rs.MoveNext();
        }
        rs.Close();
        db.Close();
        LogAlways(L"ReadYourCards (%d)", m_YourCards.size());
        Services::GetCardSet().SetCardPointers(m_YourCards);
        return true;
    }
    catch(CDBException* pdbe)
    {
        LogError(L"ReadYourCards() exception: %ls", pdbe->m_strError);
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////

void
LonPlayer_t::
ShowYourCards(
    const Flag_t Flags,
    size_t       Order) const
{
    m_YourCards.Show(Flags, Order, L"YourCards");
}

///////////////////////////////////////////////////////////////////////////////
