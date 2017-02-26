///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// Trade_t.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TradeManager_t.h"
#include "Trade_t.h"
#include "Log.h"
#include "LonCardSet_t.h"
#include "XmlUtil.h"
#include "Services.h"
#include "DbTrades_t.h"
#include "DbGroupedCards_t.h"
#include "DbGroups_t.h"
#include "XmlFile.h"
#include "LonTrader_t.h"
#include "LonPlayer_t.h"

///////////////////////////////////////////////////////////////////////////////

Trade_t::
Trade_t() :
    m_Flags(0)
{
    Clear();
}

///////////////////////////////////////////////////////////////////////////////

bool
Trade_t::
empty() const
{
    return 0 == GetId();
}

///////////////////////////////////////////////////////////////////////////////

void
Trade_t::
Clear()
{
    m_Id = 0;
    offer.Items.clear();
    offer.Value = 0;
    want.Items.clear();
    want.Value = 0;

    m_strUser.clear();
    m_Flags = 0;
    memset(&m_PostedTime,  0, sizeof(m_PostedTime));
    memset(&m_RemovedTime, 0, sizeof(m_RemovedTime));
}

/////////////////////////////////////////////////////////////////////////////

bool
Trade_t::
IsOffered(
    const Card_t* pCard,
          size_t* pCount) const
{
    CardCollection_t::const_iterator
        it = GetOfferCards().find(CardQuantity_t(pCard));
    const bool bHas = GetOfferCards().end() != it;
    if (NULL != pCount)
    {
        *pCount = bHas ? it->Quantity : 0;
    }
    return bHas;
}

/////////////////////////////////////////////////////////////////////////////

bool
Trade_t::
IsWanted(
    const Card_t* pCard,
          size_t* pCount) const
{
    CardCollection_t::const_iterator
        it = GetWantCards().find(CardQuantity_t(pCard));
    const bool bHas = GetWantCards().end() != it;
    if (NULL != pCount)
    {
        *pCount = bHas ? it->Quantity : 0;
    }
    return bHas;
}

///////////////////////////////////////////////////////////////////////////////
//
// Get the high bid (buying price) for a trade.  This is the sum of the 
// high bid value for each offered card.
//

size_t
Trade_t::
GetHighBid() const
{
    size_t Total = 0;
    CardCollection_t::const_iterator it = offer.Items.begin();
    for(; offer.Items.end() != it; ++it)
    {
        size_t Value = 0;
#if defined(LOOKUP)
        const LonCard_t* pCard = m_CardSet.Lookup(it->CardId);
#else
        const LonCard_t* pCard = (LonCard_t*)it->pCard;
#endif
        ASSERT(NULL != pCard);
        Value = pCard->GetHighBid();
        // Any unvalued card in a group is ok, it keeps the
        // bid value low, which is what we want.
        Total += Value * it->Quantity;
    }
    return Total;
}

///////////////////////////////////////////////////////////////////////////////
//
// Get the low ask (selling price) for a trade.  This is the sum of the 
// low ask value for each wanted card.
//

size_t
Trade_t::
GetLowAsk() const
{
    size_t Total = 0;
    CardCollection_t::const_iterator it = want.Items.begin();
    for(; want.Items.end() != it; ++it)
    {
        size_t Value = 0;
#if defined(LOOKUP)
        const LonCard_t* pCard = m_CardSet.Lookup(it->CardId);
#else
        const LonCard_t* pCard = (LonCard_t*)it->pCard;
#endif
        ASSERT(NULL != pCard);
        Value = pCard->GetLowAsk();
        // Any unvalued card == invalid asking price
        if (0 == Value)
            return 0;
        Total += Value * it->Quantity;
    }
    return Total;
}

///////////////////////////////////////////////////////////////////////////////

