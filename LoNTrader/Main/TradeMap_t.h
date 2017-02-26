///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TradeMap_t.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TRADEMAP_T_H
#define Include_TRADEMAP_T_H

#include "Trade_t.h"
#include "Log.h"

#define TRADEMAP_TEMPLATE


///////////////////////////////////////////////////////////////////////////////
//
// TradeMap_t
//
///////////////////////////////////////////////////////////////////////////////

typedef std::unordered_map<TradeId_t, Trade_t>  TradeMapContainer_t;
typedef std::list<Trade_t>                      TradeListContainer_t;

template<class Container_t>
class TradeContainer_t :
    public Container_t
{

private:

    static const wchar_t XmlElementName[];

public:

#ifndef TRADEMAP_TEMPLATE

    void
    Show(
        const wchar_t* pszText) const;

    void
    bool
    HasTrade(
        TradeId_t TradeId) const
    {
        return end() != find(TradeId);
    }

/*
    Replace(
        TradeId_t OldTradeId,
        TradeId_t NewTradeId);

    void
    Replace(
        typename Container_t::iterator& itTrade,
        TradeId_t NewTradeId);
*/

    bool
    WriteXml(
              IXmlWriter* pWriter,
        const wchar_t*    ElementName = XmlElementName) const;

    bool
    ReadXml(
              IXmlReader* pReader,
        const wchar_t*    ElementName = XmlElementName);

    size_t
    WriteTrades(
        CDatabase& db) const;
#else

void
Show(
    const wchar_t* pszText) const
{
    if (NULL == pszText)
        pszText = L"TradeMap_t";
    LogAlways(L"++%ls: (%d)", pszText, size());
    for (Container_t::const_iterator it = begin(); end() != it; ++it)
        it->second.Show();
//  for_each(begin(), end(), boost::bind())
    LogAlways(L"--%ls: (%d)", pszText, size());
}

///////////////////////////////////////////////////////////////////////////////

    typename Container_t::iterator
    Find(TradeId_t TradeId)
    {
        return find(TradeId);
    }

///////////////////////////////////////////////////////////////////////////////

    bool
    HasTrade(
        TradeId_t TradeId) const
    {
        return end() != find(TradeId);
    }

///////////////////////////////////////////////////////////////////////////////

/*
void
Replace(
    TradeId_t OldTradeId,
    TradeId_t NewTradeId)
{
    iterator it = find(OldTradeId);
    if (end() == it)
    {
        LogError(L"TradeMap_t::Replace(%d,%d) failed", OldTradeId, NewTradeId);
        throw std::runtime_error("TradeMap_t::Replace() OldTradeId not found");
    }
    Replace(it, NewTradeId);
}

///////////////////////////////////////////////////////////////////////////////

void
Replace(
    typename Container_t::iterator& itTrade,
             TradeId_t              NewTradeId)
{
    const TradeId_t OldTradeId = itTrade->second.GetId();
    Trade_t::Pair_t NewTradePair(NewTradeId, itTrade->second);
    NewTradePair.second.SetId(NewTradeId);
    erase(itTrade);
    if (!insert(NewTradePair).second)
    {
        LogError(L"TradeMap_t::Replace() insertion failed (%d)", NewTradeId);
        throw std::runtime_error("TradeMap_t::Replace() insertion failed");
    }
    LogAlways(L"TradeMap_t::Replace(%d)->(%d) done", OldTradeId, NewTradeId);
}
*/

///////////////////////////////////////////////////////////////////////////////
//
// <TradeMap Count="1">
//   <Trade>
//   </Trade>
// </TradeMap>
//

bool
ReadXml(
          IXmlReader* pReader,
    const wchar_t*    ElementName)
{
    if (!ReadNextXml(pReader, XmlNodeType_Element, ElementName))
        return false;
    HRESULT hr;
    hr = pReader->MoveToAttributeByName(XmlNames::AttributeCount, NULL);
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
        Trade_t Trade;
        if (!Trade.ReadXml(pReader))
            return false;
        // TODO: writing over the name like this is fun and everything but..
        Trade.SetUser(LonTrader_t::GetPlayer().GetName());
        bool bInserted = Insert(Trade);
        ASSERT(bInserted);
    }
    return ReadNextXml(pReader, XmlNodeType_EndElement, ElementName);
}

