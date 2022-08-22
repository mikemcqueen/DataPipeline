///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XmlUtil.h"
#include "Log.h"

namespace util
{

///////////////////////////////////////////////////////////////////////////////

bool
CreateXmlWriter(
          IXmlWriter** ppWriter,
    const wchar_t*     pszFilename)
{
    CComPtr<IXmlWriter> pWriter;
    CComPtr<IStream>    pOutFileStream;

    HRESULT hr;
    hr = SHCreateStreamOnFile(pszFilename, STGM_CREATE | STGM_WRITE, &pOutFileStream);
    if (FAILED(hr))
    {
        LogError(L"Error creating file writer, error is %08.8lx", hr);
        return false;
    }
    hr = ::CreateXmlWriter(__uuidof(IXmlWriter), reinterpret_cast<void**>(&pWriter), NULL);
    if (FAILED(hr))
    {
        LogError(L"Error creating xml writer, error is %08.8lx", hr);
        return false;
    }
    hr = pWriter->SetOutput(pOutFileStream);
    if (FAILED(hr))
    {
        LogError(L"Error, Method: SetOutput, error is %08.8lx", hr);
        return false;
    }
    hr = pWriter->SetProperty(XmlWriterProperty_Indent, TRUE);
    if (FAILED(hr))
    {
        LogError(L"Error, Method: SetProperty XmlWriterProperty_Indent, error is %08.8lx", hr);
        return false;
    }
/*
    hr = pWriter->WriteStartDocument(XmlStandalone_Omit);
    if (FAILED(hr))
    {
        LogError(L"WriteStartDocument, error is %08.8lx", hr);
        return hr;
    }
*/
    *ppWriter = pWriter.Detach();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
CreateXmlReader(
          IXmlReader** ppReader,
    const wchar_t*     pszFilename)
{
    CComPtr<IXmlReader> pReader;
    CComPtr<IStream>    pStream;

    HRESULT hr;
    hr = SHCreateStreamOnFile(pszFilename, STGM_READ, &pStream);
    if (FAILED(hr))
    {
        LogError(L"CreateStreamOnFile('%ls') %08.8lx", pszFilename, hr);
        return false;
    }
    hr = ::CreateXmlReader(__uuidof(IXmlReader), reinterpret_cast<void**>(&pReader), NULL);
    if (FAILED(hr))
    {
        LogError(L"::CreateXmlReader(), error is %08.8lx", hr);
        return false;
    }
    hr = pReader->SetInput(pStream);
    if (FAILED(hr))
    {
        LogError(L"Reader->SetInput(): error is %08.8lx", hr);
        return false;
    }
    hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit);
    if (FAILED(hr))
    {
        LogError(L"Reader->SetProperty(XmlReaderProperty_DtdProcessing), error is %08.8lx", hr);
        return false;
    }
    *ppReader = pReader.Detach();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace util

///////////////////////////////////////////////////////////////////////////////

static
std::wstring
NodeTypeToString(
    const XmlNodeType NodeType)
{
    switch (NodeType)
    {    
    case XmlNodeType_Element:        return L"Element";
    case XmlNodeType_Attribute:      return L"Attribute";
    case XmlNodeType_Text:           return L"Text";
    case XmlNodeType_CDATA:          return L"CDATA";
    case XmlNodeType_ProcessingInstruction: return L"ProcessingInstruction";
    case XmlNodeType_Comment:        return L"Comment";
    case XmlNodeType_DocumentType:   return L"DocumentType";
    case XmlNodeType_Whitespace:     return L"Whitespace";
    case XmlNodeType_EndElement:     return L"EndElement";
    case XmlNodeType_XmlDeclaration: return L"XmlDeclaration";
    default:                         break;
    }
    return L"UnknownNodeType";
}

///////////////////////////////////////////////////////////////////////////////

bool
ReadNextXml(
          IXmlReader* pReader,
    const XmlNodeType NodeType,
    const wchar_t*    pNodeName,
          bool        bQuiet)
{
bQuiet;
    for (;;)
    {
        XmlNodeType ReadType;
        HRESULT hr = pReader->Read(&ReadType);
        if (S_OK != hr)
        {
            if (FAILED(hr))
                LogError(L"ReadNextXml(%ls): Read failed, %08x", pNodeName, hr);
            return false;
        }
        if (NodeType != ReadType)
        {
            switch (ReadType)
            {
            case XmlNodeType_XmlDeclaration:
            case XmlNodeType_Whitespace:
            case XmlNodeType_Comment:
                continue;
            default:
                break;
            }
            if (!bQuiet)
            {
                LogError(L"ReadNextXml(%ls): NodeType (%ls) expected (%ls)",
                         pNodeName,
                         NodeTypeToString(ReadType).c_str(),
                         NodeTypeToString(NodeType).c_str());
            }
            return false;
        }
        if (NULL != pNodeName)
        {
            return CompareXmlNode(pReader, NodeType, pNodeName, bQuiet);
/*
            const wchar_t* pLocalName;
            hr = pReader->GetLocalName(&pLocalName, NULL);
            if (FAILED(hr))
            {
                LogError(L"ReadNextXml(): GetLocalName (%08x)", hr);
                return false;
            }
            if (0 != wcscmp(pNodeName, pLocalName))
            {
                LogError(L"ReadNextXml(): GetLocalName (%ls) expected (%ls)",
                         pLocalName, pNodeName);
                return false;
            }
*/
        }
        return true;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool
CompareXmlNode(
          IXmlReader* pReader,
    const XmlNodeType NodeType,
    const wchar_t*    pNodeName,
    bool bQuiet)
{
    XmlNodeType ReadType;
    HRESULT hr = pReader->GetNodeType(&ReadType);
    if (FAILED(hr))
    {
        LogError(L"CompareXmlNode(): GetNodeType (%08x)", hr);
        return false;
    }
    if (NodeType != ReadType)
    {
        if (!bQuiet)
        {
            LogError(L"CompareXmlNode(): NodeType (%d) expected (%d) (%ls)",
                     ReadType, NodeType, 
                     (NULL == pNodeName) ? L"" : pNodeName);
        }
        return false;
    }
    if (NULL != pNodeName)
    {
        const wchar_t* pLocalName;
        hr = pReader->GetLocalName(&pLocalName, NULL);
        if (FAILED(hr))
        {
            LogError(L"CompareXmlNode(): GetLocalName (%08x)", hr);
            return false;
        }
        if (0 != wcscmp(pNodeName, pLocalName))
        {
            if (!bQuiet)
            {
                LogError(L"CompareXmlNode(): GetLocalName (%ls) expected (%ls)",
                         pLocalName, pNodeName);
            }
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

namespace XmlUtil
{

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////

WriteElement_t::
WriteElement_t(
    IXmlWriter*    pWriter,
    const wchar_t* ElementName,
    bool           bFullEndElement)
:
    m_pWriter(pWriter),
    m_pElementName(ElementName),
    m_bFullEndElement(bFullEndElement)
{
    HRESULT hr = pWriter->WriteStartElement(NULL, ElementName, NULL);
    if (FAILED(hr))
    {
        LogError(L"WriteElement_t(%s) error is %08.8lx", ElementName, hr);
        throw std::runtime_error("WriteElement_t()");
    }
}

///////////////////////////////////////////////////////////////////////////////

WriteElement_t::
~WriteElement_t() noexcept(false) // NOTE: noexcept(false) added blindly for C++11 compatability
{
    HRESULT hr;
    hr = m_bFullEndElement ? m_pWriter->WriteFullEndElement()
                           : m_pWriter->WriteEndElement();
    if (FAILED(hr))
    {
        LogError(L"~WriteElement_t(%lss), error is %08.8lx", m_pElementName, hr);
        throw std::runtime_error("~WriteElement_t()");
    }
}

///////////////////////////////////////////////////////////////////////////////

void
WriteElement_t::
WriteValue(
    size_t Value)
{
    wchar_t szValue[16];
    _snwprintf_s(szValue, _TRUNCATE, L"%d", Value);
    HRESULT hr = m_pWriter->WriteString(szValue);
    if (FAILED(hr))
    {
        LogError(L"WriteValue(%d) failed (%08x)", Value, hr);
        throw std::runtime_error("WriteValue()");
    }
}

///////////////////////////////////////////////////////////////////////////////

void
WriteElement_t::
WriteAttribute(
    const wchar_t* pAttributeName,
          size_t   Value)
{
    wchar_t szValue[32];
    _snwprintf_s(szValue, _TRUNCATE, L"%d", Value);
    WriteAttribute(pAttributeName, szValue);
}

///////////////////////////////////////////////////////////////////////////////

void
WriteElement_t::
WriteAttribute(
    const wchar_t* pAttributeName,
    const wchar_t* Value)
{
    HRESULT hr = m_pWriter->WriteAttributeString(NULL, pAttributeName, NULL, Value);
    if (FAILED(hr))
    {
        LogError(L"WriteAttributeString(%ls) Value(%ls) failed (%08x)",
                 pAttributeName, Value, hr);
        throw std::runtime_error("WriteAttributeString()");
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////

ReadElement_t::
ReadElement_t(
          IXmlReader* pReader,
    const wchar_t*    ElementName,
          bool        bReadEnd)
:
    m_pReader(pReader),
    m_pElementName(ElementName),
    m_bReadEnd(bReadEnd)
{
    if (!ReadNextXml(pReader, XmlNodeType_Element, ElementName))
        throw std::runtime_error("ReadElement_t()");
}

///////////////////////////////////////////////////////////////////////////////

ReadElement_t::
~ReadElement_t() noexcept(false) // NOTE: noexcept(false) added blindly for C++11 compatability
{
    if (m_bReadEnd &&
        !ReadNextXml(m_pReader, XmlNodeType_EndElement, m_pElementName))
    {
        throw std::runtime_error("~ReadElement_t()");
    }
}

///////////////////////////////////////////////////////////////////////////////

void
ReadElement_t::
ReadValue(
    size_t& Value)
{
    if (!ReadNextXml(m_pReader, XmlNodeType_Text))
        throw std::runtime_error("ReadValue::ReadNextXml()");
    const wchar_t* pValue;
    HRESULT hr = m_pReader->GetValue(&pValue, NULL);
    if (FAILED(hr))
    {
        LogError(L"ReadValue::GetValue(%ls) failed (%08x)", m_pElementName, hr);
        throw std::runtime_error("ReadValue::GetValue()");
    }
    Value = _wtoi(pValue);
}

///////////////////////////////////////////////////////////////////////////////

void
ReadElement_t::
ReadAttribute(
    const wchar_t* pAttributeName,
          size_t&  Value)
{
    const wchar_t* pValue = NULL;
    ReadAttribute(pAttributeName, pValue);
    Value = _wtoi(pValue);
    HRESULT hr = m_pReader->MoveToElement();
    if (S_OK != hr)
    {
        LogError(L"MoveToElement(%ls) failed (%08x)", pAttributeName, hr);
        throw std::runtime_error("ReadAttribute::MoveToElement()");
    }
}

///////////////////////////////////////////////////////////////////////////////

void
ReadElement_t::
ReadAttribute(
    const wchar_t* pAttributeName,
    std::wstring&  Value)
{
    const wchar_t* pValue = NULL;
    ReadAttribute(pAttributeName, pValue);
    Value.assign(pValue);
    HRESULT hr = m_pReader->MoveToElement();
    if (S_OK != hr)
    {
        LogError(L"MoveToElement(%ls) failed (%08x)", pAttributeName, hr);
        throw std::runtime_error("ReadAttribute::MoveToElement()");
    }
}

///////////////////////////////////////////////////////////////////////////////

void
ReadElement_t::
ReadAttribute(
    const wchar_t*  pAttributeName,
    const wchar_t*& pValue)
{
    HRESULT hr;
    hr = m_pReader->MoveToAttributeByName(pAttributeName, NULL);
    if (S_OK != hr)
    {
        LogError(L"MoveToAttributeByName(%ls) failed (%08x)", pAttributeName, hr);
        throw std::runtime_error("MoveToAttributeByName");
    }
    hr = m_pReader->GetValue(&pValue, NULL);
    if (S_OK != hr)
    {
        LogError(L"ReadAttribute::GetValue(%ls) failed (%08x)", pAttributeName, hr);
        throw std::runtime_error("ReadAttribute::GetValue()");
    }
}

} // XmlUtil

///////////////////////////////////////////////////////////////////////////////

