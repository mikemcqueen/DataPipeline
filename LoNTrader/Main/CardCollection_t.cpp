/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// CardCollectin_t.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CardCollection_t.h"
#include "LonCard_t.h"
#include "DbCards_t.h"
#include "DbGroupedCards_t.h"
#include "XmlFile.h"
#include "XmlUtil.h"
#include "XmlNames.h"
#include "LonCardSet_t.h"
#include "Services.h"
#include "Log.h"

//#include "boost/bind.hpp"

///////////////////////////////////////////////////////////////////////////////

const wchar_t CardValue_t::XmlElementName[]         = L"CardValue";
const wchar_t CardValue_t::XmlElementBuyAt[]        = L"BuyAt";
const wchar_t CardValue_t::XmlElementSellAt[]       = L"SellAt";

const wchar_t CardValueSet_t::XmlElementName[]      = L"CardValueSet";

const wchar_t CardQuantity_t::XmlElementName[]      = L"CardQuantity";
const wchar_t CardQuantity_t::XmlElementQuantity[]  = L"Quantity";

const wchar_t CardQuantityQueue_t::XmlElementName[] = L"CardQuantityQueue";

const wchar_t CardCollection_t::XmlElementName[]    = L"CardCollection";

///////////////////////////////////////////////////////////////////////////////
//
// TODO: Move to Xml wrapper class.
//

template<
    class Element_t,
    class AddFunc_t>
bool
//CardQuantity_t::
ReadXmlContainer(
          IXmlReader*  pReader,
    const wchar_t*     ElementName,
          AddFunc_t    AddFunc)
{
    if (!ReadNextXml(pReader, XmlNodeType_Element, ElementName))
        return false;

    // Attr: Count
    HRESULT hr = pReader->MoveToAttributeByName(XmlNames::AttributeCount, NULL);
    if (S_OK != hr)
    {
        LogError(L"ReadXmlContainer: MoveToAttributeByName(Count) failed (%08x)", hr);
        return false;
    }
    const wchar_t* pCount;
    hr = pReader->GetValue(&pCount, NULL);
    if (S_OK != hr)
    {
        LogError(L"ReadXmlContainer: GetValue(Count) failed (%08x)", hr);
        return false;
    }
    size_t Count = _wtoi(pCount);
    hr = pReader->MoveToElement();
    if (S_OK != hr)
    {
        LogError(L"ReadXmlContainer: MoveToElement() failed (%08x)", hr);
        return false;
    }
    while (0 < Count--)
    {
        Element_t Element;
        if (!Element.ReadXml(pReader))
            return false;
        AddFunc(Element);
    }
    return ReadNextXml(pReader, XmlNodeType_EndElement, ElementName);
}

///////////////////////////////////////////////////////////////////////////////

