///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// PassiveFixedPricing_t.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PassiveFixedPricing_t.h"
#include "LonCard_t.h"
#include "XmlUtil.h"
#include "XmlFile.h"
#include "Log.h"
#include "TradeExecutor_t.h"
#include "TradeManager_t.h"
#include "Services.h"

///////////////////////////////////////////////////////////////////////////////

namespace TradePoster
{

const wchar_t PricingPolicy_i::XmlElementName[]         = L"PricingPolicy";
const wchar_t PostingPolicy_i::XmlElementName[]         = L"PostingPolicy";

const wchar_t PassiveFixedPricing_t::PolicyName[]       = L"PassiveFixed";

///////////////////////////////////////////////////////////////////////////////

PassiveFixedPricing_t::
PassiveFixedPricing_t() :
    m_Price(0)
{
}

///////////////////////////////////////////////////////////////////////////////

void
PassiveFixedPricing_t::
OnTradePosted(
    const Trade_t&  Trade,
          TradeId_t NewTradeId,
          size_t    Value)
{
Trade;NewTradeId;Value;
}

///////////////////////////////////////////////////////////////////////////////

void
PassiveFixedPricing_t::
OnTradeAdded(
    const Trade_t& Trade)
{
Trade;
}

///////////////////////////////////////////////////////////////////////////////

void
PassiveFixedPricing_t::
OnTradeRemoved(
    const Data_t&   Data,
          TradeId_t TradeId)
{
Data; TradeId;
}

///////////////////////////////////////////////////////////////////////////////

void
PassiveFixedPricing_t::
Show(
    bool bDetail) const
{
    bDetail;
    LogAlways(L"%ls: Price(%d)", PolicyName, m_Price);
}

///////////////////////////////////////////////////////////////////////////////
//
// <PassiveFixed>
//   <Price>12345</Price>
// </PassiveFixed>
//

bool
PassiveFixedPricing_t::
ReadXml(
          IXmlReader* pReader,
    const wchar_t*    ElementName)
{
    if (!ReadNextXml(pReader, XmlNodeType_Element, ElementName))
        return false;
    if (!ReadNextXml(pReader, XmlNodeType_Element, XmlElementPrice))
        return false;
    if (!ReadNextXml(pReader, XmlNodeType_Text))
        return false;
    const wchar_t* pValue;
    HRESULT hr = pReader->GetValue(&pValue, NULL);
    if (FAILED(hr))
    {
        LogError(L"ReadXml: GetValue() failed (%08x)", hr);
        return false;
    }
    m_Price = _wtoi(pValue);
    if (!ReadNextXml(pReader, XmlNodeType_EndElement, XmlElementPrice))
        return false;
    return ReadNextXml(pReader, XmlNodeType_EndElement, ElementName);
}

///////////////////////////////////////////////////////////////////////////////
//
// <PassiveFixed>
//   <Price>12345</Price>
// </PassiveFixed>
//

bool
PassiveFixedPricing_t::
WriteXml(
          IXmlWriter* pWriter,
    const wchar_t*    ElementName) const
{
    HRESULT hr;
    hr = pWriter->WriteStartElement(NULL, ElementName, NULL);
    if (FAILED(hr))
    {
        LogError(L"WriteStartElement() failed %08x", hr);
        return false;
    }
    hr = pWriter->WriteStartElement(NULL, XmlElementPrice, NULL);
    if (FAILED(hr))
    {
        LogError(L"WriteStartElement() failed %08x", hr);
        return false;
    }
    wchar_t szPrice[16];
    _snwprintf_s(szPrice, _TRUNCATE, L"%d", m_Price);
    hr = pWriter->WriteString(szPrice);
    if (FAILED(hr))
    {
        LogError(L"WriteString() failed %08x", hr);
        return false;
    }
    hr = pWriter->WriteFullEndElement();
    if (FAILED(hr))
    {
        LogError(L"WriteEndElement() failed %08x", hr);
        return false;
    }
    hr = pWriter->WriteFullEndElement();
    if (FAILED(hr))
    {
        LogError(L"WriteEndElement() failed %08x", hr);
        return false;
    }
    return true;
}
///////////////////////////////////////////////////////////////////////////////

}; // namespace TradePoster

///////////////////////////////////////////////////////////////////////////////
