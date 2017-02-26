///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TradeTypes.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TradeMap_t.h"
#include "TradeTypes.h"
#include "LonTrader_t.h"
#include "LonPlayer_t.h"
#include "XmlUtil.h"
#include "XmlNames.h"
#include "Log.h"

///////////////////////////////////////////////////////////////////////////////
//
// TradeMap_t
//
///////////////////////////////////////////////////////////////////////////////

const wchar_t TradeMap_t::XmlElementName[] = L"TradeMap";

#ifndef TRADEMAP_TEMPLATE
///////////////////////////////////////////////////////////////////////////////

void
TradeMap_t::
Show(
    const wchar_t* pszText) const
{
    if (NULL == pszText)
        pszText = L"TradeMap_t";
    LogAlways(L"++%ls: (%d)", pszText, size());
    for (TradeMap_t::const_iterator it = begin(); end() != it; ++it)
        it->second.Show();
//  for_each(begin(), end(), boost::bind())
    LogAlways(L"--%ls: (%d)", pszText, size());
}

///////////////////////////////////////////////////////////////////////////////

void
TradeMap_t::
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
TradeMap_t::
Replace(
    iterator& itTrade,
    TradeId_t NewTradeId)
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

///////////////////////////////////////////////////////////////////////////////
//
// <TradeMap Count="1">
//   <Trade>
//   </Trade>
// </TradeMap>
//

bool
TradeMap_t::
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
        bool bInserted = insert(Trade_t::Pair_t(Trade.GetId(), Trade)).second;
        ASSERT(bInserted);
    }
    return ReadNextXml(pReader, XmlNodeType_EndElement, ElementName);
}

///////////////////////////////////////////////////////////////////////////////
//
// <TradeMap Count="1">
//   <Trade>
//   </Trade>
// </TradeMap>
//

bool
TradeMap_t::
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
        const Trade_t& Trade = it->second;
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

///////////////////////////////////////////////////////////////////////////////

size_t
TradeMap_t::
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

#endif // TRADEMAP_TEMPLATE

///////////////////////////////////////////////////////////////////////////////
//
// TradeIdSet_t
//
///////////////////////////////////////////////////////////////////////////////

TradeIdSet_t::
TradeIdSet_t(
    const TradeMap_t& Map)
{
    // TODO: some better way to do this using boost::bind
    TradeMap_t::const_iterator it = Map.begin();
    for (; Map.end() != it; ++it)
        insert(it->second.GetId());
}

TradeIdSet_t::
TradeIdSet_t(
    const TradeList_t& List)
{
    // TODO: some better way to do this using boost::bind
    TradeList_t::const_iterator it = List.begin();
    for (; List.end() != it; ++it)
        insert(it->GetId());
}

///////////////////////////////////////////////////////////////////////////////

void
TradeIdSet_t::
Show(
    const wchar_t* pszText) const 
{
    LogAlways(L"%s: (%d)", (NULL == pszText) ? L"TradeIdSet_t" : pszText, size());
    for(const_iterator it = begin(); end() != it; ++it)
    {
        LogAlways(L"  %d", *it);
    }
}

///////////////////////////////////////////////////////////////////////////////
