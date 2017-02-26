/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TiTradeBuilder.cpp
//
// Text Interpreter for LoN Posted Trade Builder.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TransactionManager.h"
#include "TiTradeBuilder.h"
#include "DcrTradeBuilder.h"
#include "PipelineManager.h"
#include "LonCardSet_t.h"
#include "TradeManager_t.h"
#include "LonTrader_t.h"
#include "Trade_t.h"
#include "Services.h"

using namespace Lon;

/////////////////////////////////////////////////////////////////////////////

static const size_t StateTimeoutThreshold = 20;
static bool         EXTRALOG = true;

Window::Type_e g_TradeBuilderVScroll = Window::TradeBuilderTheirTableVScroll;
Window::Type_e g_TradeBuilderHScroll = Window::TradeBuilderTheirTableHScroll;

/////////////////////////////////////////////////////////////////////////////

namespace TradeBuilder
{
namespace Interpret
{

Handler_t::
Handler_t(
    ManagerBase_t& Manager)
:
    m_TiTable(Table::TheirCharColumnWidths, Table::TheirColumnCount),
    m_Manager(Manager),
    m_State(Ready),
    m_StateTimeout(0),
    m_WhichCollection(Collection::Theirs),
    m_bExitBuilder(true),
    m_bSwitched(false),
    m_bScrolledYours(false)
{
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
ExecuteTransaction(
    const DP::Transaction::Data_t& Data)
{
    // Determine if this is a transaction that we care about.
    switch (Data.Id)
    {
    case Transaction::Id::GetYourCards:
    case Transaction::Id::PostTrade:
        break;
    default:
        return S_FALSE;
    }

    CLock lock(m_csState);
    ASSERT(Ready == m_State);
    if (Ready != m_State)
        return SCHED_E_TASK_NOT_READY;

    switch (Data.Id)
    {
    case Transaction::Id::GetYourCards:
        GetYourCards();
        break;
    case Transaction::Id::PostTrade:
        PostNewTrade(static_cast<const TradePoster::EventPost_t::Data_t&>(Data));
        break;
    default:
        throw std::logic_error("TiTradeBuilder: Invalid transaction Id");
    }
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
GetYourCards()
{
    m_Trade.Clear();
    m_WhichCollection  = Collection::Yours;
    m_bExitBuilder     = false;
    m_bSwitched        = false;
    m_bScrolledYours   = false;
    SetState(SelectCollection);
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
PostNewTrade(
    const TradePoster::EventPost_t::Data_t& Data)
{
    m_Trade            = *Data.pTrade;
    m_WhichCollection  = Collection::Theirs;
    m_bExitBuilder     = Data.bExitBuilder;
    m_bSwitched        = false;
    m_bScrolledYours   = false;
    SetState(ClearTrade);
    return true;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
OnTransactionComplete(
    const DP::Transaction::Data_t& TransactionData)
{
    LogInfo(L"TiTradeBuilder::OnTransactionComplete: State (%d) Error (%d)",
            m_State, TransactionData.Error);
    using namespace Transaction;
    switch (TransactionData.Id)
    {
    case Id::PostTrade:
        // If we are currently in "Waiting" state because we passed the 
        // transaction off to ConfirmTrade, and the transaction is completed
        // there without error, we're done waiting and need to return to 
        // "Ready" state.
        if ((Error::None == TransactionData.Error) && (Waiting == m_State))
        {
            SetState(Ready);
        }
        else
        {
            // Otherwise TransactionData.Error must be set.
            ASSERT(Error::None != TransactionData.Error);
            ASSERT(Ready == m_State);
            SetState(Ready); // just in case
        }
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
   const DP::Message::Data_t* pMessage)
{
    if (Message::Id::TextTable != pMessage->Id)
        return S_FALSE;

    const Translate::Data_t*
        pLonData = static_cast<const Translate::Data_t*>(pMessage);
    if (TopWindowType != pLonData->WindowType)
        return S_FALSE;

    DP::Transaction::Data_t* pData = GetTransactionManager().Acquire();
    DP::TransactionManager_t::AutoRelease_t ar(pData);
    if (NULL == pData)
        return S_FALSE;

    LogInfo(L"TiTradeBuilder::Interpret(%d) State=%d", pData->Id, m_State);

    switch (pData->Id)
    {
    case Transaction::Id::PostTrade:
        InterpretPostTrade(
            pLonData,
            static_cast<TradePoster::EventPost_t::Data_t&>(*pData));
        break;
    case Transaction::Id::GetYourCards:
        InterpretGetYourCards(
            pLonData,
            static_cast<LonPlayer_t::EventGetYourCards_t::Data_t&>(*pData));
        break;
    default:
        return S_FALSE;
    }
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
InterpretGetYourCards(
    const Translate::Data_t*                        pBuilderData,
          LonPlayer_t::EventGetYourCards_t::Data_t& TransactionData)
{
    CLock lock(m_csState);
    switch (m_State)
    {
    case SelectCollection:
        if (DoClickCollection(m_WhichCollection))
            SetState(ClearCardName);
        break;
    case ClearCardName:
        DoClearCardName();
        SetState(ScrollTable);
        break;
    case ScrollTable:
        if (HScrollTable())
            SetState(AddCard);
        break;
    case AddCard: // InterpretLines
        {
            size_t FirstNewLine;
            bool bNew = m_TiTable.CompareText(pBuilderData->Text, FirstNewLine);
            if (bNew)
            {
                AddCardsToCollection(pBuilderData->Text, FirstNewLine);
                m_TiTable.SetText(pBuilderData->Text);
                VScrollTable();
            }
            else
            {
                const ScrollBar_t::ThumbPosition_e
                    ThumbPos = LonWindow_t::GetThumbPosition(GetVScrollType(),
                                                             ScrollBar_t::Vertical);
                if ((ScrollBar_t::Bottom == ThumbPos) &&
                    (5 <= m_TiTable.GetSameTextCount()))
                {
                    SetState(Done);
                }
            }
        }
        break;
    case Done:
        if (LonWindow_t::Click(Window::TradeBuilderExit))
        {
            TransactionData.YourCards = m_Trade.GetOfferCards();
            LogAlways(L"TradeBuilder: YourCards(%d, %d)",
                      TransactionData.YourCards.size(),
                      TransactionData.YourCards.GetTotalQuantity());
            GetTransactionManager().CompleteTransaction(TransactionData.Id);
            SetState(Ready);
        }
        break;
    case Ready:
        break;
    default:
        ASSERT(false);
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
HScrollTable() const
{
    LonWindow_t::EventScroll_t::Type_e ScrollType;
    Window::Type_e                     WindowType = Window::Unknown;
    size_t                             Count = 0;
                    
    if (Collection::Yours == m_WhichCollection)
    {
        if (!m_bScrolledYours)
        {
            ScrollType = LonWindow_t::EventScroll_t::LineDown;
            WindowType = Window::TradeBuilderYourTableHScroll;
            Count      = Your::ColumnOffset;
            m_bScrolledYours = 
                LonWindow_t::Scroll(
                    WindowType,
                    ScrollBar_t::Horizontal,
                    ScrollType,
                    Count);
        }
        return m_bScrolledYours;
    }
    else
    {
        return true;
/*
        ScrollType = LonWindow_t::EventScroll_t::LineDown;
        WindowType = Window::TradeBuilderTheirTableHScroll,
*/
    }
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
AddCardsToCollection(
    const TiTable_t::Text_t& Text,
          size_t             FirstNewLine)
{
    const size_t EndLine = Text.GetEndRow();
    CardCollection_t& Cards = GetCardCollection(m_WhichCollection);
    for (size_t Line = FirstNewLine; EndLine > Line; ++Line)
    {
        const wchar_t* pLine = Text.GetRow(Line);
        std::wstring strName;
        const wchar_t Rarity = pLine[Text.GetColumnOffset(Your::Rarity)];
        if (CardType::IsNumberPrefixed(Rarity))
        {
            // TODO: should take no rarity also and be able to figure it out
            // TODO: LonCard_t::BuildCardNumberString(std::wstring strName);
            wchar_t buf[32];
            wsprintf(buf, L"%d%lc%d ",
                _wtoi(&pLine[Text.GetColumnOffset(Your::Set)]),
                Rarity,
                _wtoi(&pLine[Text.GetColumnOffset(Your::Number)]));
            strName.assign(buf);
        }
        const wchar_t* pName = &pLine[Text.GetColumnOffset(Your::Title)];
        strName.append(pName);
        LonCard_t::UiNameCleanup(strName);
        const Card_t* pCard = Services::GetCardSet().Lookup(strName.c_str());
        ASSERT(NULL != pCard);
        if (NULL != pCard)
        {
            // Try to insert the base card name.
            const int Quantity = _wtoi(&pLine[Text.GetColumnOffset(Your::Trade)]);
            LogAlways(L"Inserting: %d x %s", Quantity, pCard->GetName());
            CardQuantity_t cq(pCard, Quantity);
            if (Cards.insert(cq).second)
                continue;

            // If the base card name is already inserted, this must be the
            // 2nd occurance of the name; try to insert the foil card name.
            std::wstring strFoil(pCard->GetName());
            strFoil.append(L" (foil)");
            const Card_t* pFoilCard = Services::GetCardSet().LookupPartialAtAnyPos(strFoil.c_str());
            if (NULL != pFoilCard)
            {
                CardQuantity_t cqFoil(pFoilCard, Quantity);
                if (Cards.insert(cqFoil).second)
                    continue;
            }
            // If the foil card name is already inserted, we've hit the
            // third occurance of the card name, which isn't supported.
            ASSERT(false);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
VScrollTable() const
{
    static const LonWindow_t::EventScroll_t::Type_e
                        ScrollType  = LonWindow_t::EventScroll_t::PageDown;
    static const size_t ScrollCount = 1;

    Window::Type_e WindowType = GetVScrollType();
    if (LonWindow_t::CanScroll(WindowType))
    {
        LonWindow_t::Scroll(WindowType,
                            ScrollBar_t::Vertical,
                            ScrollType,
                            ScrollCount,
                            ScrollPageLines);
    }
}

/////////////////////////////////////////////////////////////////////////////

Window::Type_e
Handler_t::
GetVScrollType() const
{
    Window::Type_e WindowType = Window::Unknown;
    if (Collection::Yours == m_WhichCollection)
        WindowType = Window::TradeBuilderYourTableVScroll;
    else
        WindowType = Window::TradeBuilderTheirTableVScroll;
    return WindowType;
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
InterpretPostTrade(
    const Translate::Data_t*                pBuilderData,
          TradePoster::EventPost_t::Data_t& TransactionData)
{
    using namespace Transaction;
    // NOTE: If SetState() is called, then return; else break;
    CLock lock(m_csState);
    switch (m_State)
    {
    case ClearTrade:
        if (DoClearTrade())
            SetState(SelectCollection);
        break;
    case SelectCollection:
        {
            bool bEmpty = false;
            do
            {
                bEmpty = GetCardCollection(m_WhichCollection).empty();
            } while (bEmpty && SwitchCollection());
            if (!bEmpty)
            {
                if (DoClickCollection(m_WhichCollection))
                    SetState(ClearCardName);
            }
            else
                SetState(SubmitTrade);
        }
        break;
    case ClearCardName:
        if (DoClearCardName())
            SetState(ScrollTable);
        return;
    case ScrollTable:
        if (HScrollTable())
            SetState(EnterCardName);
        return;
    case EnterCardName:
        if (DoEnterCardName())
            SetState(AddCard);
        return;
    case AddCard:
        m_TiTable.SetText(pBuilderData->Text);
        if (DoAddCardToTrade(
                pBuilderData->Rect,
                StateTimeoutThreshold < m_StateTimeout))
        {
            SetState(SelectCollection);
        }
        break;
    case SubmitTrade:
        if (DoSubmitTrade())
            SetState(Waiting);
        return;
    case Waiting:
        if (Error::None == TransactionData.Error)
            return;
        // SetState before OnPostTradeError
        SetState(Ready);
        // TransactionData.Error was set in ConfirmTrade.
        OnPostTradeError(Error_e(TransactionData.Error));
        return;
    case Error:
        LogError(L"Error building posted trade");
        TransactionData.Error = Error::BuildingTrade;
        // SetState before OnPostTradeError
        SetState(Ready);
        OnPostTradeError(Error_e(TransactionData.Error));
        return;
    case Ready:
        return;
    default:
        ASSERT(false);
        return;
    }
    if (StateTimeoutThreshold < m_StateTimeout++)
    {
        LogWarning(L"Builder: Timeout...");
        SetState(Error);
    }
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
OnPostTradeError(
    DWORD Error) const
{
    // NOTE: call this from DCR is bad, will result in a SSTask assert.
    // We really just want to flag the transaction as error'd and post
    // it back to PM for TM to pick up.
    if (m_bExitBuilder)
        LonWindow_t::Click(Window::TradeBuilderExit);
    GetTransactionManager().CompleteTransaction(Transaction::Id::PostTrade, Error);
}

/////////////////////////////////////////////////////////////////////////////

CardCollection_t&
Handler_t::
GetCardCollection(
    Collection_e WhichCollection)
{
    return (Collection::Yours == WhichCollection)
        ? m_Trade.GetOfferCards()
        : m_Trade.GetWantCards();
}

/////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
DoClearTrade()
{
    if (EXTRALOG)
        LogAlways(L"BuildState: DoClearTrade");
    return LonWindow_t::Click(Window::TradeBuilderClear);
}

/////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
SwitchCollection()
{
    if (!m_bSwitched)
    {
        m_bSwitched = true;
        m_WhichCollection =
            (Collection::Theirs == m_WhichCollection)
                ? Collection::Yours
                : Collection::Theirs;
        return true;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
DoClickCollection(
    Collection_e WhichCollection)
{
    // TODO: SendEvent(SetCollectionEvent);
    GetManager().GetTranslator().SetCollection(WhichCollection);
    if (Collection::Theirs == WhichCollection)
    {
        g_TradeBuilderVScroll = Window::TradeBuilderTheirTableVScroll;
        g_TradeBuilderHScroll = Window::TradeBuilderTheirTableHScroll;
    }
    else
    {
        g_TradeBuilderVScroll = Window::TradeBuilderYourTableVScroll;
        g_TradeBuilderHScroll = Window::TradeBuilderYourTableHScroll;
    }
    if (!ClickCollection(WhichCollection))
    {
        LogError(L"DoClickCollection(%d)", WhichCollection);
        SetState(Error);
        return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
ClickCollection(
    Collection_e Collection)
{
    if (EXTRALOG)
        LogAlways(L"BuildState: ClickCollection(%d)", Collection);
    const Window::Type_e Tab = Window::TradeBuilderCollectionTab;
    HWND hWnd = LonWindow_t::GetWindow(Tab);
    if (NULL == hWnd)
        return false;
    RECT rcTab;
    ::GetClientRect(hWnd, &rcTab);
    rcTab.right /= CollectionTabDivisor;
    OffsetRect(&rcTab, Collection * rcTab.right, 0);
    return LonWindow_t::Click(Tab, &rcTab);
}

/////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
DoClearCardName()
{
    if (EXTRALOG)
        LogAlways(L"BuildState: ClearCardName");
    LonWindow_t::Click(Window::TradeBuilderSearchEditClear);
    // Always return true. The "X" Clear button isn't visible if there's no
    // text in the edit box, resulting in Click() returning false.
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
DoEnterCardName()
{
    const CardCollection_t& Cards = GetCardCollection(m_WhichCollection);
    if (Cards.empty())
    {
        ASSERT(false);
        SetState(Error);
        return false;
    }
    CardCollection_t::const_iterator it = Cards.begin();
    ASSERT(Cards.end() != it);
    const LonCard_t* pLonCard = static_cast<const LonCard_t*>(it->pCard);
    std::wstring strName(it->pCard->GetName());
    NameFixup(strName, pLonCard->GetNumber());
    if (EXTRALOG)
        LogAlways(L"BuildState: TryEnterCardName '%ls'", strName.c_str());
    return LonWindow_t::SendChars(
               Window::TradeBuilderSearchEdit,
               strName.c_str());
}

/////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
DoAddCardToTrade(
    const RECT& rcTable,
          bool  bForce)
{
    const size_t EndLine = m_TiTable.GetText().GetEndRow();
    if (EXTRALOG)
        LogAlways(L"BuildState: DoAddCard: Force(%d) EndLine(%d)",
                  int(bForce), EndLine);
    if (0 == EndLine)
    {
        LogError(L"DoAddCardToTrade: No cards");
        SetState(Error);
        return false;
    }
    // TODO: should calculate line count from height of window/LineHeightPixels
    if (!bForce && (Table::LineCount - 1 <= EndLine))
        return false;

    CardCollection_t& Cards = GetCardCollection(m_WhichCollection);
    ASSERT(!Cards.empty());
    CardCollection_t::iterator it = Cards.begin();
    const LonCard_t* pLonCard = static_cast<const LonCard_t*>(it->pCard);
    size_t Line = 0;
    if (!FindCardInText(pLonCard, m_TiTable.GetText(), Line))
    {
        // Could still scroll, but i don't see this happening soon
        SetState(Error);
        return false;
    }

    RECT rc = rcTable;
    rc.bottom = rc.top + Table::CharHeightPixels;
    OffsetRect(&rc, 0, int(Line * Table::LineHeightPixels));
    // double-click
    // TODO: if () ?
    LonWindow_t::Click(
        LonWindow_t::GetSsWindowType(TopWindowType),
        &rc,
        LonWindow_t::EventClick_t::LeftButton,
        it->Quantity,
        true);

    Cards.erase(it);
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
FindCardInText(
    const LonCard_t*         pLonCard,
    const TiTable_t::Text_t& Text,
          size_t&            OutLine) const
{
    std::wstring strName(pLonCard->GetName());
    NameFixup(strName, pLonCard->GetNumber());

    size_t First = 1;
    size_t Second = 0;
    size_t Count = 0;
    size_t TitlePos = GetTitleColumnOffset(m_TiTable.GetText());
    // for now, just look for an exact name match.
    for (size_t Line = 0; Line < Text.GetEndRow(); ++Line)
    {
        const wchar_t* pLine = Text.GetRow(Line);
        std::wstring strTitle(&pLine[TitlePos]);
        LonCard_t::UiNameCleanup(strTitle);
        if (0 == strName.compare(strTitle.c_str()))
        {
            ++Count;
            if (Second < First)
            {
                First = Line;
                Second = Line;
            }
            else if (Second == First)
            {
                Second = Line;
            }
        }
    }
    LogInfo(L"FindCardInText(%s): Count (%d) First (%d) Second (%d)",
        strName.c_str(), Count, First, Second);
    switch (Count)
    {
    case 1:
        OutLine = First;
        return true;
    case 2:
        ASSERT(Second > First);
        OutLine = (1 == pLonCard->GetNumber().foil) ? Second : First;
        return true;
    default:
        // zero, or 3 or more and we fail.
        break;
    }
    LogError(L"FindCardInText(%s): Count (%d), Line0Title (%s)",
        strName.c_str(), Count, &Text.GetRow(0)[TitlePos]);
    return false;
}

/////////////////////////////////////////////////////////////////////////////

size_t
Handler_t::
GetTitleColumnOffset(
    const TiTable_t::Text_t& Text) const
{
    const size_t Column =
        (Collection::Theirs == m_WhichCollection) ? Their::Title : Your::Title;
    return Text.GetColumnOffset(Column);
}

/////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
DoSubmitTrade()
{
    LogAlways(L"TiPostedTradeBuilder_t::SubmitTrade");
    return LonWindow_t::Click(Window::TradeBuilderSubmit);
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
NameFixup(
    std::wstring&       strName,
    LonCard_t::Number_t Number) const
{
    // If card has a number like 1F12, skip past it.
    if (0 < Number.num)
    {
        size_t Pos = strName.find(L' ');
        ASSERT(strName.npos != Pos);
        if (strName.npos != Pos)
            strName = strName.substr(Pos + 1);
    }

    // If card has a comma, remove it and everything following it.
    size_t CommaPos = strName.find(L',');
    if (strName.npos != CommaPos)
        strName.erase(CommaPos);

    // If card has (foil) on the end, remove it.
    if (1 == Number.foil)
    {
        size_t Pos = strName.find(L" (foil)");
        if (strName.npos != Pos)
            strName.erase(Pos);
    }
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
SetState(State_e State)
{
    m_State = State;
    m_StateTimeout = 0;
}

/////////////////////////////////////////////////////////////////////////////

} // Interpret
} // TradeBuilder

/////////////////////////////////////////////////////////////////////////////
