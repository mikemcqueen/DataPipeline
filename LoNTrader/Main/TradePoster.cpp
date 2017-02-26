///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TradePoster.cpp
//
// Implementation of the TradePoster::Manager_t class.
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TradePoster.h"
#include "PassiveFixedPricing_t.h"
#include "SimpleRangePricing_t.h"
#include "SimplePosting_t.h"
#include "Services.h"
#include "Log.h"
#include "XmlUtil.h"
#include "XmlFile.h"
#include "PostedTradesTypes.h"
#include "PcapTrades_t.h"
#include "TradePosterData.h"
#include "LonTrader_t.h"
#include "LonPlayer_t.h"
#include "LonCardSet_t.h"
#include "PipelineManager.h"

namespace TradePoster
{

//
// Static definitions
//

volatile LONG Manager_t::m_NextId    = 0;

static const size_t MaxBuyCardCount  = 50;
static const size_t MaxSellCardCount = 16;

///////////////////////////////////////////////////////////////////////////////

Manager_t::
Manager_t()
{
}

/////////////////////////////////////////////////////////////////////////////

bool
Manager_t::
Initialize(
    const wchar_t* pszClass)
{
    if (!DP::Handler_t::Initialize(pszClass))
        return false;

	m_hTimer = CreateWaitableTimer(NULL, TRUE, NULL);
    if (NULL == m_hTimer.get())
        return false;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT
Manager_t::
EventHandler(
    DP::Event::Data_t& Data)
{
    HRESULT hr = DP::Handler_t::EventHandler(Data);
    if (S_FALSE != hr)
        return hr;
    using namespace Lon::Event;
    switch (Data.Id)
    {
    case Id::AddTrade:
        OnAddTrade(static_cast<PostedTrades::EventAddTrade_t::Data_t&>(Data).Trade);
        break;
    default:
        return S_FALSE;
    }
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT
Manager_t::
OnTransactionComplete(
    const DP::Transaction::Data_t& Data)
{
    using namespace Lon::Transaction;
    switch (Data.Id)
    {
    case Id::PostTrade:
        if (Error::None == Data.Error)
        {
            const EventPost_t::Data_t&
                PostData = static_cast<const EventPost_t::Data_t&>(Data);
            if (OnPostTradeComplete(PostData))
                return S_OK;
        }
        break;
    case Id::GatherTrades:
        OnGatherTradesComplete();
//            static_cast<const TradeManager_t::EventGatherTrades_t::Data_t&>(Data));
        break;
    default:
        break;
    }
    return S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT
Manager_t::
MessageHandler(
    const DP::Message::Data_t* pData)
{
    using namespace Lon::Message;
    if (Id::RemoveTrade == pData->Id)
    {
        const PcapTrades_t::AcquireData_t&
            PcapData = static_cast<const PcapTrades_t::AcquireData_t&>(*pData);
        OnRemoveTrade(PcapData.TradeId);
        return S_OK;
    }
    return S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//
// HACK. 
//

void
Manager_t::
OnGatherTradesComplete()
{
#if 0
    CheckTrades();
    const size_t TimerDelayInSeconds = 300;// (300 + rand() % 60); // 5-6 min
    SetTimer(TimerDelayInSeconds);
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
// HACK.  Move to PM.QueueSetTimer() ?
//
static
struct TimerSet_t
{
    Manager_t* pTradePoster;
    size_t     Seconds;
} s_TimerSet;

void
Manager_t::
QueueSetTimer(
    size_t Seconds)
{
    LogAlways(L"TP::QueueSetTimer(%d)", Seconds);
    s_TimerSet.pTradePoster = this;
    s_TimerSet.Seconds = Seconds;
    GetPipelineManager().QueueApc(TimerSetApc, ULONG_PTR(&s_TimerSet));
}

///////////////////////////////////////////////////////////////////////////////

/*static*/
void
CALLBACK
Manager_t::
TimerSetApc(
    ULONG_PTR Param)
{
    LogAlways(L"TP::TimerSetApc()");
    TimerSet_t& Ts = *reinterpret_cast<TimerSet_t*>(Param);
    Ts.pTradePoster->SetTimer(Ts.Seconds);
}

///////////////////////////////////////////////////////////////////////////////

void
Manager_t::
SetTimer(
    size_t Seconds)
{
    LogAlways(L"TP::SetTimer (%d)", Seconds);
    static const __int64 OneSecond = -10000000LL; // 1 seconds
    LARGE_INTEGER Time;
    Time.QuadPart = OneSecond * Seconds; 
    if (!SetWaitableTimer(m_hTimer.get(), &Time, 0L, TimerTickApc, this, FALSE))
        throw std::runtime_error("SetWaitableTimer() failed");
}

///////////////////////////////////////////////////////////////////////////////

/*static*/
void
CALLBACK
Manager_t::
TimerTickApc(
   void* pArg,
   DWORD dwTimerLowValue,
   DWORD dwTimerHighValue)
{
    LogAlways(L"TP::TimerTickApc()");
    dwTimerLowValue; dwTimerHighValue;
    static_cast<Manager_t*>(pArg)->OnTimer();
}

///////////////////////////////////////////////////////////////////////////////

void
Manager_t::
OnTimer()
{
    LogAlways(L"TP::OnTimer()");
}

///////////////////////////////////////////////////////////////////////////////
//
// This is an event handler; called sychronously within the context of a 
// DP::Interpret call.
//
// So, we can't fire off another event, right?
//
// TODO: QueueUserAPC, then it won't matter.
//

void
Manager_t::
OnAddTrade(
    Trade_t& Trade)
{
    CLock lock(m_csMap);
    Map_t::const_iterator it = m_Map.begin();
    for (; m_Map.end() != it; ++it)
    {
        const Data_t& Data = it->second;
        // NOTE: Neither of these currently do anything, 01-Jun-2008.
        Data.GetPricingPolicy().OnTradeAdded(Trade);
        Data.GetPostingPolicy().OnTradeAdded(Trade);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
// This is DP::Callback handler.
//
// So, we can fire off another event, right?
//

void
Manager_t::
OnRemoveTrade(
    TradeId_t TradeId)
{
    CLock lock(m_csMap);
    Map_t::const_iterator it = m_Map.begin();
    for (; m_Map.end() != it; ++it)
    {
        const Data_t& Data = it->second;
        Data.GetPricingPolicy().OnTradeRemoved(Data, TradeId);
        Data.GetPostingPolicy().OnTradeRemoved(Data, TradeId);
    }
}

///////////////////////////////////////////////////////////////////////////////

bool
Manager_t::
OnPostTradeComplete(
    const EventPost_t::Data_t& Event)
{
    LogAlways(L"TP::OnPostTradeComplete(%d) Id(%d) -> Posted(%d)",
              Event.TradePosterId, Event.pTrade->GetId(), Event.TradeId);
    {
        CLock lock(m_csMap);
        Map_t::const_iterator it = MapFindId(Event.TradePosterId);
        if (m_Map.end() != it)
        {
            const Data_t& Data = it->second;
            Data.GetPricingPolicy().
                OnTradePosted(*Event.pTrade, Event.TradeId, Event.Value);
            Data.GetPostingPolicy().
                OnTradePosted(*Event.pTrade, Event.TradeId, Event.Value);
            return true;
        }
    }
    LogError(L"OnPostTradeComplete(%d) Invalid TradePosterId", Event.TradePosterId);
    return false;
}

///////////////////////////////////////////////////////////////////////////////

size_t
Manager_t::
PostAllTradesAtDesiredValues(
    bool bTest)
{
    return PostAllTrades(m_DesiredCardValues, bTest);
}

///////////////////////////////////////////////////////////////////////////////

size_t
Manager_t::
PostAllTrades(
    const CardValueSet_t& CardValues,
          bool            bTest)
{
    size_t Count = 0;
    Flag_t Flags = 0;
    if (bTest)
        Flags |= Flag::TestPost;
    CLock lock(m_csMap);
    Map_t::const_iterator it = m_Map.begin();
    for (; m_Map.end() != it; ++it)
    {
        const Data_t& Data = it->second;
        size_t Value = Data.GetValue(CardValues);
        if (0 < Value)
        {
            Count += Data.GetPostingPolicy().
                         PostAllTrades(
                             Data.GetId(),
                             Value,
                             Flags);
        }
    }
    return Count;
}

///////////////////////////////////////////////////////////////////////////////
// PostTradesAtDesiredValues(Id_t Id)
//    return PostTrade(TradePosterId, m_DesiredCardValues, bTest);

size_t
Manager_t::
PostTrades(
    Id_t      TradePosterId,
    size_t    Value,
    TradeId_t TradeId,
    bool      bTest)
{
#if 0
    TradePosterId; Value; TradeId; bTest;
    LogError(L"TODO: PostTrade() NotImpl");
    return 0;
#else
    TradeId; // ignored for now
    CLock lock(m_csMap);
    Map_t::const_iterator it = MapFindId(TradePosterId);
    if (m_Map.end() == it)
    {
        LogError(L"TP::PostTrades(%d) Invalid TradePosterId", TradePosterId);
        return 0;
    }
    Flag_t Flags = 0;
    if (bTest)
        Flags |= Flag::TestPost;
    const Data_t& Data = it->second;
    if (0 == Value)
        Value = Data.GetValue(m_DesiredCardValues);
    if (0 == Value)
    {
        LogError(L"TP::PostTrades(%d) Value == 0", TradePosterId);
        return 0;
    }
    return Data.GetPostingPolicy().PostAllTrades(Data.GetId(), Value, Flags);
#endif
}

///////////////////////////////////////////////////////////////////////////////

/*
size_t
Manager_t::
RemoveAllTrades(
    bool bTest)
{
    size_t Count = 0;
    Flag_t Flags = 0;
    if (bTest)
        Flags |= Flag::TestPost;
    CLock lock(m_csMap);
    Map_t::const_iterator it = m_Map.begin();
    for (; m_Map.end() != it; ++it)
    {
        const Data_t& Data = it->second;
        Count += Data.pPostingPolicy->RemoveTrade(0, Flags, Data.Id);
    }
    return Count;
}
*/

///////////////////////////////////////////////////////////////////////////////

size_t
Manager_t::
RemoveTrades(
    Id_t  Id,
    bool  bTest)
{
LogError(L"Not implemented");
Id; bTest;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

size_t
Manager_t::
RemoveTrades(
    Id_t   Id,
    size_t Value,
    bool   bTest)
{
LogError(L"Not implemented");
Value; Id; bTest;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

bool
Manager_t::
RemoveTrade(
    Id_t      Id,
    size_t    Value,
    TradeId_t TradeId,
    bool      bTest)
{
Value;Id; bTest; TradeId;
LogError(L"Not implemented");
#if 0
    CLock lock(m_csMap);
    Map_t::const_iterator it = MapFindId(PosterId);
    if (m_Map.end() != it)
    {
        Flag_t Flags = 0;
        if (bTest)
            Flags |= Flag::TestPost;
        PostingPolicy_i* pPolicy = it->second.pPostingPolicy;
        return pPolicy->RemoveTrade(TradeId, Flags, PosterId);
    }
#endif
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

PricingPolicy_i*
Manager_t::
GetPricingPolicy(
    Id_t Id)
{
    PricingPolicy_i* pPolicy = NULL;
    CLock lock(m_csMap);
    Map_t::const_iterator it = MapFindId(Id);
    if (m_Map.end() != it)
    {
        pPolicy = &it->second.GetPricingPolicy();
    }
    return pPolicy;
}

///////////////////////////////////////////////////////////////////////////////

PostingPolicy_i*
Manager_t::
GetPostingPolicy(
    Id_t Id)
{
    PostingPolicy_i* pPolicy = NULL;
    CLock lock(m_csMap);
    Map_t::const_iterator it = MapFindId(Id);
    if (m_Map.end() != it)
    {
        pPolicy = &it->second.GetPostingPolicy();
    }
    return pPolicy;
}

///////////////////////////////////////////////////////////////////////////////

Id_t
Manager_t::
AddPostData(
    const Data_t& NewData)
{
    CLock lock(m_csMap);
    m_Map.insert(Pair_t(NewData.m_CardQ.pCard->GetId(), NewData));
    return NewData.GetId();
}

///////////////////////////////////////////////////////////////////////////////

Map_t::const_iterator
Manager_t::
MapFindId(
    Id_t Id) const
{
    Map_t::const_iterator it = m_Map.begin();
    for (; m_Map.end() != it; ++it)
    {
        if (it->second.GetId() == Id)
            break;
    }
    return it;
}

///////////////////////////////////////////////////////////////////////////////

Map_t::iterator
Manager_t::
MapFindId(
    Id_t Id)
{
    Map_t::iterator it = m_Map.begin();
    for (; m_Map.end() != it; ++it)
    {
        if (it->second.GetId() == Id)
            break;
    }
    return it;
}

///////////////////////////////////////////////////////////////////////////////

void
Manager_t::
Show(
    bool bDetail) const
{
    Show(0, 0, bDetail);
}

///////////////////////////////////////////////////////////////////////////////

void
Manager_t::
Show(
    Id_t   Id,
    bool   bDetail) const
{
    Show(Id, 0, bDetail);
}

///////////////////////////////////////////////////////////////////////////////

void
Manager_t::
Show(
    Id_t   Id,
    size_t Value,
    bool   bDetail) const
{
    CLock lock(m_csMap);
    Map_t::const_iterator it = MapFindId(Id);
    if (m_Map.end() != it)
    {
        ASSERT(it->second.GetId() == Id);
        it->second.Show(Value, bDetail);
        return;
    }
    LogError(L"TradePoster::Show(%d) Id not found", Id);
}

///////////////////////////////////////////////////////////////////////////////

void
Manager_t::
ShowAll(
    Flag_t Flags) const
{
    ASSERT(0 != (Flag::ShowAllTrades & Flags));
    bool bDetail = 0 != (Flag::ShowDetail & Flags);

    CLock lock(m_csMap);
    Map_t::const_iterator it = m_Map.begin();
    for (; m_Map.end() != it; ++it)
    {
        const Data_t& Data = it->second;
/*
        if ((Data_t::Offer == Data.Type) && (0 == (Flag::ShowSellTrades & Flags)))
            continue;
        if ((Data_t::Want == Data.Type) && (0 == (Flag::ShowBuyTrades & Flags)))
            continue;
*/
        Data.Show(bDetail);
    }
}

///////////////////////////////////////////////////////////////////////////////

size_t
Manager_t::
WriteXmlDirectory(
    const wchar_t* pszDir) const
{
    size_t Count = 0;
    CLock lock(m_csMap);
    Map_t::const_iterator it = m_Map.begin();
    for (; m_Map.end() != it; ++it)
    {
        const Data_t& Data = it->second;
        Count += Data.WriteXmlDirectory(pszDir);
    }
    LogAlways(L"WriteXmlDirectory done (%d)", Count);
    // TODO: throw if fail?
    WriteXmlDesiredCardValues(pszDir);
    return Count;
}

///////////////////////////////////////////////////////////////////////////////

bool
Manager_t::
WriteXmlDesiredCardValues(
    const wchar_t* pszDir) const
{
    wchar_t szFile[MAX_PATH] = { 0 };
    _snwprintf_s(szFile, _TRUNCATE, L"%ls\\DesiredCardValues.xml", pszDir);
    CLock lock(m_csMap);
    return m_DesiredCardValues.WriteXmlFile(szFile);
}

///////////////////////////////////////////////////////////////////////////////

bool
Manager_t::
ReadXmlDesiredCardValues(
    const wchar_t* pszDir)
{
    wchar_t szFile[MAX_PATH] = { 0 };
    _snwprintf_s(szFile, _TRUNCATE, L"%ls\\DesiredCardValues.xml", pszDir);
    CLock lock(m_csMap);
    m_DesiredCardValues.clear();
    m_DesiredCardValues.ReadXmlFile(szFile);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
Manager_t::
ReadXmlFile(
    const wchar_t* pszFilename)
{
    Data_t Data;
    if (!Data.ReadXmlFile(pszFilename))
        return false;
    Data.m_Id = GetNextId();
    AddPostData(Data);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
Manager_t::
WriteXmlDirectory(
    const wchar_t* pszDir,
          Id_t     Id) const
{
#if 0
Id;
LogError(L"Not implemented.");
#else
    CLock lock(m_csMap);
    Map_t::const_iterator it = MapFindId(Id);
    if (m_Map.end() != it)
    {
        const Data_t& Data = it->second;
        ASSERT(Data.GetId() == Id);
        return Data.WriteXmlDirectory(pszDir);
    }
#endif
    return false;
}

///////////////////////////////////////////////////////////////////////////////

static const int MaxTotalTrades = 4; // 1 per booster .. 

size_t
Manager_t::
GenerateTrades(
    size_t MaxPerAllowedCard) const
{
    CLock lock(m_csMap);
    size_t Count = 0;
    Map_t::const_iterator it = m_Map.begin();
    for (; m_Map.end() != it; ++it)
    {
        Count += GenerateTrades(it->second, MaxPerAllowedCard);
    }
    return Count;
}

///////////////////////////////////////////////////////////////////////////////

size_t
Manager_t::
GenerateTrades(
    Id_t   Id,
    size_t MaxPerAllowedCard) const
{
    CLock lock(m_csMap);
    Map_t::const_iterator it = MapFindId(Id);
    if (m_Map.end() == it)
    {
        LogError(L"GenerateTrades() Invalid TradePosterId (%d)", Id);
        return 0;
    }
    return GenerateTrades(it->second, MaxPerAllowedCard);
}

///////////////////////////////////////////////////////////////////////////////

size_t
Manager_t::
GenerateTrades(
    const Data_t& Data,
          size_t  MaxPerAllowedCard) const
{
    if (NULL == Data.m_pPricingPolicy)
    {
        LogError(L"No pricing policy.");
        return 0;
    }
    if (NULL == Data.m_pPostingPolicy)
    {
        LogError(L"No posting policy.");
        return 0;
    }
    const PricingPolicy_i& Pricing = Data.GetPricingPolicy();
    size_t Price = Pricing.GetLowPrice();
    size_t PriceIncrement = Pricing.GetPriceIncrement();
    size_t Count = 0;
    while (Pricing.GetHighPrice() >= Price)
    {
        Count += Data.GetPostingPolicy().
            GenerateTrades(
                Data.GetType(),
                Price,
                MaxPerAllowedCard,
                MaxTotalTrades);
        if (0 == PriceIncrement)
            break;
        Price += PriceIncrement;
    }
    return Count;
}

///////////////////////////////////////////////////////////////////////////////

size_t
Manager_t::
GenerateTrades(
    Id_t   Id,
    size_t Value,
    size_t MaxPerAllowedCard) const
{
Id;Value;MaxPerAllowedCard;
    LogError(L"TODO: GenerateTrades() Not impl");
    return 0;
#if 0
    return Data.pPostingPolicy->
               GenerateTrades(
                   Data.Type,
                   Value,
                   MaxPerAllowedCard,
                   MaxTotalTrades);
#endif
}

///////////////////////////////////////////////////////////////////////////////

bool
TradePoster::
Manager_t::
Remove(
   Id_t Id)
{
    CLock lock(m_csMap);
    Map_t::iterator it = MapFindId(Id);
    if (m_Map.end() != it)
    {
        ASSERT(it->second.GetId() == Id);
        m_Map.erase(it);
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void
Manager_t::
BuyCard(
    const LonCard_t* pCard,
          size_t     Low,
          size_t     High,
          size_t     Increment,
          size_t     Count,
          Flag_t     Flags)
{
    LogAlways(L"TP::BuyCard(%ls) Buy up to %d @ %d-%d step %d each",
              pCard->GetName(), Count, Low, High, Increment);
    PricingPolicy_i* pPricing = NewPricingPolicy(Low, High, Increment, Flags);
    ASSERT(NULL != pPricing);
    CardQuantity_t CardQ(pCard, Count);
    CardQuantityQueue_t Cards;
    AddBoosters(Cards, true);
    SimplePosting_t* pPosting = new SimplePosting_t(CardQ, Cards);
    Id_t Id = BuyCard(CardQ, pPricing, pPosting);
    if (0 != Id)
        Show(Id);
    else
        LogError(L"TP::BuyCard() failed.");
}

////////////////////////////////////////////////////////////////////////////////

void
Manager_t::
SellCard(
    const LonCard_t* pCard,
          size_t     Low,
          size_t     High,
          size_t     Increment,
          size_t     Count,
          Flag_t     Flags)
{
    LogAlways(L"TP::SellCard(%ls) Sell up to %d @ %d-%d each",
              pCard->GetName(), Count, Low, High);
    PricingPolicy_i* pPricing = NewPricingPolicy(Low, High, Increment, Flags);
    CardQuantity_t CardQ(pCard, Count);
    CardQuantityQueue_t Cards;
    AddBoosters(Cards, false);
    SimplePosting_t* pPosting = new SimplePosting_t(CardQ, Cards);
    Id_t Id = SellCard(CardQ, pPricing, pPosting);
    if (0 != Id)
        Show(Id);
    else
        LogError(L"TP::SellCard() failed.");
}

///////////////////////////////////////////////////////////////////////////////

bool
Manager_t::
AddBoosters(
    CardQuantityQueue_t& Cards,
    bool                 bBuying)
{
    size_t Count = (bBuying) ? MaxBuyCardCount : MaxSellCardCount;
    const Card_t* pCard;

    pCard = Services::GetCardSet().LookupPartialAtAnyPos(L"Oathbreaker Booster");
    if (NULL == pCard)
        return false;
    Cards.push_back(CardQuantity_t(pCard, Count));

    pCard = Services::GetCardSet().LookupPartialAtAnyPos(L"Inquisitor Booster");
    if (NULL == pCard)
        return false;
    Cards.push_back(CardQuantity_t(pCard, Count));

    pCard = Services::GetCardSet().LookupPartialAtAnyPos(L"Forsworn Booster");
    if (NULL == pCard)
        return false;
    Cards.push_back(CardQuantity_t(pCard, Count));

    pCard = Services::GetCardSet().LookupPartialAtAnyPos(L"Oathbound Booster");
    if (NULL == pCard)
        return false;
    Cards.push_back(CardQuantity_t(pCard, Count));
    return true;
}

///////////////////////////////////////////////////////////////////////////////

PricingPolicy_i*
Manager_t::
NewPricingPolicy(
    size_t Low,
    size_t High,
    size_t Increment,
    Flag_t Flags) const
{
Flags;
    ASSERT((0 != Low) && (0 != High) && (Low <= High));
    PricingPolicy_i *pPolicy = NULL;
    if (Low == High)
    {
        pPolicy = new PassiveFixedPricing_t(Low);
    }
    else
    {
        pPolicy = new SimpleRangePricing_t(Low, High, Increment);
    }
    return pPolicy;
}

///////////////////////////////////////////////////////////////////////////////

size_t
Manager_t::
InitDesiredCardValues()
{
    return InitCardValues(m_DesiredCardValues);
}

///////////////////////////////////////////////////////////////////////////////

size_t
Manager_t::
InitActualCardValues()
{
    return InitCardValues(m_ActualCardValues);
}

///////////////////////////////////////////////////////////////////////////////

size_t
Manager_t::
InitCardValues(
    CardValueSet_t& CardValues) const
{
    CLock lock(m_csMap);
    CardValues.clear();
    size_t Count = 0;
#if 1
    Map_t::const_iterator it = m_Map.begin();
    for (; m_Map.end() != it; ++it)
    {
        const Data_t& Data = it->second;
        Count += Data.AddCardValues(CardValues);
    }
#else
    std::for_each(m_Map.begin(), m_Map.end(),
//        Count +=
              boost::bind(
                  &Data_t::BuildCardValues,
                  boost::bind(&Pair_t::second, _1),
                  m_DesiredCardValues));
#endif
    Services::GetCardSet().GetCardValues(CardValues);
    return Count;
}

///////////////////////////////////////////////////////////////////////////////

size_t
Manager_t::
InitCardValues(
    Id_t Id)
//    CardValueSet_t& Cards) const
{
    LogError(L"Not implemented");
    Id;return 0;
// Cards; 
}

///////////////////////////////////////////////////////////////////////////////

void
Manager_t::
ShowCardValues() const
{
    // TODO: Rethink this.  1 loop, display both every iteration.
    m_DesiredCardValues.Show(L"DesiredCardValues");
    m_ActualCardValues.Show(L"ActualCardValues");
}

///////////////////////////////////////////////////////////////////////////////

void
Manager_t::
SetCardValue(
    const CardValue_t& CardValue)
{
    m_DesiredCardValues.SetCardValue(CardValue);
}

///////////////////////////////////////////////////////////////////////////////

} // namespace TradePoster
