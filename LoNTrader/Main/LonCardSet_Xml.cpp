///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// LonCardSet_Xml.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LonCardSet_t.h"
#include "Log.h"
#include "XmlUtil.h"

///////////////////////////////////////////////////////////////////////////////

const wchar_t
LonCardSet_t::XmlElementName[] = L"CardSet";

///////////////////////////////////////////////////////////////////////////////

bool
LonCardSet_t::
WriteXml(
    const wchar_t* pszFilename) const
{
    LogAlways(L"LonCardSet_t::WriteXml(%ls)", pszFilename);

    HRESULT hr;
    CComPtr<IXmlWriter> pWriter;
    if (!util::CreateXmlWriter(&pWriter, pszFilename))
        return false;
    hr = pWriter->WriteStartDocument(XmlStandalone_Omit);
    if (FAILED(hr))
    {
        LogError(L"Error, Method: WriteStartDocument, error is %08.8lx", hr);
        return false;
    }

    if (WriteXml(pWriter))
        return false;

    if (FAILED(hr = pWriter->WriteEndDocument()))
    {
        LogError(L"Error, Method: WriteEndDocument, error is %08.8lx", hr);
        return false;
    }
    if (FAILED(hr = pWriter->Flush()))
    {
        LogError(L"Error, Method: Flush, error is %08.8lx", hr);
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
LonCardSet_t::
WriteXml(
    IXmlWriter* pWriter) const
{
    HRESULT hr;
    hr = pWriter->WriteStartElement(NULL, XmlElementName, NULL);
    if (FAILED(hr))
    {
        LogError(L"WriteStartElement, error is %08.8lx", hr);
        return false;
    }
    {
        CLock lock(m_csCards);
        LonCardMap_t::const_iterator it(m_Cards.begin());
        for (; m_Cards.end() != it; ++it)
        {
            const LonCard_t& Card = it->second;
            Card.WriteXml(pWriter);
        }
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
