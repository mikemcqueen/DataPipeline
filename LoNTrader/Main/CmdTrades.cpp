///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// CmdTrades.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LonTrader_t.h"

#include "TiPostedTrades.h"
#include "DcrPostedTrades.h"
#include "PostedTrades.h"

#include "TradeManager_t.h"
#include "TradeMaker_t.h"
#include "Timer_t.h"
#include "LonCardSet_t.h"
#include "Services.h"
#include "TradePoster.h"
#include "PassiveFixedPricing_t.h"
#include "SimpleRangePricing_t.h"
#include "SimplePosting_t.h"
#include "Log.h"
#include "LonCard_t.h"
#include "TradeExecutor_t.h"
#include "LonPlayer_t.h"
#include "CardCollection_t.h"

#if _MSC_VER > 1000
#pragma warning(push)
#pragma warning(disable:4245) // signed/unsigned mismatch in boost
#endif

#include "boost/filesystem.hpp"
#include "boost/regex/v4/fileiter.hpp"

#if _MSC_VER > 1000
#pragma warning(pop)
#endif

using namespace std;

///////////////////////////////////////////////////////////////////////////////

void
ShowSet(const CardTradesSet_t& Set)
{
    CardTradesSet_t::const_iterator it(Set.begin());
    for (; Set.end() != it; ++it)
    {
        const Card_t* pCard = it->pCard;
        LogAlways(L"%65ls", "-");
        LogAlways(L"%5d,\"%ls\"", pCard->GetId(), pCard->GetName());
        LogAlways(L"%65ls", "-");
        const TradePtrVector_t& o = it->Offered;
        LogAlways(L"      Offered=%d", o.size());
        TradePtrVector_t::const_iterator ptr(o.begin());
        for (; o.end() != ptr; ++ptr)
        {
            const Trade_t* pTrade = *ptr;
            pTrade->Show();
        }

        const TradePtrVector_t& w = it->Wanted;
        LogAlways(L"      Wanted=%d", w.size());
        TradePtrVector_t::const_iterator wit(w.begin());
        for (; w.end() != wit; ++wit)
        {
            const Trade_t* pTrade = *wit;
            pTrade->Show();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void
BuildTrade(
    Trade_t& Trade)
{
    static const
    struct temp
    {
        wchar_t* name;
        size_t   count;
    }
    Offered[] = {
        L"2M1 Trakanon",          1,
        L"1C56 Wind Walk",            3,
        L"1U108 Aery Stalker",        2,
    },
    Want[] = {
        L"1R21 Infernal Sacrament",   1,
        L"1U22 Instigate",            1,
//        L"Kiss of Erollisi Marr",                     3,
//        L"2F2 Swooping Dragon",                       2,
    };
    size_t Card;
    for (Card = 0; _countof(Offered) > Card; ++Card)
    {
        const Card_t* pCard = Services::GetCardSet().Lookup(Offered[Card].name);
        if (NULL == pCard)
            ASSERT(false);
        else
            Trade.AddOfferCard(pCard, Offered[Card].count);
    }
    for (Card = 0; _countof(Want) > Card; ++Card)
    {
        const Card_t* pCard = Services::GetCardSet().Lookup(Want[Card].name);
        if (NULL == pCard)
            ASSERT(false);
        else
            Trade.AddWantCard(pCard, Want[Card].count);
    }
}

///////////////////////////////////////////////////////////////////////////////

void
GetTradeIds(
    const wchar_t*      pszText,
          TradeIdSet_t& TradeIds)
{
    static const wchar_t szDelim[] = L", ";
#pragma warning(disable:4996) // wcstok unsafe
    wchar_t* pszToken = wcstok(const_cast<wchar_t *>(pszText), szDelim);
    while (NULL != pszToken)
    {
        TradeId_t TradeId = _wtoi(pszToken);
        if (0 != TradeId)
            TradeIds.insert(TradeId);
        pszToken = wcstok(NULL, szDelim);
    }
}

///////////////////////////////////////////////////////////////////////////////

/*
void
PostXmlTrades(
    const wchar_t* pText,
          bool     bTest)
{
    size_t Pos = 0;
    switch (pText[Pos++])
    {
    case L'd': // directory
        LogAlways(L"Not implemented.");
        break;
    case L'f': // file
        {
            Trade_t Trade;
            if (!Trade.ReadXmlFile(&pText[Pos]))
                break;
            Trade.Show();
            Services::GetTradeExecutor().PostTrade(Trade, bTest, 0);
        }
        break;
    case L'l': // list
        LogAlways(L"Not implemented.");
        break;
    default:
        LogAlways(L"PostXmlTrades: Unknown option");
        break;
    }
}
*/

///////////////////////////////////////////////////////////////////////////////

void
ReadTradeDbs(
    const wchar_t* pszDirectory)
{
    size_t Count = 0;
    Timer_t Timer(L"ReadTradeDbs()");

    using namespace boost::filesystem;

    wpath Directory(pszDirectory);
    const wdirectory_iterator itEnd;
    LogAlways(L"  Directory: %ls", Directory.string().c_str());
    // loop through each file in the directory
    for (wdirectory_iterator it(Directory); itEnd != it; ++it)
    {
        LogAlways(L"  File: %ls", it->path().string().c_str());
        // skip directories
        if (is_directory(it->status()))
        {
            LogAlways(L"    Skipping directory");
            continue;
        }
        if (extension(it->path()) != L".mdb")
        {
            LogAlways(L"    Skipping non-mdb file");
            continue;
        }
        LonTrader_t::SetDbName(it->path().string().c_str());
        CDatabase db;
        try
        {
            BOOL b = db.Open(NULL, FALSE, FALSE, LonTrader_t::GetDbConnectString());
            ASSERT(!!b);
            Count = Services::GetTradeManager().ReadTrades(db, TradeManager_t::Flag::RemovedTrades);
            LogAlways(L"  Read (%d) from (%s)", Count, it->path().string().c_str());
        }
        catch(CDBException *e)
        {
            LogError(L"  ReadTradeDbs() exception: %ls", e->m_strError);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

bool
LonTrader_t::
CmdTrades(
    const wchar_t* pszCmd)
{
    static const wchar_t szNoTestSuffix[] = L"***";
    static const size_t  SuffixLen        = wcslen(szNoTestSuffix);

    bool bTest = true;
    size_t Len = wcslen(pszCmd);
    if ((SuffixLen < Len) &&
        (0 == wcscmp(&pszCmd[Len - SuffixLen], szNoTestSuffix)))
    {
        ((wchar_t*)pszCmd)[Len - SuffixLen] = L'\0';
        bTest = false;
    }
    size_t Pos = 0;
    switch (pszCmd[Pos++])
    {
    case L'b': // buy trade #
        if (L'\0' == pszCmd[Pos])
        {
#if 1
            // TODO: Convert to use SendEvent or just call TradeExector().BuyTrade().
            // It may just work.
#else
            LonWindow_t::Handle_t Top = m_Window.GetTopWindow();
            if (LonWindow_t::PostedTradeDetailWindow == Top.Type)
            {
                GetTiPostedTradeDetail().BuyTrade(0);
                return true;
            }
#endif
            LogError(L"No TradeId specified.");//"Only works on the trade detail window.");
        }
        else
        {
            TradeId_t TradeId = _wtoi(&pszCmd[Pos]);
            if (!GetTradeManager().FindTrade(TradeId))
                LogError(L"Trade not found (%d)", TradeId);
            else
            {
                GetTradeExecutor().BuyTrade(TradeId, bTest);
                return true;
            }
        }
        break;

    case L'd': // delete trade from trade manager, not from UI
        {
            TradeIdSet_t TradeIds;
            GetTradeIds(&pszCmd[Pos], TradeIds);
            if (!TradeIds.empty())
                GetTradeManager().RemoveTrades(TradeIds);
            return true;
        }
        break;
/*
    case L'f': // find trades with partial card name
        {
            const Card_t* pCard = GetCardSet().LookupPartial(&pszCmd[Pos]);
            if (NULL != pCard)
            {
                pCard->Show();
                CardTradesSet_t Set;
                Timer_t Timer;
//                size_t Count = GetTradeManager().FindTrades(pCard, Set);
                Timer.Show();
                if (0 < Count)
                {
                    ShowSet(Set);
                }
                else
                    LogAlways(L"No trades found.");
                return true;
            }
            else
                LogAlways(L"No card found (%ls)", &pszCmd[Pos]);
        }
        break;
*/
    case L'i': // info
        {
            DWORD Id = _wtoi(&pszCmd[Pos]);
            if (0 < Id)
            {
                GetTradeManager().ShowTrade(Id);
                return true;
            }
            LogError(L"Missing or invalid Id");
        }
        break;

    case L'm': // (make) trade poster
        return CmdTradePoster(&pszCmd[Pos], bTest);

    case L'p': // post new trade
        {
            if (L'\0' == pszCmd[Pos])
            {
                // Create a bogus trade and post it.
                Trade_t Trade;
                BuildTrade(Trade);
                GetTradeExecutor().PostTrade(Trade, 0, 0, false);
            }
/*  // see tmxr/tmtp
            else if (L'x' == pszCmd[Pos])
            {
                // post from xml
                PostXmlTrades(&pszCmd[++Pos], bTest);
            }
*/
            else
            {
                // post a copy of an existing trade
                bool bPosted = false;
                TradeId_t TradeId = _wtoi(&pszCmd[Pos]);
                if (0 < TradeId)
                    bPosted = GetTradeManager().TestPostTrade(TradeId);
                if (!bPosted)
                    LogError(L"Trade not found (%d)", TradeId);
            }
        }
        return true;

    case L'r': // remove/read
        if (L'd' == pszCmd[Pos]) // read (removed) trades from directory
        {
            ReadTradeDbs(&pszCmd[++Pos]);
            return true;
        }
        else if (L'x' == pszCmd[Pos]) // read xml
        {
            Trade_t Trade;
            if (Trade.ReadXmlFile(&pszCmd[++Pos]))
                Trade.Show();
            return true;
        }
        else
        {
#if 1
            TradeIdSet_t TradeIds;
            GetTradeIds(&pszCmd[Pos], TradeIds);
            if (TradeIds.empty())
            {
                LogError(L"No TradeIds specified");
                break;
            }
            GetTradeExecutor().RemoveTrades(TradeIds, bTest);
#else
            TradeId_t TradeId = _wtoi(&pszCmd[Pos]);
            if (!GetTradeManager().FindTrade(TradeId))
                LogError(L"Trade not found (%d)", TradeId);
            else
            {
                GetTradeExecutor().RemoveTrade(TradeId, bTest);
            }
#endif
            return true;
        }
        break;

    case L's': // state (of TiTrades_t)
//        m_TiPostedTrades.ShowState(L"State", true);
        GetPostedTrades().GetInterpreter().ShowState(L"State", true);
        return true;

    case L't': // totals/timer
//        m_TiPostedTrades.ShowMissedTradeNumbers();
        GetPostedTrades().GetInterpreter().ShowMissedTradeNumbers();
        GetTradeManager().ShowTradeTotals();
#if 0
        GetTradeManager().m_Timer.Show();
        GetTradeManager().m_Timer.Set();
#endif
        return true;

    case L'v': // value
        GetTradeManager().ValueAllTrades();
        return true;

    case L'w': // write
        if (L'\0' == pszCmd[Pos])
        {
            LogError(L"Trade Write: No default.");//"Only works on the trade detail window.");
        }
        else if (0 == wcsncmp(&pszCmd[Pos], L"all", 3))
        {
            Pos += 3;
            const wchar_t* pDbName = &pszCmd[Pos];
            if (L'\0' != *pDbName)
            {
                if (!CopyDb(pDbName))
                    break;
                SetDbName(pDbName);
            }
            GetTradeManager().WriteAllTrades();
        }
        else
        {
            std::wstring Args(&pszCmd[Pos]);
            size_t CommaPos = Args.find(L',');
            const wchar_t* pFilename = NULL;
            if (Args.npos != CommaPos)
                pFilename = &Args[CommaPos + 1];
            TradeId_t TradeId = _wtoi(&pszCmd[Pos]);
            GetTradeManager().WriteTradeXml(TradeId, pFilename);
        }
        return true;

//    case L'0':
    case L'1':
    case L'2':
    case L'3': // show
        GetTradeManager().ShowTrades(_wtoi(&pszCmd[Pos]));
        return true;
    default:
        break;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