void
Trade_t::
Show() const
{
    std::wstring strFlags;
    if (0 != (m_Flags & Flag::Other))
        strFlags += L"O";
    if (0 != (m_Flags & Flag::Removed))
        strFlags += L"R";
    LogAlways(L"[%ls] Trade #%d, %s", //Offer = %d, Want = %d", 
        strFlags.c_str(), GetId(), GetUser());//, trade.dwOfferValue, trade.dwWantValue);

    //  TODO: sort by offer-want?

    size_t HighBid = GetHighBid();
    size_t LowAsk = GetLowAsk();
    LogAlways(L"%-57ls%-8d%-57ls%-8d  (%d)",
        L"Offered:", HighBid,
        L"Want:",    LowAsk,
        (int)HighBid - LowAsk);

    size_t OfferCards = 0;
    size_t WantCards = 0;
    CardCollection_t::const_iterator theoffer = offer.Items.begin();
    CardCollection_t::const_iterator thewant = want.Items.begin();
    while (theoffer != offer.Items.end() || thewant != want.Items.end())
    {
        wchar_t buf[256];
        if (theoffer != offer.Items.end())
        {
            const Card_t* pCard = Services::GetCardSet().Lookup(theoffer->CardId);
            ASSERT(NULL != pCard);
            wsprintf(buf, L"%2d x %-60ls",
                theoffer->Quantity,
                (NULL == pCard) ? L"NULL_CARD" : pCard->GetName());
            OfferCards += theoffer->Quantity;
            ++theoffer;
        }
        else
            wsprintf(buf, L"%-65ls", L" ");

        if (thewant != want.Items.end())
        {
            wchar_t buf2[256];
            const Card_t* pCard = Services::GetCardSet().Lookup(thewant->CardId);
            ASSERT(NULL != pCard);
            wsprintf(buf2, L"%2d x %-60ls",
                thewant->Quantity,
                (NULL == pCard) ? L"NULL_CARD" : pCard->GetName());
            wcscat_s(buf, _countof(buf), buf2);
            WantCards += thewant->Quantity;
            ++thewant;
        }
        LogAlways(buf);
    }
    LogAlways(L"Totals: Offered %d (%d), Want %d (%d)",
              offer.Items.size(), OfferCards,
              want.Items.size(), WantCards);
}

///////////////////////////////////////////////////////////////////////////////

bool
Trade_t::
Compare(
    const Trade_t& Trade,
          bool     bCompareUser) const 
{
    if (bCompareUser && (0 != m_strUser.compare(Trade.GetUser())))
        return false;
    return Trade.want.Items.Compare(want.Items) &&
           Trade.offer.Items.Compare(offer.Items);
}

///////////////////////////////////////////////////////////////////////////////

bool
Trade_t::
AddCard(
          CardCollection_t& Cards,
    const Card_t*           pCard,
          size_t            Quantity)
{
    return Cards.insert(CardQuantity_t(pCard, Quantity)).second;
}

///////////////////////////////////////////////////////////////////////////////

