///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// SimpleRangePricing_t.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SimpleRangePricing_t.h"
#include "LonCard_t.h"
#include "XmlUtil.h"
#include "XmlFile.h"
#include "Log.h"
#include "TradeExecutor_t.h"
#include "TradeManager_t.h"
#include "Services.h"

namespace TradePoster
{

const wchar_t SimpleRangePricing_t::PolicyName[]             = L"Simple";
const wchar_t SimpleRangePricing_t::XmlElementHigh[]         = L"High";
const wchar_t SimpleRangePricing_t::XmlElementLow[]          = L"Low";
const wchar_t SimpleRangePricing_t::XmlElementIncrement[]    = L"Increment";
const wchar_t SimpleRangePricing_t::XmlElementCurrent[]      = L"Current";

///////////////////////////////////////////////////////////////////////////////

SimpleRangePricing_t::
SimpleRangePricing_t(
    size_t LowPrice,
    size_t HighPrice,
    size_t PriceIncrement)
:
    m_HighPrice(HighPrice),
    m_LowPrice(LowPrice),
    m_PriceIncrement(PriceIncrement),
    m_CurrentPrice(0)
{
}

///////////////////////////////////////////////////////////////////////////////

SimpleRangePricing_t::
SimpleRangePricing_t() :
    m_HighPrice(0),
    m_LowPrice(0),
    m_PriceIncrement(0),
    m_CurrentPrice(0)
{
}

///////////////////////////////////////////////////////////////////////////////

void
SimpleRangePricing_t::
OnTradePosted(
    const Trade_t&  Trade,
          TradeId_t NewTradeId,
          size_t    Value)
{
Trade; NewTradeId; Value;
}

///////////////////////////////////////////////////////////////////////////////

void
SimpleRangePricing_t::
OnTradeAdded(
    const Trade_t& Trade)
{
Trade;
}

///////////////////////////////////////////////////////////////////////////////

void
SimpleRangePricing_t::
OnTradeRemoved(
    const Data_t&   Data,
          TradeId_t TradeId)
{
TradeId;Data;
}

///////////////////////////////////////////////////////////////////////////////

void
SimpleRangePricing_t::
SetPrice(
    size_t Low,
    size_t High,
    size_t Step,
    size_t Current)
{
    ASSERT(Low <= High);
    ASSERT((Current >= Low) && (Current <= High));
    m_LowPrice = Low;
    m_HighPrice = High;
    m_PriceIncrement = Step;
    m_CurrentPrice = Current;
    Show();
}

///////////////////////////////////////////////////////////////////////////////

void
SimpleRangePricing_t::
Show(
    bool bDetail) const
{
    bDetail;
    LogAlways(L"%ls: Low(%d) High(%d) Step(%d) Current(%d)",
        PolicyName, m_LowPrice, m_HighPrice, m_PriceIncrement, m_CurrentPrice);
}

///////////////////////////////////////////////////////////////////////////////
//
// <SimpleRangePricing>
//   <Low>1000</Low>
//   <High>2000</High>
//   <Increment>500</Low>
//   <Current>1000</Current>
// </SimpleRangePricing>
//

bool
SimpleRangePricing_t::
ReadXml(
          IXmlReader* pReader,
    const wchar_t*    ElementName)
{
    HRESULT hr;

    if (!ReadNextXml(pReader, XmlNodeType_Element, ElementName))
        return false;

    // Low
    if (!ReadNextXml(pReader, XmlNodeType_Element, XmlElementLow))
        return false;
    if (!ReadNextXml(pReader, XmlNodeType_Text))
        return false;
    const wchar_t* pLow;
    hr = pReader->GetValue(&pLow, NULL);
    if (FAILED(hr))
    {
        LogError(L"ReadXml: GetValue(Low) failed (%08x)", hr);
        return false;
    }
    m_LowPrice = _wtoi(pLow);
    if (!ReadNextXml(pReader, XmlNodeType_EndElement, XmlElementLow))
        return false;

    // High
    if (!ReadNextXml(pReader, XmlNodeType_Element, XmlElementHigh))
        return false;
    if (!ReadNextXml(pReader, XmlNodeType_Text))
        return false;
    const wchar_t* pHigh;
    hr = pReader->GetValue(&pHigh, NULL);
    if (FAILED(hr))
    {
        LogError(L"ReadXml: GetValue(High) failed (%08x)", hr);
        return false;
    }
    m_HighPrice = _wtoi(pHigh);
    if (!ReadNextXml(pReader, XmlNodeType_EndElement, XmlElementHigh))
        return false;

    // Increment
    if (!ReadNextXml(pReader, XmlNodeType_Element, XmlElementIncrement))
        return false;
    if (!ReadNextXml(pReader, XmlNodeType_Text))
        return false;
    const wchar_t* pIncrement;
    hr = pReader->GetValue(&pIncrement, NULL);
    if (FAILED(hr))
    {
        LogError(L"ReadXml: GetValue(Increment) failed (%08x)", hr);
        return false;
    }
    m_PriceIncrement = _wtoi(pIncrement);
    if (!ReadNextXml(pReader, XmlNodeType_EndElement, XmlElementIncrement))
        return false;

    // Current
    if (!ReadNextXml(pReader, XmlNodeType_Element, XmlElementCurrent))
        return false;
    if (!ReadNextXml(pReader, XmlNodeType_Text))
        return false;
    const wchar_t* pCurrent;
    hr = pReader->GetValue(&pCurrent, NULL);
    if (FAILED(hr))
    {
        LogError(L"ReadXml: GetValue(Current) failed (%08x)", hr);
        return false;
    }
    m_CurrentPrice = _wtoi(pCurrent);
    if (!ReadNextXml(pReader, XmlNodeType_EndElement, XmlElementCurrent))
        return false;

    return ReadNextXml(pReader, XmlNodeType_EndElement, ElementName);
}

///////////////////////////////////////////////////////////////////////////////
//
// <SimpleRangePricing>
//   <Low>1000</Low>
//   <High>2000</High>
//   <Increment>500</Low>
//   <Current>1000</Current>
// </SimpleRangePricing>
//

bool
SimpleRangePricing_t::
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
#ifdef WRITE_POLICYNAME
    hr = pWriter->WriteAttributeString(NULL, XmlAttributeName, NULL, PolicyName);
    if (FAILED(hr))
    {
        LogError(L"WriteAttributeString, error is %08.8lx", hr);
        return false;
    }
#endif
    // Low
    hr = pWriter->WriteStartElement(NULL, XmlElementLow, NULL);
    if (FAILED(hr))
    {
        LogError(L"WriteStartElement() failed %08x", hr);
        return false;
    }
    wchar_t szLow[16];
    _snwprintf_s(szLow, _TRUNCATE, L"%d", m_LowPrice);
    hr = pWriter->WriteString(szLow);
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

