/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// CmdCards.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LonTrader_t.h"
#include "CommonTypes.h"
#include "LonCardSet_t.h"
#include "Log.h"
#include "TradeManager_t.h"
#include "LonPlayer_t.h"

/////////////////////////////////////////////////////////////////////////////

bool
LonTrader_t::
CmdCards(
    const wchar_t* pszCmd)
{
    Flag_t ShowFlag = 0;
    bool bDetail = false;
    size_t Pos = 0;

    switch (pszCmd[Pos++])
    {
    case L'c': // compare count
//        LogAlways(L"CompareCount (%d)", LonCard_t::CompareCount = 0);
        break;

    case L'd': // detail
        bDetail = true;
        ShowFlag = LonCardSet_t::Flag::ShowDetail;
        // fall through
    case L'i': // info 
        {
            if (L'\0' == pszCmd[Pos])
            {
                LogAlways(L"No default command");
                return true;
            }
            // TODO: if isnum(Cmd[1]) look up by ID?
            // TODO: if 0 == cmd[1] show all cards?
            size_t Order = 0;
            if (L'v' == pszCmd[Pos]) // valued cards
            {
                ++Pos;
                ShowFlag = LonCardSet_t::Flag::SortByHighBid;
                if (L'b' == pszCmd[Pos])
                    ShowFlag = LonCardSet_t::Flag::SortByHighBid;
                else if (L'a' == pszCmd[Pos])
                    ShowFlag = LonCardSet_t::Flag::SortByLowAsk;
                GetCardSet().ShowCards(true, bDetail, ShowFlag, Order);
                return true;
            }
            if (L'x' == pszCmd[Pos]) // transaction cards (cards which have been Bought or Sold)
            {
                ++Pos;
                if (L'b' == pszCmd[Pos])
                    ShowFlag |= LonCardSet_t::Flag::SortByBoughtCount;
                else if (L's' == pszCmd[Pos])
                    ShowFlag |= LonCardSet_t::Flag::SortBySoldCount;
                GetCardSet().ShowTransactions(ShowFlag);
                return true;
            }
            if (0 == wcsncmp(L"all", &pszCmd[Pos], 3))
            {
                Pos += 3;
                ShowFlag = 0;
                if (L'v' == pszCmd[Pos])
                    ShowFlag = LonCardSet_t::Flag::SortByHighBid;
                if (L'o' == pszCmd[Pos + 1])
                    Order = _wtoi(&pszCmd[Pos + 2]);
                GetCardSet().ShowCards(false, bDetail, ShowFlag, Order);
                return true;
            }
            if (0 == wcscmp(L"new", &pszCmd[Pos]))
            {
                GetCardSet().ShowNewCards(bDetail);
                return true;
            }
            if (0 == wcscmp(L"your", &pszCmd[Pos]))
            {
                using namespace ShowFlags;
                GetPlayer().ShowYourCards(ValuedOnly | (bDetail ? Detail : 0) );
                return true;
            }

            if (L'o' == pszCmd[Pos])
            {
                ++Pos;
                Order = _wtoi(&pszCmd[Pos++]);
            }
            // allow colon to precede card name to avoid above commands
            if (L':' == pszCmd[Pos])
                ++Pos;
            // match partial name to any pos of card name
            const LonCard_t* pCard =
                static_cast<const LonCard_t*>
                    (GetCardSet().LookupPartialAtAnyPos(&pszCmd[Pos]));
            if (NULL != pCard)
            {
                ShowFlag = bDetail ? LonCard_t::Flag::ShowDetail : 0;
                pCard->ShowValue(ShowFlag, Order);
                return true;
            }
            LogAlways(L"No card found (%ls)", &pszCmd[Pos]);
        }
        break;

    case L'l': // lookup (match partial card name anywhere in card name)
        {
            const LonCard_t* pCard =
                static_cast<const LonCard_t*>
                    (GetCardSet().LookupPartialAtAnyPos(&pszCmd[Pos]));
            if (NULL != pCard)
            {
                pCard->ShowValue(bDetail);
                return true;
            }
            LogAlways(L"No card found (%ls)", &pszCmd[Pos]);
        }
        break;

    case L'r': // read
        if (0 == wcsncmp(L"your", &pszCmd[Pos], 4))
        {
            Pos += 4;
            if (L'\0' == pszCmd[1])
            {
                LogAlways(L"No filename specified.");
                return true;
            }
            SetDbName(&pszCmd[Pos]);
            GetPlayer().ReadYourCards();
            return true;
        }
        break;

    case L'v': // value
        {
            if (L'\0' == pszCmd[1])
            {
                LogAlways(L"No default command");
                return true;
            }
            // TODO: if isnum(Cmd[1]) look up by ID?
            if (0 == wcscmp(L"all", &pszCmd[1]))
            {
                GetCardSet().ValueCards();
                return true;
            }
            const Card_t* pCard = GetCardSet().LookupPartialAtAnyPos(&pszCmd[1]);
            if (NULL != pCard)
            {
                GetCardSet().ValueCards(0, pCard);
                return true;
            }
            LogAlways(L"No card found (%ls)", &pszCmd[1]);
        }
        return true;

    case L'w': // write
        {
            if (L'\0' == pszCmd[Pos])
            {
                LogAlways(L"No default command");
                return true;
            }
            // TODO: if isnum(Cmd[1]) look up by ID?
            // TODO: if 0 == cmd[1] show all cards?
            if (0 == wcscmp(L"all", &pszCmd[Pos]))
            {
                LogAlways(L"Write All Cards: Not Implemented");
                return true;
            }
            if (0 == wcsncmp(L"new", &pszCmd[Pos], 3))
            {
                Pos += 3;
                const wchar_t* pDbName = &pszCmd[Pos];
                if (L'\0' != *pDbName)
                {
                    CopyDb(pDbName);
                    SetDbName(pDbName);
                }
                GetCardSet().WriteNewCards();
                return true;
            }
            if (0 == wcsncmp(L"your", &pszCmd[Pos], 4))
            {
                Pos += 4;
                const wchar_t* pDbName = &pszCmd[Pos];
                if (L'\0' != *pDbName)
                {
                    CopyDb(pDbName);
                    SetDbName(pDbName);
                }
                GetPlayer().WriteYourCards();
                return true;
            }
            if (0 == wcscmp(L"xml", &pszCmd[Pos]))
            {
                GetCardSet().WriteXml(L"cards.xml");
                return true;
            }
        }
        break;

    case L'x': // transactions
        {
            if (L'\0' == pszCmd[Pos])
            {
                LogAlways(L"No default command");
                return true;
            }
            if (0 == wcscmp(L"init", &pszCmd[Pos]))
            {
                GetTradeManager().HackInitTransactions();
                return true;
            }

            // match partial name to any pos of card name
            const LonCard_t* pCard = 
                (const LonCard_t*)GetCardSet().LookupPartialAtAnyPos(&pszCmd[Pos]);
            if (NULL != pCard)
            {
                pCard->ShowTransactions(LonCard_t::Flag::ShowDetail);
                return true;
            }
            LogAlways(L"No card found (%ls)", &pszCmd[Pos]);
        }
        break;

    default:
        break;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////