template<class T>
bool
//CardQuantity_t::
WriteXmlContainer(
          IXmlWriter* pWriter,
    const wchar_t*    ElementName,
    const T&          Container)
{
    HRESULT hr;
    hr = pWriter->WriteStartElement(NULL, ElementName, NULL);
    if (FAILED(hr))
    {
        LogError(L"WriteStartElement(): error is %08.8lx", hr);
        return false;
    }
    // Attr: Count
    wchar_t szCount[32];
    _snwprintf_s(szCount, _TRUNCATE, L"%d", Container.size());
    hr = pWriter->WriteAttributeString(NULL, XmlNames::AttributeCount, NULL, szCount);
    if (FAILED(hr))
    {
        LogError(L"WriteAttributeString, error is %08.8lx", hr);
        return false;
    }
    for(T::const_iterator it = Container.begin(); Container.end() != it; ++it)
    {
        it->WriteXml(pWriter);
    }
    hr = pWriter->WriteFullEndElement();
    if (FAILED(hr))
    {
        LogError(L"WriteEndElement, error is %08.8lx", hr);
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
// CardValue_t
//
///////////////////////////////////////////////////////////////////////////////

/*
CardValue_t::
CardValue_t() :
    Card(Card_t::DummyCard),
    BuyAt(0),
    SellAt(0)
{
}
*/

///////////////////////////////////////////////////////////////////////////////

bool
CardValue_t::
operator()(
    const CardValue_t& lhs,
    const CardValue_t& rhs) const
{
    return lhs.pCard->GetId() < rhs.pCard->GetId();
}

///////////////////////////////////////////////////////////////////////////////

bool
CardValue_t::
operator==(
    const CardValue_t& rhs) const
{
    return pCard->GetId() == rhs.pCard->GetId();
}

///////////////////////////////////////////////////////////////////////////////

void
CardValue_t::
Show() const
{
    LogAlways(L"%-40ls: BuyAt(%d) SellAt(%d) Margin(%d)",
              pCard->GetName(), BuyAt, SellAt, SellAt - BuyAt);
}

///////////////////////////////////////////////////////////////////////////////
//
// <CardValue>
//   <Card Name=""/>
//   <BuyAt>1000</BuyAt>
//   <SellAt>1500</SellAt>
// </CardValue>
//

bool
CardValue_t::
ReadXml(
          IXmlReader* pReader,
    const wchar_t*    ElementName)
{
    using namespace XmlUtil;
    // <CardValue>
    ReadElement_t CardValueElement(pReader, ElementName);
    //   <Card>
    LonCard_t Card(Card_t::ClassToken);
    if (!Card.ReadXml(pReader))
        return false;
    pCard = Services::GetCardSet().Find(Card.GetName());
    if (NULL == pCard)
    {
        LogError(L"ReadXml: Card not found (%ls)", Card.GetName());
        return false;
    }
    //   <BuyAt>
    {
        ReadElement_t BuyAtElement(pReader, XmlElementBuyAt);
        BuyAtElement.ReadValue(BuyAt);
    }
    //   <SellAt>
    {
        ReadElement_t SellAtElement(pReader, XmlElementSellAt);
        SellAtElement.ReadValue(SellAt);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
CardValue_t::
WriteXml(
          IXmlWriter* pWriter,
    const wchar_t*    ElementName) const
{
    using namespace XmlUtil;
    // <CardValue>
    WriteElement_t CardValueElement(pWriter, ElementName);
    //   <Card>
    static_cast<const LonCard_t*>(pCard)->WriteXml(pWriter);
    //   <BuyAt>
    {
        WriteElement_t BuyAtElement(pWriter, XmlElementBuyAt);
        BuyAtElement.WriteValue(BuyAt);
    }
    //   <SellAt>
    {
        WriteElement_t SellAtElement(pWriter, XmlElementSellAt);
        SellAtElement.WriteValue(SellAt);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
// CardValueSet_t
//
///////////////////////////////////////////////////////////////////////////////

template<class T>
CardValueSet_t::
CardValueSet_t(
    const T& Cards)
{
    AddCards(Cards);
}

///////////////////////////////////////////////////////////////////////////////

template<class T>
size_t
CardValueSet_t::
AddCards(
    const T& Cards)
{
    size_t Count = 0;
    T::const_iterator it = Cards.begin();
    for (; Cards.end() != it; ++it)
    {
        if (insert(CardValue_t(*it->pCard)).second)
            ++Count;
    }
    return Count;
}

///////////////////////////////////////////////////////////////////////////////
//
// Add or change a card value.
//
// TODO: AddOrUpdateCardValue
//
// This whole thing isn't designed well. Should probably have a reference to
// a struct containing buyAt/SellAt prices, and just update the reference?
// Maybe.  I forget how std::set hashes are calculated for value-types.
// or is it just using operator==?
// (err.. is hash_set default? or is this an ordered set?)

void
CardValueSet_t::
SetCardValue(
    const CardValue_t& CardValue)
{
	CardValue;
	/* NOTE commented out to get it compiling. Looks like i was up to some kind of clever trickery here but i have no idea what.
erase(
        remove_if(
            begin(), end(),
            boost::bind(std::equal_to<CardValue_t>(), _1, CardValue)),
        end());
    bool bInserted = insert(CardValue).second;
    ASSERT(bInserted);
	*/
}

///////////////////////////////////////////////////////////////////////////////

void
CardValueSet_t::
Show(
    const wchar_t* pHeader) const
{
    if (NULL == pHeader) 
        pHeader = L"CardValueSet_t";
    LogAlways(L"++%ls (%d)", pHeader, size());
#if 0
    std::for_each(begin(), end(), boost::bind(CardValue_t::Show(), _1));
#else
    for (const_iterator it = begin(); end() != it; ++it)
        it->Show();
#endif
    LogAlways(L"--%ls", pHeader);
}

///////////////////////////////////////////////////////////////////////////////

size_t
CardValueSet_t::
ReadXmlFile(
    const wchar_t* pszFilename)
{
    return util::ReadXmlFile(this, pszFilename);
}

/////////////////////////////////////////////////////////////////////////////

bool
CardValueSet_t::
WriteXmlFile(
    const wchar_t* pszFilename) const
{
    return util::WriteXmlFile(this, pszFilename);
}
///////////////////////////////////////////////////////////////////////////////
//
// <CardValueSet Count="1">
//   <CardValue>
//   </CardValue>
// </CardValueSet>
//

bool
CardValueSet_t::
ReadXml(
          IXmlReader* pReader,
    const wchar_t*    ElementName)
{
    struct Insert
    {
        CardValueSet_t* pColl;
        explicit
        Insert(CardValueSet_t* pC) : pColl(pC) {}
        void operator()(const CardValue_t& cv)
        {
            pColl->insert(cv);
        }
    };
    return ReadXmlContainer<CardValue_t>(pReader, ElementName, Insert(this));
}

///////////////////////////////////////////////////////////////////////////////

bool
CardValueSet_t::
WriteXml(
          IXmlWriter* pWriter,
    const wchar_t*    ElementName) const
{
    return WriteXmlContainer(pWriter, ElementName, *this);
}

///////////////////////////////////////////////////////////////////////////////
//
// CardQuantity_t
//
///////////////////////////////////////////////////////////////////////////////

CardQuantity_t::
CardQuantity_t(
    const Card_t* pC,
          size_t  Count )
:
    pCard(pC),
    CardId(pC->GetId()),
    Quantity(Count)
{ }

///////////////////////////////////////////////////////////////////////////////

CardQuantity_t::
CardQuantity_t(
    CardId_t Id ,
    size_t   Count )
:
    pCard(NULL),
    CardId(Id),
    Quantity(Count)
{ }

///////////////////////////////////////////////////////////////////////////////

bool
CardQuantity_t::
IsBooster::
operator()(
    const CardQuantity_t& CardQ) const
{
    return static_cast<const LonCard_t*>(CardQ.pCard)->IsBoosterPack();
}

///////////////////////////////////////////////////////////////////////////////

#if 0
template<
    class AddFunc_t>
bool
//CardQuantity_t::
ReadXmlContainer(
          IXmlReader*  pReader,
    const wchar_t*     ElementName,
          AddFunc_t    AddFunc)
{
    if (!ReadNextXml(pReader, XmlNodeType_Element, ElementName))
        return false;

    // Attr: Count
    HRESULT hr = pReader->MoveToAttributeByName(XmlNames::AttributeCount, NULL);
    if (S_OK != hr)
    {
        LogError(L"ReadXml: MoveToAttributeByName(Count) failed (%08x)", hr);
        return false;
    }
    const wchar_t* pCount;
    hr = pReader->GetValue(&pCount, NULL);
    if (S_OK != hr)
    {
        LogError(L"ReadXml: GetValue(Count) failed (%08x)", hr);
        return false;
    }
    size_t Count = _wtoi(pCount);
    hr = pReader->MoveToElement();
    if (S_OK != hr)
    {
        LogError(L"ReadXml: MoveToElement() failed (%08x)", hr);
        return false;
    }
    while (0 < Count--)
    {
        CardQuantity_t CardQuantity;
        if (!CardQuantity.ReadXml(pReader))
            return false;
        AddFunc(CardQuantity);
    }
    return ReadNextXml(pReader, XmlNodeType_EndElement, ElementName);
}

///////////////////////////////////////////////////////////////////////////////

template<class T>
bool
CardQuantity_t::
WriteXmlContainer(
          IXmlWriter* pWriter,
    const wchar_t*    ElementName,
    const T&          Container)
{
    HRESULT hr;
    hr = pWriter->WriteStartElement(NULL, ElementName, NULL);
    if (FAILED(hr))
    {
        LogError(L"WriteStartElement(): error is %08.8lx", hr);
        return false;
    }
    // Attr: Count
    wchar_t szCount[32];
    _snwprintf_s(szCount, _TRUNCATE, L"%d", Container.size());
    hr = pWriter->WriteAttributeString(NULL, XmlNames::AttributeCount, NULL, szCount);
    if (FAILED(hr))
    {
        LogError(L"WriteAttributeString, error is %08.8lx", hr);
        return false;
    }
    for(T::const_iterator it = Container.begin(); Container.end() != it; ++it)
    {
        it->WriteXml(pWriter);
    }
    hr = pWriter->WriteEndElement();
    if (FAILED(hr))
    {
        LogError(L"WriteEndElement, error is %08.8lx", hr);
        return false;
    }
    return true;
}
#endif

///////////////////////////////////////////////////////////////////////////////
//
// <CardQuantity>
//   <Card Name=""/>
//   <Quantity>1</Quantity>
// </CardQuantity>
//

bool
CardQuantity_t::
ReadXml(
          IXmlReader* pReader,
    const wchar_t*    ElementName)
{
    using namespace XmlUtil;
    // <CardQuantity>
    ReadElement_t CardQuantityElement(pReader, ElementName);
    //   <Card>
    LonCard_t Card(Card_t::ClassToken);
    if (!Card.ReadXml(pReader))
        return false;
    pCard = Services::GetCardSet().Find(Card.GetName());
    if (NULL == pCard)
    {
        LogError(L"ReadXml: Card not found (%ls)", Card.GetName());
        return false;
    }
    CardId = pCard->GetId();
    //   <Quantity>
    {
        ReadElement_t QuantityElement(pReader, XmlElementQuantity);
        QuantityElement.ReadValue(Quantity);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
CardQuantity_t::
WriteXml(
          IXmlWriter* pWriter,
    const wchar_t*    ElementName) const
{
    using namespace XmlUtil;
    // <CardQuantity>
    WriteElement_t CardQuantityElement(pWriter, ElementName);
    //   <Card>
    static_cast<const LonCard_t*>(pCard)->WriteXml(pWriter);
    //   <Quantity>
    {
        WriteElement_t QuantityElement(pWriter, XmlElementQuantity);
        QuantityElement.WriteValue(Quantity);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
// CardQuantityQueue_t
//
///////////////////////////////////////////////////////////////////////////////

void 
CardQuantityQueue_t::
Show(
    const wchar_t* pHeader,
          Flag_t   Flags) const
{
Flags;
    if (NULL == pHeader)
        pHeader = L"CardQuantityQueue";
    LogAlways(L"%ls (%d)", pHeader, size());
    const_iterator it = begin();
    for(; end() != it; ++it)
    {
        LogAlways(L"  %3d x %ls", it->Quantity, it->pCard->GetName());
    }
}

///////////////////////////////////////////////////////////////////////////////

bool
CardQuantityQueue_t::
ReadXml(
          IXmlReader* pReader,
    const wchar_t*    ElementName)
{
    // TODO: boost::bind
    struct PushBack
    {
        CardQuantityQueue_t* pQueue;
        PushBack(CardQuantityQueue_t* pQ) : pQueue(pQ) {}
        void operator()(const CardQuantity_t& CardQ)
        {
            pQueue->push_back(CardQ);
        }
    };
    return ReadXmlContainer<CardQuantity_t>(pReader, ElementName, PushBack(this));
}

///////////////////////////////////////////////////////////////////////////////

bool
CardQuantityQueue_t::
WriteXml(
          IXmlWriter* pWriter,
    const wchar_t*    ElementName) const
{
    return WriteXmlContainer(pWriter, ElementName, *this);
}

///////////////////////////////////////////////////////////////////////////////
//
// CardCollection_t
//
///////////////////////////////////////////////////////////////////////////////

bool
CardCollection_t::
Compare(
    const CardCollection_t& Other) const
{
    if (size() != Other.size())
        return false;
    const_iterator it = begin();
    const_iterator itOther = Other.begin();
    for (; (end() != it) && (Other.end() != itOther); ++it, ++itOther)
    {
        if (!it->Compare(*itOther))
            return false;
    }
    return (end() == it) && (Other.end() == itOther); 
}

///////////////////////////////////////////////////////////////////////////////
//
// Return true if this collection contains only the cards present in the
// supplied Cards.
//

bool
CardCollection_t::
HasOnly(
    const CardCollection_t& Cards,
          bool              bCompareQuantity) const
{
    // TODO: Use set_intersection or something?
    if (empty())
        return false;
    const_iterator it = begin();
	const_iterator endThis = end();
	const_iterator endCards = Cards.end();
    for(; endThis != it; ++it)
    {
        const_iterator itCards = Cards.find(*it);
        if ((endCards == itCards) ||
            (bCompareQuantity && (itCards->Quantity < it->Quantity)))
        {
            return false;
        }
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////

size_t
CardCollection_t::
GetQuantity(
    const Card_t* pCard) const
{
    const_iterator it = find(CardQuantity_t(pCard));
    return (end() == it) ? 0 : it->Quantity;
}

/////////////////////////////////////////////////////////////////////////////

size_t
CardCollection_t::
Add(
    const CardCollection_t& Cards)
{
    size_t Added = 0;
    const_iterator itCards = Cards.begin();
	const_iterator endCards = Cards.end();
    for (; endCards != itCards; ++itCards)
    {
        Added += Add(*itCards);
    }
    return Added;
}

/////////////////////////////////////////////////////////////////////////////

size_t
CardCollection_t::
Add(
    const CardQuantity_t& CardQ)
{
    const size_t Added = CardQ.Quantity;
    iterator it = find(CardQ);
    if (end() != it)
    {
// NOTE set elements are apparently immutable now, so just commenting out all mutations to get it comipiling
//        it->Quantity += Added;
    }
    else
    {
        insert(CardQ);
    }
    return Added;
}

/////////////////////////////////////////////////////////////////////////////

size_t
CardCollection_t::
Remove(
    const CardCollection_t& Cards)
{
    size_t Removed = 0;
    const_iterator it = Cards.begin();
    for (; Cards.end() != it; ++it)
    {
        Removed += Remove(*it);
    }
    return Removed;
}

/////////////////////////////////////////////////////////////////////////////

size_t
CardCollection_t::
Remove(
    const CardQuantity_t& CardQ)
{
    size_t Removed = 0;
    iterator it = find(CardQ);
    if (end() != it)
    {
        if (CardQ.Quantity < it->Quantity)
        {
            Removed = CardQ.Quantity;
			// NOTE set elements are apparently immutable now, so just commenting out all mutations to get it comipiling
			//            it->Quantity -= Removed;
        }
        else
        {
            Removed = it->Quantity;
            erase(it);
        }
    }
    return Removed;
}

/////////////////////////////////////////////////////////////////////////////

DWORD ccLookupTicks;

size_t
CardCollection_t::
Read(
    CDatabase& db,
    long       GroupId)
{
    static const wchar_t szSqlFormat[] = 
        L"SELECT * FROM groupedcards_t WHERE groupid = %d";
    wchar_t szSql[256];

    size_t Count = 0;
    _snwprintf_s(szSql, _TRUNCATE, szSqlFormat, GroupId); 
    GroupedCards_t gc(&db);
    BOOL b = gc.Open(CRecordset::forwardOnly, szSql, CRecordset::readOnly);
    ASSERT(!!b);
    for (; !gc.IsEOF(); gc.MoveNext())
    {
//DWORD Ticks = GetTickCount64();
        const Card_t* pCard = Services::GetCardSet().Lookup(gc.m_cardid);
        ASSERT(NULL != pCard);
        if (NULL == pCard)
            break;
        // TODO: only  Count++ if insert succeeds?
        insert(CardQuantity_t(pCard, gc.m_quantity));
        Count++;
//ccLookupTicks += GetTickCount64() - Ticks;
    }
    gc.Close();
    return Count;
}

/////////////////////////////////////////////////////////////////////////////

void 
CardCollection_t::
Show(
          Flag_t   Flags,
          size_t   Order,
    const wchar_t* pHeader) const
{
    if (NULL == pHeader)
        pHeader = L"CardCollection";

    CardQuantityPtrVector_t SortedCards;
    CardCollection_t::const_iterator it(begin());
    for(; end() != it; ++it)
        SortedCards.push_back(&*it);

    using namespace ShowFlags;
    if (0 != (Flags & SortByValue))
    {
        struct SortByHighBid
        {
            bool operator()(const CardQuantity_t* lhs, const CardQuantity_t* rhs)
            {
                const LonCard_t* pLeft  = static_cast<const LonCard_t*>(lhs->pCard);
                const LonCard_t* pRight = static_cast<const LonCard_t*>(rhs->pCard);
                return pLeft->GetHighBid() < pRight->GetHighBid();
            }
        };
        std::sort(SortedCards.begin(), SortedCards.end(), SortByHighBid());
    }

    Flag_t ShowFlags = LonCard_t::Flag::ShowValues;
    if (0 != (Flags & Detail))
        ShowFlags |= LonCard_t::Flag::ShowDetail;
    bool bValuedOnly = 0 != (Flags & ValuedOnly);
    size_t Count = 0;
    LogAlways(L"++%ls(%d, %d)", pHeader, size(), GetTotalQuantity());
    CardQuantityPtrVector_t::const_iterator itSorted(SortedCards.begin());
    for(; SortedCards.end() != itSorted; ++itSorted)
    {
        const LonCard_t& Card = static_cast<const LonCard_t&>(*(*itSorted)->pCard);
        if (bValuedOnly && !Card.IsValued())
            continue;
        ++Count;
        Card.ShowValue(ShowFlags, Order, (*itSorted)->Quantity);
    }
    LogAlways(L"--%ls(%d)", pHeader, Count);
}

///////////////////////////////////////////////////////////////////////////////

size_t
CardCollection_t::
ReadXmlFile(
    const wchar_t* pszFilename)
{
    return util::ReadXmlFile(this, pszFilename);
}

/////////////////////////////////////////////////////////////////////////////

bool
CardCollection_t::
WriteXmlFile(
    const wchar_t* pszFilename) const
{
    return util::WriteXmlFile(this, pszFilename);
}

///////////////////////////////////////////////////////////////////////////////
//
// <CardCollection Count="1">
//   <CardQuantity>
//   </CardQuantity>
// </CardCollection>
//

bool
CardCollection_t::
ReadXml(
          IXmlReader* pReader,
    const wchar_t*    ElementName)
{
    struct Insert
    {
        CardCollection_t* pColl;
        explicit
        Insert(CardCollection_t* pC) : pColl(pC) {}
        void operator()(const CardQuantity_t& CardQ)
        {
            pColl->insert(CardQ);
        }
    };
    return ReadXmlContainer<CardQuantity_t>(pReader, ElementName, Insert(this));
}

///////////////////////////////////////////////////////////////////////////////

bool
CardCollection_t::
WriteXml(
          IXmlWriter* pWriter,
    const wchar_t*    ElementName) const
{
    return WriteXmlContainer(pWriter, ElementName, *this);
}

/////////////////////////////////////////////////////////////////////////////