    // High
    hr = pWriter->WriteStartElement(NULL, XmlElementHigh, NULL);
    if (FAILED(hr))
    {
        LogError(L"WriteStartElement() failed %08x", hr);
        return false;
    }
    wchar_t szHigh[16];
    _snwprintf_s(szHigh, _TRUNCATE, L"%d", m_HighPrice);
    hr = pWriter->WriteString(szHigh);
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

    // Increment
    hr = pWriter->WriteStartElement(NULL, XmlElementIncrement, NULL);
    if (FAILED(hr))
    {
        LogError(L"WriteStartElement(Increment) failed %08x", hr);
        return false;
    }
    wchar_t szIncrement[16];
    _snwprintf_s(szIncrement, _TRUNCATE, L"%d", m_PriceIncrement);
    hr = pWriter->WriteString(szIncrement);
    if (FAILED(hr))
    {
        LogError(L"WriteString(Inc) failed %08x", hr);
        return false;
    }
    hr = pWriter->WriteFullEndElement();
    if (FAILED(hr))
    {
        LogError(L"WriteEndElement() failed %08x", hr);
        return false;
    }

    // Current
    hr = pWriter->WriteStartElement(NULL, XmlElementCurrent, NULL);
    if (FAILED(hr))
    {
        LogError(L"WriteStartElement(Cur) failed %08x", hr);
        return false;
    }
    wchar_t szCurrent[16];
    _snwprintf_s(szCurrent, _TRUNCATE, L"%d", m_CurrentPrice);
    hr = pWriter->WriteString(szCurrent);
    if (FAILED(hr))
    {
        LogError(L"WriteString(Cur) failed %08x", hr);
        return false;
    }
    hr = pWriter->WriteFullEndElement();
    if (FAILED(hr))
    {
        LogError(L"WriteEndElement(Cur) failed %08x", hr);
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

} // TradePoster

///////////////////////////////////////////////////////////////////////////////