bool
Trade_t::
HasLootCard(
    const CardCollection_t& Cards) const
{
    CardCollection_t::const_iterator it = Cards.begin();
    for (; Cards.end() != it; ++it)
    {
        const LonCard_t* pCard = (LonCard_t*)it->pCard;
        if (CardType::Loot == pCard->GetNumber().type)
            return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void
Trade_t::
Read(
    CDatabase&  Db,
    DbTrades_t& RsTrades)
{
    offer.Value    = RsTrades.m_offervalue;
    want.Value     = RsTrades.m_wantvalue;

    SetId           (RsTrades.m_tradeid);
    SetUser         (RsTrades.m_username);
    SetFlags        (RsTrades.m_flags);
    SetPostedTime   (RsTrades.m_postedtime.GetTime());
    SetRemovedTime  (RsTrades.m_removedtime.GetTime());

    offer.Items.Read(Db, RsTrades.m_offerid);
    want.Items.Read (Db, RsTrades.m_wantid);
}

///////////////////////////////////////////////////////////////////////////////

bool
Trade_t::
WriteTrade(
    CDatabase& Db) const
{
    bool bWrote = false;
    DbTrades_t RsTrade(&Db);
    BOOL b = RsTrade.Open(CRecordset::dynaset, NULL, CRecordset::none /*appendOnly*/);
    ASSERT(!!b);
    if (RsTrade.CanAppend())
    {
        long iOfferGroup = WriteCardGroup(Db, offer.Items);
        long iWantGroup =  WriteCardGroup(Db, want.Items);
        RsTrade.AddNew();
        RsTrade.m_tradeid     = (long)GetId();
        RsTrade.m_username    = GetUser();
        RsTrade.m_offerid     = iOfferGroup;
        RsTrade.m_wantid      = iWantGroup;
        RsTrade.m_offervalue  = (long)offer.Value;
        RsTrade.m_wantvalue   = (long)want.Value;
        RsTrade.m_flags       = GetFlags();
        RsTrade.m_postedtime  = GetPostedTime();
        RsTrade.m_removedtime = GetRemovedTime();
        RsTrade.Update();
        bWrote = true;
    }
    else
    {
        LogError(L"WriteTrade(): Can't append to %ls",
                 (const wchar_t*)RsTrade.GetDefaultSQL());
    }
    RsTrade.Close();
    return bWrote;
}

///////////////////////////////////////////////////////////////////////////////

long
Trade_t::
WriteCardGroup(
          CDatabase&        db,
    const CardCollection_t& Items) const
{
//OutputDebugString(L"WriteCardGroup\r\n");
    long GroupId = GetNewCardGroup(db);
    GroupedCards_t grouped(&db);
    BOOL b = grouped.Open(CRecordset::dynaset, NULL, CRecordset::appendOnly);
    ASSERT(!!b);
    CardCollection_t::const_iterator it = Items.begin();
    for (; Items.end() != it; ++it)
    {
        if (grouped.CanAppend())
        {
            grouped.AddNew();
            grouped.m_groupid  = GroupId;
            grouped.m_cardid   = it->pCard->GetId();
            grouped.m_quantity = (LONG)it->Quantity;
            grouped.Update();
        }
        else
            LogError(L"Can't append to groupedcards_t");
    }
    grouped.Close();
    return GroupId;
}

////////////////////////////////////////////////////////////////////////////

long
Trade_t::
GetNewCardGroup(
    CDatabase& db) const
{
//OutputDebugString(L"GetNewCardGroup\r\n");
    Groups_t group(&db);
    BOOL b;
    b = group.Open(CRecordset::dynaset, NULL, CRecordset::appendOnly);
    ASSERT(!!b);
    if (!group.CanAppend())
    {
        LogError(L"Can't append to groups_t");
        return 0;
    }
    group.AddNew();
    group.m_value   = 0;
    if (!group.Update())
    {
        LogError(L"Can't update groups_t");
        return 0;
    }
    group.Requery();
    group.SetAbsolutePosition(-1);
    group.Close();
    return group.m_groupid;
}

///////////////////////////////////////////////////////////////////////////////

const wchar_t Trade_t::XmlElementName[]  = L"Trade";
const wchar_t Trade_t::XmlElementOffer[] = L"Offer";
const wchar_t Trade_t::XmlElementWant[]  = L"Want";

const wchar_t XmlElementInclude[]        = L"Include";

const wchar_t Trade_t::XmlAttributeId[]  = L"Id";

///////////////////////////////////////////////////////////////////////////////

bool
Trade_t::
ReadXmlFile(
    const wchar_t* pszFilename)
{
    return util::ReadXmlFile(this, pszFilename);
}

///////////////////////////////////////////////////////////////////////////////

bool
Trade_t::
WriteXmlFile(
    const wchar_t* pszFilename) const
{
    XmlFile_t<Trade_t> XmlFile(this, pszFilename);
    return XmlFile.Write(GetId());
}

///////////////////////////////////////////////////////////////////////////////
//
// <Trade Id="1">
//   <Offer>
//   </Offer>
//   <Want>
//   </Want>
// </Trade>
//

bool
Trade_t::
WriteXml(
    IXmlWriter* pWriter,
    TradeId_t   TradeId) const
{
    using namespace XmlUtil;    
    WriteElement_t TradeElement(pWriter, XmlElementName);
    TradeElement.WriteAttribute(XmlAttributeId, TradeId);
    if (!offer.Items.WriteXml(pWriter, XmlElementOffer))
        return false;
    if (!want.Items.WriteXml(pWriter, XmlElementWant))
        return false;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
// <Trade Id="1">
//   <Offer>
//   </Offer>
//   <Want>
//   </Want>
// </Trade>
//

bool
Trade_t::
ReadXml(
    IXmlReader* pReader)
{
    using namespace XmlUtil;
    // <Trade>
    ReadElement_t TradeElement(pReader, XmlElementName);
    TradeElement.ReadAttribute(XmlAttributeId, m_Id);
    if (!offer.Items.ReadXml(pReader, XmlElementOffer))
        return false;
    if (!want.Items.ReadXml(pReader, XmlElementWant))
        return false;
    return true;
}