bool
Insert(const Trade_t& Trade)
{
    insert(Trade_t::Pair_t(Trade.GetId(), Trade)).second;
}

///////////////////////////////////////////////////////////////////////////////
//
// <TradeMap Count="1">
//   <Trade>
//   </Trade>
// </TradeMap>
//

bool
WriteXml(
          IXmlWriter* pWriter,
    const wchar_t*    ElementName) const
{
    HRESULT hr;

    // <TradeMap>
    hr = pWriter->WriteStartElement(NULL, ElementName, NULL);
    if (FAILED(hr))
    {
        LogError(L"WriteStartElement() failed %08.8lx", hr);
        return false;
    }
    // Attribute: Count="N"
    wchar_t szCount[32];
    _snwprintf_s(szCount, _TRUNCATE, L"%d", size());
    hr = pWriter->WriteAttributeString(NULL, XmlNames::AttributeCount, NULL, szCount);
    if (FAILED(hr))
    {
        LogError(L"WriteAttributeString() failed %08.8lx", hr);
        return false;
    }

    TradeId_t TradeId = 0;
    for(const_iterator it = begin(); end() != it; ++it, ++TradeId)
    {
        // <Trade Id="N">
        // </Trade>
        const Trade_t& Trade = GetTrade(it); // it->second;
        Trade.WriteXml(pWriter, TradeId);
    }

    // </TradeMap>
    hr = pWriter->WriteFullEndElement();
    if (FAILED(hr))
    {
        LogError(L"WriteEndElement() failed %08.8lx", hr);
        return false;
    }
    return true;
}

const Trade_t&
GetTrade(typename Container_t::const_iterator it) const
 { return it->second; }

///////////////////////////////////////////////////////////////////////////////

size_t
WriteTrades(
    CDatabase& db) const
{
    size_t Count = 0;
    TradeMap_t::const_iterator it = begin();
    for (; end() != it; ++it)
    {
        const Trade_t& Trade = it->second;
        if (Trade.WriteTrade(db))
            ++Count;
    }
    return Count;
}
#endif

};


typedef TradeContainer_t<TradeMapContainer_t>  TradeMap_t;
typedef TradeContainer_t<TradeListContainer_t> TradeList_t;

template<>
bool
TradeContainer_t<TradeListContainer_t>::
HasTrade(
    TradeId_t TradeId) const
{
TradeId;
    return false;
}

///////////////////////////////////////////////////////////////////////////////

template<>
void
TradeContainer_t<TradeListContainer_t>::
Show(
    const wchar_t* pszText) const
{
    if (NULL == pszText)
        pszText = L"TradeListn_t";
    LogAlways(L"++%ls: (%d)", pszText, size());
    for (TradeListContainer_t::const_iterator it = begin(); end() != it; ++it)
        it->Show();
//  for_each(begin(), end(), boost::bind())
    LogAlways(L"--%ls: (%d)", pszText, size());
}

template<>
bool
TradeContainer_t<TradeListContainer_t>::
Insert(const Trade_t& Trade)
{
    push_back(Trade);
    return true;
}

template<>
const Trade_t&
TradeContainer_t<TradeListContainer_t>::
GetTrade(TradeListContainer_t::const_iterator it)const 
{ return *it; }


template<>
TradeListContainer_t::iterator
TradeContainer_t<TradeListContainer_t>::
Find(TradeId_t TradeId)
{
    TradeListContainer_t::iterator it = begin();
    for (; end() != it; ++it)
    {
        if (TradeId == it->GetId())
            break;
    }
    return it;
}

///////////////////////////////////////////////////////////////////////////////
//
// TradeIdSet_t
//
///////////////////////////////////////////////////////////////////////////////

class TradeIdSet_t :
    public std::set<TradeId_t>
{

public:

    TradeIdSet_t()
    { }

    TradeIdSet_t(
        const TradeList_t& List);

    TradeIdSet_t(
        const TradeMap_t& Map);

    void
    Show(
        const wchar_t* pszText = NULL) const;
};

///////////////////////////////////////////////////////////////////////////////

#endif // Include_TRADEMAP_T_H

///////////////////////////////////////////////////////////////////////////////
