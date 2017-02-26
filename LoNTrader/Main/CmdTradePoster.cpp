///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// CmdTradePoster.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LonTrader_t.h"
#include "Timer_t.h"
#include "LonCardSet_t.h"
#include "LonCard_t.h"
#include "TradeMaker_t.h"
#include "TradePoster.h"
#include "SimpleRangePricing_t.h"
#include "SimplePosting_t.h"
#include "Log.h"

#include "TiPostedTrades.h"
#include "DcrPostedTrades.h"
#include "PostedTrades.h"

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

bool
LonTrader_t::
CmdTradePoster(
    const wchar_t* pszCmd,
          bool     bTest)
{
    if (0 == wcscmp(pszCmd, L"list"))
    {
        GetTradePoster().ShowAll(TradePoster::Flag::ShowBuyTrades |
                                 TradePoster::Flag::ShowSellTrades);
        return true;
    }

    bool bBuy = false;
    bool bDetail = false;
    size_t Pos = 0;
    switch (pszCmd[Pos++])
    {
    case L'c': // card
        return CmdTradePosterCard(&pszCmd[Pos]);

    case L'd': // details - Show(true)
        bDetail = true;
        // fall through
    case L'i': // info    - Show(false)
        {
            TradePoster::Id_t Id;
            size_t Value;
            if (!CmdTradePosterGetArgs(&pszCmd[Pos], Id, Value))
                break;
            if (0 == Id)
                GetTradePoster().Show(bDetail);
            else if (0 == Value)
                GetTradePoster().Show(Id, bDetail);
            else
                GetTradePoster().Show(Id, Value, bDetail);
            return true;
        }
        break;

    case L'b': // buy
        bBuy = true;
        // fall through
    case L's': // sell
        return CmdTradePosterBuySellCard(bBuy, &pszCmd[Pos], bTest);

    case L'p': // price - format tmp
        return CmdTradePosterPrice(&pszCmd[Pos]);

    case L'r': // remove a data_t entry - format tmr
        {
            TradePoster::Id_t Id = _wtoi(&pszCmd[Pos]);
            if (0 == Id)
            {
                LogError(L"Id = 0");
                break;
            }
            bool bRemoved = GetTradePoster().Remove(Id);
            LogAlways(L"Removed(%d)", bRemoved);
            return bRemoved;
        }
        break;

    case L't': // trade - format tmt
        return CmdTradePosterTrade(&pszCmd[Pos], bTest);

    case L'v': // value - format tmv
        return CmdTradePosterValue(&pszCmd[Pos], bTest);

    case L'w': // (waitable) timer control
        switch (pszCmd[Pos++])
        {
        case L's': //set
            {
                size_t Seconds = _wtoi(&pszCmd[Pos]);
                if (0 == Seconds)
                {
                    LogError(L"Seconds == 0");
                    break;
                }
                GetTradePoster().QueueSetTimer(Seconds);
                return true;
            }
            break;
        default:
            break;
        }
        break;

    case L'x': // xml - format: tmx
        switch (pszCmd[Pos++])
        {
        case L'r':
            // Read Xml 
            // Format: tmxrFile.xml
            //         tmxrDir\*.xml
            return 0 < ReadTradeXmls(&pszCmd[Pos]);

        case L'w':
            // Write Xml
            // Format: tmxwall<Dir>
            //         tmxw<Id>,<Dir>
            {
                if (0 == wcsncmp(&pszCmd[Pos], L"all", 3))
                {
                    Pos += 3;
                    wstring strDir(&pszCmd[Pos]);
                    if (strDir.empty())
                    {
                        LogError(L"Missing directory name");
                        break;
                    }
                    WriteTradeXmls(strDir.c_str());
                    return true;
                }
                // Id
                wstring strId(&pszCmd[Pos]);
                size_t CommaPos = strId.find(L',');
                if (strId.npos == CommaPos)
                {
                    LogError(L"Missing comma following Id");
                    return false;
                }
                wstring strDir(&strId[CommaPos + 1]);
                strId.erase(CommaPos);
                TradePoster::Id_t Id = _wtoi(strId.c_str());
                if (0 == Id)
                {
                    LogError(L"Id == 0");
                    return false;
                }
                return GetTradePoster().WriteXmlDirectory(strDir.c_str(), Id);
            }
            break;
        default:
            break;
        }
        break;
    default:
        LogError(L"Can't parse: '%ls'", pszCmd);
        break;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool
LonTrader_t::
CmdTradePosterGetArgs(
    const wchar_t*      pszCmd,
    TradePoster::Id_t&  Id,
    size_t&             Value,
    wstring*            pstrOtherArgs)
{
    wstring strId(pszCmd);
    wstring strValue;

    // Id
    size_t CommaPos = strId.find(L',');
    if (strId.npos != CommaPos)
    {
        strValue.assign(&strId[CommaPos + 1]);
        strId.erase(CommaPos);
    }
    Id = 0;
    if (0 != strId.compare(L"all"))
    {    
        Id = _wtoi(strId.c_str());
        if (0 == Id)
        {
            LogError(L"TpGetArgs Id = 0: '%ls'", pszCmd);
            return false;
        }
    }

    // Value
    CommaPos = strValue.find(L',');
    if (strValue.npos != CommaPos)
    {
        if (NULL != pstrOtherArgs)
            pstrOtherArgs->assign(&strId[CommaPos + 1]);
        else
            LogWarning(L"Ignoring extra arguments: %ls", &strId[CommaPos + 1]);
        strValue.erase(CommaPos);
    }
    Value = 0;
    if (!strValue.empty() && (0 != strValue.compare(L"all")))
    {
        Value = _wtoi(strValue.c_str());
        if (0 == Value)
        {
            LogError(L"TpGetArgs Value = 0");
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
// tmc*
//

bool
LonTrader_t::
CmdTradePosterCard(
    const wchar_t* pszCmd)
{
    bool bAdd = false;
    size_t Pos = 0;
    switch (pszCmd[Pos++])
    {
    case L'a': // add
        bAdd = true;
        // fall through
    case L'r': // remove
        {
            // Format: tmc[a|r]Id,CardName,Quantity
            std::wstring strId(&pszCmd[Pos]);
            size_t CommaPos = strId.find(L',');
            if (strId.npos == CommaPos)
            {
                LogError(L"Missing comma after Id");
                break;
            }
            std::wstring strCard(&strId[CommaPos + 1]);
            strId.erase(CommaPos);
            TradePoster::Id_t Id = _wtoi(strId.c_str());
            if (0 == Id)
            {
                LogError(L"Id = 0");
                break;
            }
            if (strCard.empty())
            {
                LogError(L"Missing card name");
                break;
            }
            size_t Quantity = 1;
            CommaPos = strCard.find(L',');
            if (strCard.npos != CommaPos)
            {
                Quantity = _wtoi(&strCard[CommaPos + 1]);
                if (0 == Quantity)
                {
                    LogError(L"Quantity == 0");
                    break;
                }
                strCard.erase(CommaPos);
            }
            TradePoster::SimplePosting_t* pPosting =
                (TradePoster::SimplePosting_t*)GetTradePoster().GetPostingPolicy(Id);
            if (NULL == pPosting)
            {
                LogError(L"No posting policy.");
                break;
            }
#if 0
            if (0 == strCard.compare(L"your"))
            {
                if (!bAdd)
                {
                    LogError(L"Remove YourCards not supported.");
                    break;
                }
                // Add YourCards.
                const CardCollection_t& Cards = GetPlayer().GetYourCards();
                CardCollection_t::const_iterator it = Cards.begin();
                for(; Cards.end() != it; ++it)
                {
                    const LonCard_t* pCard = (LonCard_t*)it->pCard;
                    if (!pCard->IsLootCard() || (0 == pCard->GetTransactionCount()))
                        continue;
                    pPosting->AddAllowedCard(CardQuantity_t(pCard,0));
                }
                return true;
            }
#endif
            const LonCard_t* pCard =
                static_cast<const LonCard_t*>
                    (GetCardSet().LookupPartialAtAnyPos(strCard.c_str()));
            if (NULL == pCard)
            {
                LogError(L"Card not found (%s)", strCard.c_str());
                break;
            }
            if (bAdd)
                pPosting->AddAllowedCard(CardQuantity_t(pCard, Quantity));
            else
                pPosting->RemoveAllowedCard(CardQuantity_t(pCard, Quantity));
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
//
// tmt*
//

bool
LonTrader_t::
CmdTradePosterTrade(
    const wchar_t* pszCmd,
          bool     bTest)
{
    size_t Pos = 0;
    switch (pszCmd[Pos++])
    {
    case L'g':
        {   // Generate Trades
            // Format: tmtgall           - generate trades for all values of all Ids
            // Format: tmtg<Id>          - generate trades for all values of one Id
            // Format: tmtg<Id>,<Value>  - generate trades for one value of one Id
            const size_t MaxTrades = 1;
            if (0 == wcscmp(&pszCmd[Pos], L"all"))
            {
                GetTradePoster().GenerateTrades(MaxTrades);
                return true;
            }
            TradePoster::Id_t Id = 0;
            size_t Value = 0;
            if (!CmdTradePosterGetArgs(&pszCmd[Pos], Id, Value))
                break;
            if (0 == Value)
                GetTradePoster().GenerateTrades(Id, MaxTrades);
            else
                GetTradePoster().GenerateTrades(Id, Value, MaxTrades);
            return true;
        }
        break;

    case L'r':
        {   // Remove Trades
            // Format: tmtr<Id>[,all]
            //         tmtr<Id>,<Value>
            //  TODO:  tmtr<Id>,<Value>,<TradeId>
            TradePoster::Id_t Id = 0;
            size_t Value = 0;
            if (!CmdTradePosterGetArgs(&pszCmd[Pos], Id, Value))
                break;
            TradeId_t TradeId = 0;
            size_t Count = 0;
            if (0 == Value)
                Count = GetTradePoster().RemoveTrades(Id, bTest);
            else if (0 == TradeId)
                Count = GetTradePoster().RemoveTrades(Id, Value, bTest);
            else
                Count = GetTradePoster().RemoveTrade(Id, Value, TradeId, bTest);

            if (0 < Count)
                LogAlways(L"Trades removes queued (%d)", Count);
            else
                LogWarning(L"No trades removed.");
            return true;
        }
        break;

    default:
        break;
    }
    return false;

#if 0
    if (L'c' == pszCmd[Pos])      // clear
    {
        //LogError(L"Not implemented.");
        //break;
        TradePoster::Id_t Id = _wtoi(&pszCmd[++Pos]);
        if (0 == Id)
        {
            LogError(L"Id = 0");
            break;
        }
        TradePoster::SimplePosting_t* Posting =
            (TradePoster::SimplePosting_t*)GetTradePoster().GetPostingPolicy(Id);
        if (NULL == Posting)
        {
            LogError(L"No posting policy.");
            break;
        }
        Posting->ClearTrades();
        return true;
    }
    else
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
// Format: tmpId,Low,High,Step,Current
//

bool
LonTrader_t::
CmdTradePosterPrice(
    const wchar_t* pszCmd)
{
    // Id
    wstring strId(pszCmd);
    size_t CommaPos = strId.find(L',');
    if (strId.npos == CommaPos)
    {
        LogError(L"Missing comma after Id");
        return false;
    }
    wstring strLow(&pszCmd[CommaPos + 1]);
    strId.erase(CommaPos);
    TradePoster::Id_t Id = _wtoi(strId.c_str());
    if (0 == Id)
    {
        LogError(L"Id = 0");
        return false;
    }

    // Low
    CommaPos = strLow.find(L',');
    if (strLow.npos == CommaPos)
    {
        LogError(L"Missing comma after Low");
        return false;
    }
    wstring strHigh(&strLow[CommaPos + 1]);
    strLow.erase(CommaPos);
    size_t Low = _wtoi(strLow.c_str());
    if (0 == Low)
    {
        LogError(L"Low = 0");
        return false;
    }

    // High
    CommaPos = strHigh.find(L',');
    if (strHigh.npos == CommaPos)
    {
        LogError(L"Missing comma after High");
        return false;
    }
    wstring strIncrement(&strHigh[CommaPos + 1]);
    strHigh.erase(CommaPos);
    size_t High = _wtoi(strHigh.c_str());
    if (0 == High)
    {
        LogError(L"High == 0");
        return false;
    }

    // Increment
    CommaPos = strIncrement.find(L',');
    if (strIncrement.npos == CommaPos)
    {
        LogError(L"Missing comma after Increment");
        return false;
    }
    wstring strCurrent(&strIncrement[CommaPos + 1]);
    strIncrement.erase(CommaPos);
    size_t Increment = _wtoi(strIncrement.c_str());
    if (0 == Increment)
    {
        LogError(L"Increment == 0");
        return false;
    }

    // Current
    size_t Current = _wtoi(strCurrent.c_str());
    if (0 == Current)
    {
        LogError(L"Current == 0");
        return false;
    }

    using namespace TradePoster;
    PricingPolicy_i* pPricing = GetTradePoster().GetPricingPolicy(Id);
    if (NULL == pPricing)
    {
        LogError(L"No Pricing policy (%d)", Id);
        return false;
    }
    PostingPolicy_i* pPosting = GetTradePoster().GetPostingPolicy(Id);
    if (NULL == pPosting)
    {
        LogError(L"No Posting policy (%d)", Id);
        return false;
    }
    if (0 != wcscmp(pPricing->GetName(), SimpleRangePricing_t::PolicyName))
    {
        LogError(L"Policy (%ls) not supported", pPricing->GetName());
        return false;
    }
//    pPosting->RemoveAllTrades(Id, pPricing->GetPrice(), 0);
    pPosting->EraseAllTrades();
    SimpleRangePricing_t* pSrp = static_cast<SimpleRangePricing_t*>(pPricing);
    pSrp->SetPrice(Low, High, Increment, Current);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
// Format: tm[b|s]*
//
 
bool
LonTrader_t::
CmdTradePosterBuySellCard(
          bool     bBuy,
    const wchar_t* pszCmd,
          bool     bTest)
{
    size_t Pos = 0;

    if (0 == wcscmp(&pszCmd[Pos], L"list"))
    {
        if (bBuy)
            GetTradePoster().ShowAll(TradePoster::Flag::ShowBuyTrades);
        else
            GetTradePoster().ShowAll(TradePoster::Flag::ShowSellTrades);
        return true;
    }
    else if (L'x' == pszCmd[Pos]) // execute buy/sell trade #
    {
        ++Pos;
        size_t Count = 0;
        if (0 == wcscmp(&pszCmd[Pos], L"all"))
        {
            Count = GetTradePoster().PostAllTradesAtDesiredValues(bTest);
        }
        else
        {
#if 0
            LogError(L"TODO: Only 'all' implemented");
            return false;
#else
            // Format: tmbxId[,Value]
            std::wstring strId(&pszCmd[Pos]);
            size_t CommaPos = strId.rfind(L',');
            size_t Value = 0;
            if (strId.npos != CommaPos)
            {
                Value = _wtoi(&strId[CommaPos + 1]);
                strId.erase(CommaPos);
            }
            TradePoster::Id_t Id = _wtoi(strId.c_str());
            Count = GetTradePoster().PostTrades(Id, Value, 0, bTest);
#endif
        }
        LogAlways(L"Trades queued (%d)", Count);
        return true;
    }
    else if (L'r' == pszCmd[Pos]) // remove a card from buy/sell trades list
    {
        LogError(L"TODO: Not implemented");
        return false;
        /*
        std::wstring strCard(&pszCmd[++Pos]);
        const LonCard_t* pCard = (LonCard_t*)GetCardSet().LookupPartialAtAnyPos(strCard.c_str());
        if (NULL != pCard)
        {
        if (bBuy)
        GetTradeMaker().RemoveBuyTradesWithCard(pCard);
        else
        GetTradeMaker().RemoveSellTradesWithCard(pCard);
        return true;
        }
        */
    }
    else
    {
        // Default - add new TradePoster buy/sell entry.
        // Format: tm[b|s]CardName,Low,High,Increment[,Count]

        // CardName
        wstring strCard(&pszCmd[Pos]);
        size_t CommaPos = strCard.find(L',');
        if (strCard.npos == CommaPos)
        {
            LogError(L"Missing comma after CardName");
            return false;
        }
        wstring strLow(&strCard[CommaPos + 1]);
        strCard.erase(CommaPos);

        // Low
        CommaPos = strLow.find(L',');
        if (strLow.npos == CommaPos)
        {
            LogError(L"Missing comma after Low");
            return false;
        }
        wstring strHigh(&strLow[CommaPos + 1]);
        strLow.erase(CommaPos);
        size_t Low = _wtoi(strLow.c_str());
        if (0 == Low)
        {
            LogError(L"Low = 0");
            return false;
        }

        // High
        CommaPos = strHigh.find(L',');
        if (strHigh.npos == CommaPos)
        {
            LogError(L"Missing comma after High");
            return false;
        }
        wstring strIncrement(&strHigh[CommaPos + 1]);
        strHigh.erase(CommaPos);
        size_t High = _wtoi(strHigh.c_str());
        if (0 == High)
        {
            LogError(L"High == 0");
            return false;
        }

        // Increment
        wstring strCount;
        CommaPos = strIncrement.find(L',');
        if (strIncrement.npos != CommaPos)
        {
            strCount.assign(&strIncrement[CommaPos + 1]);
            strIncrement.erase(CommaPos);
        }
        size_t Increment = _wtoi(strIncrement.c_str());
        if (0 == Increment)
        {
            LogError(L"Increment == 0");
            return false;
        }

        // Count
        size_t Count = 1;
        if (!strCount.empty())
        {
            Count = _wtoi(strCount.c_str());
            if (0 == Count)
            {
                LogError(L"Count == 0");
                return false;
            }
        }

        const LonCard_t* pCard = static_cast<const LonCard_t*>
            (GetCardSet().LookupPartialAtAnyPos(strCard.c_str()));
        if (NULL != pCard)
        {
            if (bBuy)
                GetTradePoster().BuyCard(pCard, Low, High, Increment, Count);
            else
                GetTradePoster().SellCard(pCard, Low, High, Increment, Count);
            return true;
        }
        LogError(L"No card found (%s)", strCard.c_str());
        return false;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
// Format: tmv*
//

bool
LonTrader_t::
CmdTradePosterValue(
    const wchar_t* pszCmd,
          bool     bTest)
{
bTest;
//    CardValueSet_t cvs;
    size_t Count = 0;
    size_t Pos = 0;
    bool bDetail = false;
    bool bRead = false;
    bool bAdd = false;

    if (0 == wcscmp(pszCmd, L"init"))
    {
        Count = GetTradePoster().InitDesiredCardValues();
        /*
        TradePoster::Id_t Id = _wtoi(pszCmd);
        if (0 == Id)
        {
            LogError(L"Id == 0");
            return false;
        }
        Count = GetTradePoster().InitCardValues(Id);
        */
        LogAlways(L"InitDesiredCardValues (%d)", Count);
        return true;
    }
    else switch (pszCmd[Pos++])
    {
    case 'd': // detail
        bDetail = true;
        // fall through
    case 'i': // info
        GetTradePoster().ShowCardValues();
        return true;

    case 'a': // add - Format: tmvaCardname,BuyAt,SellAt
        bAdd = true;
        // fall through
    case 's': // set - Format: tmvsCardname,BuyAt,SellAt
        {
            // CardName
            wstring strCard(&pszCmd[Pos]);
            size_t CommaPos = strCard.find(L',');
            if (strCard.npos == CommaPos)
            {
                LogError(L"Missing comma after CardName");
                break;
            }
            wstring strBuyAt(&strCard[CommaPos + 1]);
            strCard.erase(CommaPos);
            const LonCard_t* pCard =
                static_cast<const LonCard_t*>
                    (GetCardSet().LookupPartialAtAnyPos(strCard.c_str()));
            if (NULL == pCard)
            {
                LogError(L"Card not found (%s)", strCard.c_str());
                break;
            }
            CardValue_t CardValue(pCard);

            // BuyAt & SellAt
            CommaPos = strBuyAt.find(L',');
            if (strBuyAt.npos == CommaPos)
            {
                LogError(L"Missing comma after BuyAt");
                break;
            }
            wstring strSellAt(&strBuyAt[CommaPos + 1]);
            strBuyAt.erase(CommaPos);
            size_t BuyAt = _wtoi(strBuyAt.c_str());
            size_t SellAt = _wtoi(strSellAt.c_str());
            if ((0 == BuyAt) || (0 == SellAt))
            {
                LogError(L"BuyAt or SellAt == 0");
                break;
            }
            CardValue.BuyAt = BuyAt;
            CardValue.SellAt = SellAt;
            GetTradePoster().SetCardValue(CardValue);
            return true;
        }
        break;

    case L'x': // xml
        switch (pszCmd[Pos++])
        {
        case L'r': // read
            bRead = true;
        case L'w': // write
            {
                std::wstring strDir(&pszCmd[Pos]);
                if (strDir.empty() || !PathIsDirectory(strDir.c_str()))
                {
                    LogError(L"Not a directory (%ls)", strDir.c_str());
                    break;
                }
                if (bRead)
                    GetTradePoster().ReadXmlDesiredCardValues(strDir.c_str());
                else
                    GetTradePoster().WriteXmlDesiredCardValues(strDir.c_str());
            }
            return true;

        default:
            break;
        }
    default:
        break;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

size_t
LonTrader_t::
ReadTradeXmls(
    const wchar_t* pszWildcard)
{
    Timer_t Timer(L"ReadTradeXmls");
    using namespace boost::re_detail;

    char buf[MAX_PATH];
    wsprintfA(buf, "%ls", pszWildcard);

    typedef file_iterator iter_type;
    iter_type it(buf);
    const iter_type itEnd;
//    LogAlways(L"  Buf: '%hs' - Path: '%hs', Root '%hs', Name: '%hs'",
//        buf, it.path(), it.root(), it.name());
    size_t Count = 0;
    for(; itEnd != it; ++it)
    {
        wchar_t szFile[MAX_PATH];
        wsprintf(szFile, L"%hs", it.path());
//        LogAlways(L"  File: %ls", szFile);
        // HACK
        // if (0 == wcscmp_s(szFile, TradePoster::DesiredCardValuesFilename))
        //   GetTradePoster().ReadXmlDesiredCardValues(szFile);
        if (GetTradePoster().ReadXmlFile(szFile))
            ++Count;
    }
    LogAlways(L"  Reading done (%d)", Count);
    // TODO: throw if fails?
    return Count;
}

///////////////////////////////////////////////////////////////////////////////

size_t
LonTrader_t::
WriteTradeXmls(
    const wchar_t* pszDir)
{
    Timer_t Timer(L"WriteTradeXmls");
    if (FALSE == CreateDirectory(pszDir, NULL))
    {
        LogError(L"CreateDirectory(%ls) failed (%d)", pszDir, GetLastError());
        return 0;
    }
    return GetTradePoster().WriteXmlDirectory(pszDir);
}

///////////////////////////////////////////////////////////////////////////////
