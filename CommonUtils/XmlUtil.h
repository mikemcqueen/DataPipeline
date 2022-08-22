///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef Include_XMLUTIL_H
#define Include_XMLUTIL_H

///////////////////////////////////////////////////////////////////////////////

namespace util
{

bool CreateXmlWriter(IXmlWriter** ppWriter, const wchar_t* pszFilename);
bool CreateXmlReader(IXmlReader** ppReader, const wchar_t* pszFilename);

} // util

bool
ReadNextXml(
          IXmlReader* pReader,
    const XmlNodeType NodeType,
    const wchar_t*    pNodeName = NULL,
          bool        bQuiet    = false);

bool
CompareXmlNode(
          IXmlReader* pReader,
    const XmlNodeType NodeType,
    const wchar_t*    pNodeName = NULL,
          bool        bQuiet = false);

namespace XmlUtil
{

///////////////////////////////////////////////////////////////////////////////
//
// WriteElement_t
//
///////////////////////////////////////////////////////////////////////////////

class WriteElement_t
{

private:

    IXmlWriter*    m_pWriter;
    const wchar_t* m_pElementName;
    bool           m_bFullEndElement;

public:

    WriteElement_t(
        IXmlWriter*    pWriter,
        const wchar_t* pElementName,
        bool           bFullEndElement = true);
    
	~WriteElement_t()  noexcept(false); // NOTE: noexcept(false) added blindly for C++11 compatability

    void
    WriteValue(
        size_t Value);

    void
    WriteAttribute(
        const wchar_t* pAttributeName,
              size_t   Value);

    void
    WriteAttribute(
        const wchar_t* pAttributeName,
        const wchar_t* Value);

private:

    WriteElement_t();
    WriteElement_t(const WriteElement_t&);
    WriteElement_t& operator=(const WriteElement_t&);

};

///////////////////////////////////////////////////////////////////////////////
//
// ReadElement_t
//
///////////////////////////////////////////////////////////////////////////////

class ReadElement_t
{

private:

    IXmlReader*    m_pReader;
    const wchar_t* m_pElementName;
    bool           m_bReadEnd;

public:

    ReadElement_t(
              IXmlReader* pReader,
        const wchar_t*    pElementName,
              bool        bReadEnd = true);
    
    ~ReadElement_t()  noexcept(false);  // NOTE: noexcept(false) added blindly for C++11 compatability

    void 
    ReadValue(
        size_t& Value);

    void
    ReadAttribute(
        const wchar_t* pAttributeName,
              size_t&  Value);

    void
    ReadAttribute(
        const wchar_t* pAttributeName,
        std::wstring&  Value);

private:

    void
    ReadAttribute(
        const wchar_t*  pAttributeName,
        const wchar_t*& pValue);

private:

    ReadElement_t();
    ReadElement_t(const ReadElement_t&);
    ReadElement_t& operator=(const ReadElement_t&);

};

} // XmlUtil

///////////////////////////////////////////////////////////////////////////////

#endif // Include_XMLUTIL_H

///////////////////////////////////////////////////////////////////////////////
