///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// LonCard_Xml.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LonCard_t.h"
#include "Log.h"
#include "XmlUtil.h"
#include "XmlFile.h"

///////////////////////////////////////////////////////////////////////////////

#include "XmlNames.h"

const wchar_t LonCard_t::XmlElementName[]     = L"Card";
const wchar_t LonCard_t::XmlAttributeName[]   = L"Name";
const wchar_t LonCard_t::XmlAttributeSet[]    = L"Set";
const wchar_t LonCard_t::XmlAttributeRarity[] = L"Rarity";
const wchar_t LonCard_t::XmlAttributeType[]   = L"Type";
const wchar_t LonCard_t::XmlAttributeFoil[]   = L"Foil";
const wchar_t LonCard_t::XmlAttributeBooster[]= L"Booster";

///////////////////////////////////////////////////////////////////////////////
//
// <Card Name="Name" />
//
bool
LonCard_t::
WriteXml(
    IXmlWriter* pWriter) const
{
    HRESULT hr;
    hr = pWriter->WriteStartElement(NULL, XmlElementName, NULL);
    if (FAILED(hr))
    {
        LogError(L"WriteStartElement(): error is %08.8lx", hr);
        return false;
    }
    hr = pWriter->WriteAttributeString(NULL, XmlAttributeName, NULL, GetName());
    if (FAILED(hr))
    {
        LogError(L"WriteAttributeString, error is %08.8lx", hr);
        return false;
    }
    hr = pWriter->WriteEndElement();
    if (FAILED(hr))
    {
        LogError(L"WriteEndElement, error is %08.8lx", hr);
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
// <Card Name="Name" />
//

bool
LonCard_t::
ReadXml(
    IXmlReader* pReader)
{
    if (!ReadNextXml(pReader, XmlNodeType_Element, XmlElementName))
        return false;
    HRESULT hr = pReader->MoveToAttributeByName(XmlAttributeName, NULL);
    if (S_OK != hr)
    {
        LogError(L"LonCard_t::ReadXml: MoveToAttributeByName(Name) (%08x)", hr);
        return false;
    }
    const wchar_t* pValue;
    hr = pReader->GetValue(&pValue, NULL);
    if (S_OK != hr)
    {
        LogError(L"LonCard_t::ReadXml: GetValue(Name) (%08x)", hr);
        return false;
    }
    m_strName.assign(pValue);
    hr = pReader->MoveToElement();
    if (S_OK != hr)
    {
        LogError(L"ReadXml: MoveToElement() failed (%08x)", hr);
        return false;
    }
    return true;
}

