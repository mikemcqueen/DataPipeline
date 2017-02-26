/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TiPostedTrades.cpp
//
// Text Interpreter for LoN Posted Trades window.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TransactionManager.h"
#include "PipelineManager.h"
#include "LonCardSet_t.h"
#include "LonTrader_t.h"
#include "TradeManager_t.h"
#include "TradeExecutor_t.h"
#include "Services.h"
#include "TiPostedTrades.h"
#include "PostedTradesTypes.h"
#include "PostedTradesPolicy.h"
#include "DpMessage.h"

using namespace Lon;

/////////////////////////////////////////////////////////////////////////////

namespace PostedTrades
{
namespace Interpret
{

static const bool EXTRALOG = false;

/////////////////////////////////////////////////////////////////////////////

Handler_t::
Handler_t(
    ManagerBase_t& Manager)
:
    m_TiTrades      (Table::CharColumnWidths, Table::ColumnCount),
    m_Manager       (Manager),
    m_State         (FindNewTrade),
    m_idLastTrade   (0),
    m_idFindTrade   (0),
    m_LastScrollType(LonWindow_t::EventScroll_t::PageDown),
    m_ScrollType    (LonWindow_t::EventScroll_t::PageDown)
{
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
ExecuteTransaction(
    const DP::Transaction::Data_t& Data)
{
    if (TopWindowType != LonWindow_t::GetTopWindow().Type)
        return S_FALSE;
    switch (Data.Id)
    {
    case Transaction::Id::BuyTrade:
        // TODO: Lock? consider MT scenarios here
        m_idFindTrade = static_cast<const TradeExecutor_t::EventBuy_t::Data_t&>(Data).TradeId;
        SetState(FindTrade);
        break;
    case Transaction::Id::RemoveTrade:
        // TODO: Lock? consider MT scenarios here
        m_idFindTrade = static_cast<const TradeExecutor_t::EventRemove_t::Data_t&>(Data).TradeId;
        SetState(FindTrade);
        break;
    default:
        return S_FALSE;
    }
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
MessageHandler(
    const DP::Message::Data_t* pData)
{
    if (Message::Id::TextTable != pData->Id)
        return S_FALSE;
    const PostedTrades::Translate::Data_t*
        pTradesData = static_cast<const PostedTrades::Translate::Data_t*>(pData);
    if (TopWindowType != pTradesData->WindowType)
        return S_FALSE;
    LogInfo(L"TiPostedTrades::Interpret(%d)", pData->Id);

    CLock lock(m_csState);
    ShowState(L"++Interpret");
    DP::Transaction::Data_t* pTransactionData = GetTransactionManager().Acquire();
    DP::TransactionManager_t::AutoRelease_t ar(pTransactionData);
    const Policy_t& Policy = GetPolicy(pTransactionData);
    if (!Policy.PreInterpret(pTransactionData))
        return S_FALSE;
    bool bSetText = true;
    if (Policy.ShouldCompareText())
    {
        size_t FirstNewLine;
        if (m_TiTrades.CompareText(pTradesData->Text, FirstNewLine))
        {
            InterpretNewText(Window::PostedTradesView, pTradesData->Text,
                             FirstNewLine);
            // This is the 'quick, check the unfinished trade at bottom of
            // window and determine if it's the trade we just posted, so we
            // can complete our PostTrade transaction right away, before
            // someone removes the trade'.
            // TODO: Need to timeout this step in case someone does happen
            // to remove it before we see it.
            if (!m_Trade.empty() && Policy.CheckTrade(m_Trade))
            {
                AddTrade(m_Trade);
                m_Trade.Clear();
                SetState(FindNewTrade);
            }
        }
        else
        {
            InterpretSameText(Window::PostedTradesView, pTradesData->Text);
            bSetText = false;
        }
        ShowState(L"  Interpreted");
    }
    if (bSetText && Policy.ShouldSetText())
        m_TiTrades.SetText(pTradesData->Text);
    GetMoreData(pTradesData->Rect);

    ShowState(L"--Interpret");
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

const Policy_t&
Handler_t::
GetPolicy() const
{
    DP::Transaction::Data_t* pData = GetTransactionManager().Acquire();
    DP::TransactionManager_t::AutoRelease_t ar(pData);
    return GetPolicy(pData);
}

/////////////////////////////////////////////////////////////////////////////

const Policy_t&
Handler_t::
GetPolicy(
    const DP::Transaction::Data_t* pData) const
{
    const
    struct TypePolicy
    {
        const Policy_t*   pPolicy;
    } PolicyArray[] =
    {
        &m_BuyTradePolicy,
        &m_GetYourCardsPolicy,
        &m_PostTradePolicy,
        &m_RemoveTradePolicy,
        &m_GatherTradesPolicy,
    };

    if (NULL != pData)
    {
        for (size_t Index = 0; Index < _countof(PolicyArray); ++Index)
        {
            if (PolicyArray[Index].pPolicy->GetTransactionId() == pData->Id)
                return *PolicyArray[Index].pPolicy;
        }
        // TODO: Error?
        LogWarning(L"TiPostedTrades: No policy found for transaction (%d)",
                   pData->Id);
    }
    return m_DefaultPolicy;
}

///////////////////////////////////////////////////////////////////////////////

void
Handler_t::
InterpretSameText(
          Window::Type_e      WindowType,
    const TiTrades_t::Text_t& Text)
{
    WindowType;
    ASSERT(Window::PostedTradesView == WindowType);

    const ScrollBar_t::ThumbPosition_e
        ThumbPos = LonWindow_t::GetThumbPosition(
                       Window::PostedTradesVScroll, 
                       ScrollBar_t::Vertical);     
    if (ScrollBar_t::Middle == ThumbPos)
        return;

    // TODO: AddPartialTrade
    // Arbitrary.. wait 1 second.
    if (5 > m_TiTrades.GetSameTextCount())
        return;

    // HACK: We got "same text" more than the allowed # of times in a row.
    // Assume we're at the bottom of the trade screen, waiting for new
    // trades to be added. If the current trade is non-empty, consider
    // it completed, and add it.
    if (!m_Trade.empty())
    {
        AddTrade(m_Trade);
        LogInfo(L"Trade added: (%d) (%ls)", m_Trade.GetId(), m_Trade.GetUser());
        m_Trade.Clear();
        // TODO: AddPartialTrade()
        GetPolicy().DoneGathering();
    }
    else if (Text.IsEmpty())
    {
        GetPolicy().DoneGathering();
    }
    SetState(FindNewTrade);
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
InterpretNewText(
          Window::Type_e      WindowType,
    const TiTrades_t::Text_t& Text,
          size_t              FirstNewLine)
{
WindowType;
    ASSERT(Window::PostedTradesView == WindowType);
    if (BuildTrade == m_State)
        MaybeSkipTrade(FirstNewLine);

    ASSERT(Text.GetEndRow() > FirstNewLine);
    for (size_t Line = FirstNewLine; Line < Text.GetEndRow(); ++Line)
    {
        if (EXTRALOG)
			Text.GetRow(Line); // commented out to compile , L"New");
        InterpretNewLine(Text.GetRow(Line));
    }
}

/////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
InterpretNewLine(
    const wchar_t* pszText)
{
    const TradeId_t id = GetTradeId(pszText);
    if (0 < id)
    {
        // Keep the "highest trade Id" updated.
        if (id > m_idLastTrade)
            m_idLastTrade = id;
    }
    switch (m_State)
    {
    case FindNewTrade:
        if (0 < id)
            return StartNewTrade(m_Trade, pszText);
        break;
    case BuildTrade:
        if (0 < id)
        {
            if (!m_Trade.empty())
            {
                GetPolicy().CheckTrade(m_Trade);
                AddTrade(m_Trade);
                m_Trade.Clear();
            }
            return StartNewTrade(m_Trade, pszText);
        }
        else if (!m_Trade.empty())
        {
            return AddTradeLine(m_Trade, pszText);
        }
        break;
    case FindTrade:
        if (id == m_idFindTrade)
        {
            return StartNewTrade(m_Trade, pszText);
        }
        break;
    default:
        ASSERT(false);
        break;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
SetState(State_e State)
{
    CLock lock(m_csState);
    m_State = State;
}

/////////////////////////////////////////////////////////////////////////////

TradeId_t
Handler_t::
GetTradeId(
    const wchar_t* pszText) const
{
    return /*(DWORD)*/_wtoi(pszText);
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
MaybeSkipTrade(
    size_t FirstNewLine)
{
    if (BuildTrade != m_State)
        return;
    // The supplied text is "continued" from the last text if there were
    // matching lines, resulting in a non-zero FirstNewLine.
    // TODO: This will need to be modified for backwards-scrolling.
    const bool bTextContinued = (0 != FirstNewLine);
    if (!bTextContinued)
    {
        // If we were in the process of building a trade which started on
        // the last text, and are sent a new non-continued text, mark the
        // trade id as skipped and start looking for a new trade.

        // could possibly be empty if only contains unknown cards not in list?
        ASSERT(!m_Trade.empty());
        if (EXTRALOG)
            LogInfo(L"Add missed trade: %d", m_Trade.GetId());
        m_MissedTrades.insert(m_Trade.GetId());
        m_Trade.Clear();
        SetState(FindNewTrade);
    }
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
AddTrade(
    Trade_t& Trade)
{
    Trade.SetPostedTime();
    EventAddTrade_t AddTradeEvent(Trade);
    GetPipelineManager().SendEvent(AddTradeEvent.m_Data);
//    GetPolicy().CheckTrade(Trade);
}

/////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
StartNewTrade(
          Trade_t& Trade,
    const wchar_t* pszText)
{
    TradeId_t id = GetTradeId(pszText);
    ASSERT(0 != id);
    //ASSERT(!IsMissedTrade(id)); // should be impossible
    // TODO: May need to use an event here.
    if (Services::GetTradeManager().FindTrade(id))
    {
        LogInfo(L"Skipping trade (%d)", id);
        return false;
    }
    RemoveMissedTrade(id);
    Trade.SetId(id);
    size_t pos = m_TiTrades.GetText().GetColumnOffset(User);
    Trade.SetUser(&pszText[pos]);
    AddTradeLine(Trade, pszText);
    SetState(BuildTrade);
    if (EXTRALOG)
        LogInfo(L"TiPostedTrades::StartNewTrade(%d, '%ls')", m_Trade.GetId(), Trade.GetUser());
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
AddTradeLine(
          Trade_t& Trade,
    const wchar_t* pszText)
{
    bool bOfferAdded = AddTradeItem(Trade.GetOfferCards(), pszText, Offer);
    bool bWantAdded  = AddTradeItem(Trade.GetWantCards(), pszText, Want);
    return bOfferAdded || bWantAdded;
}

/////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
AddTradeItem(
          CardCollection_t& Cards,
    const wchar_t*          pszText,
          size_t            TextColumn)
{
    size_t Pos = m_TiTrades.GetText().GetColumnOffset(TextColumn);
    if (L'\0' == pszText[Pos])
        return false;
    int Count;
    const wchar_t* pszName = ParseItemName(&pszText[Pos], Count);
    if (L'\0' == pszName[0])
        return false;
    const Card_t* pCard = Services::GetCardSet().Lookup(pszName, true);
    if (NULL == pCard)
        return false;
    CardQuantity_t Item(pCard, Count);
    if (!Cards.insert(Item).second)
    {
        // TODO:
        // Couldn't add a trade item.  Probably due to a near-duplicate
        // card name that looks duplicate on screen, like a variation
        // of one of the starter decks.  Need to solve this at some point.
        // Insert UNKNOWN_CARD into trade to signify its bogus
        LogError(L"Trade item not added. (%d, '%ls')", m_Trade.GetId(), pszName);
        pCard = Services::GetCardSet().Lookup(L"UNKNOWN_CARD");
        if (NULL != pCard)
        {
            CardQuantity_t UnknownCard(pCard, Count);
            Cards.insert(UnknownCard);
        }
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////

const wchar_t*
Handler_t::
ParseItemName(
    const wchar_t* pszText,
          int&     Count)
{
    static const wchar_t szQuant[] = L" x ";

    Count = 1;
    const wchar_t* pPos = wcsstr(pszText, szQuant);
    if (NULL != pPos)
    {
        Count = _wtoi(pszText);
        pszText = pPos + wcslen(szQuant);
        while (iswspace(*pszText))
            ++pszText;
    }
    return pszText;
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
GetMoreData(
    const RECT& rcBounds)
{
rcBounds;
    switch (m_State)
    {
    case FindNewTrade:
        if (!ScrollToMissedTrade())
            Scroll(m_ScrollType, 1);
        break;
    case BuildTrade:
        // If we aren't building the specific trade we were trying to
        // find, then attempt any MissedTrades first.
        if (m_idFindTrade != m_Trade.GetId())
        {
            if (ScrollToMissedTrade())
                break;
        }
        Scroll(m_ScrollType, 1);
        break;
    case FindTrade:
        if (ScrollToTrade(m_idFindTrade))
            break;
        {
            RECT rcTrade;
            if (FindTradeInText(m_idFindTrade, m_TiTrades.GetText(), rcBounds, rcTrade))
                GetPolicy().TradeFound(m_idFindTrade, rcTrade);
            else
                GetPolicy().TradeNotFound(m_idFindTrade);
        }
        RemoveMissedTrade(m_idFindTrade);
        if (!ScrollToMissedTrade())
        {
            SetState(FindNewTrade);
            m_idFindTrade = 0;
            Scroll(m_ScrollType, 1);
        }
        break;
    default:
        ASSERT(false);
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////
//
// rcTrade is relative to whatever window we're screen capturing.
//

bool
Handler_t::
FindTradeInText(
          TradeId_t           TradeId,
    const TiTrades_t::Text_t& Text,
    const RECT&               rcBounds,
          RECT&               rcTrade) const
{
    ASSERT(0 != TradeId);
    ASSERT(!IsRectEmpty(&rcBounds));
    for (size_t Line = 0; Line < Text.GetEndRow(); ++Line)
    {
        const wchar_t* pLine = Text.GetRow(Line);
        if (_wtoi(pLine) == (int)TradeId)
        {
            // TODO: calculate rect of all trade lines ;
            //       i.e. not just the rect of 1st line of the trade? 
//            GetManager().GetTranslator().GetDcr().
// NOTE next line commented out to compile, GetRect() no longer exits
			rcTrade;
//               Text.GetRect(rcBounds, Line, Line + 1, rcTrade);
            return true;
        }
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
ScrollToMissedTrade()
{
    while (0 < m_MissedTrades.size())
    {
        TradeIdSet_t::iterator it = m_MissedTrades.begin();
        if (ScrollToTrade(*it))
        {
            m_idFindTrade = *it;
            SetState(FindTrade);
            return true;
        }
        LogWarning(L"Removing missed trade, %d", *it);
        m_MissedTrades.erase(it);
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
ScrollToTrade(
    TradeId_t TradeId) const
{
    TradeId_t FirstId = GetFirstTradeId(m_TiTrades.GetText());
    if (0 == FirstId)
        return Scroll(m_LastScrollType);
    else if (TradeId < FirstId)
        return Scroll(LonWindow_t::EventScroll_t::PageUp);
    else if (TradeId > GetLastTradeId(m_TiTrades.GetText()))
        return Scroll(LonWindow_t::EventScroll_t::PageDown);
    // TODO: Handle indeterminate case!
    return false;
}

/////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
Scroll(
    LonWindow_t::EventScroll_t::Type_e Type,
    size_t                             Count) const
{
    static const Window::Type_e ScrollWindowType = Window::PostedTradesVScroll;
    if (!LonWindow_t::CanScroll(ScrollWindowType))
        return false;
    LonWindow_t::Scroll(ScrollWindowType,
                        ScrollBar_t::Vertical,
                        Type,
                        Count,
                        PageLines);
    m_LastScrollType = Type;
    return true;
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
RemoveMissedTrade(
    TradeId_t id)
{
    TradeIdSet_t::iterator it = m_MissedTrades.find(id);
    if (m_MissedTrades.end() != it)
        m_MissedTrades.erase(it);
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
ShowMissedTradeNumbers() const
{
    if (m_MissedTrades.empty())
    {
        LogAlways(L"No missed trades");
        return;
    }
    LogAlways(L"Missed trades:");
    TradeIdSet_t::const_iterator it = m_MissedTrades.begin();
    for (; m_MissedTrades.end() != it; ++it)
        LogAlways(L"  %d", *it);
    LogAlways(L"Missed %d trades.", m_MissedTrades.size());
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
ShowState(
    const wchar_t* pszText, 
          bool     bAlways) const
{
    wchar_t* pszState = L"Unknown";

    switch (m_State)
    {
    case FindNewTrade:  pszState = L"FindNewTrade";    break;
    case BuildTrade:    pszState = L"BuildTrade";      break;
    case FindTrade:     pszState = L"FindTrade";       break;
    default:
        ASSERT(0);
        break;
    }
    if (bAlways)
    {
        LogAlways(L"%ls: State=%ls, Last=%d, Find=%d, Build=%d, Missed=%d",
            (NULL == pszText) ? L"ShowState()" : pszText,
            pszState, m_idLastTrade, m_idFindTrade, m_Trade.GetId(), m_MissedTrades.size());
    }
    else
    {
        LogInfo(L"%ls: State=%ls, Last=%d, Find=%d, Build=%d, Missed=%d",
            (NULL == pszText) ? L"ShowState()" : pszText,
            pszState, m_idLastTrade, m_idFindTrade, m_Trade.GetId(), m_MissedTrades.size());
    }
}

/////////////////////////////////////////////////////////////////////////////

TradeId_t
Handler_t::
GetFirstTradeId(
    const TiTrades_t::Text_t& Text) const
{
    for (size_t Line = 0; Line < Text.GetEndRow(); ++Line)
    {
        const wchar_t* pLine = Text.GetRow(Line);
        TradeId_t TradeId = _wtoi(pLine);
        if (0 < TradeId)
            return TradeId;
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////

TradeId_t
Handler_t::
GetLastTradeId(
    const TiTrades_t::Text_t& Text) const
{
    for (size_t Line = Text.GetEndRow(); 0 < Line--;)
    {
        const wchar_t* pLine = Text.GetRow(Line);
        TradeId_t TradeId = _wtoi(pLine);
        if (0 < TradeId)
            return TradeId;
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////

} // Interpret
} // PostedTrades

/////////////////////////////////////////////////////////////////////////////
