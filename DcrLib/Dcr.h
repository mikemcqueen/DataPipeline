/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DCR.H
//
// Digital Character Recognition (a.k.a. ConvertBitmapToText)
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_DCR_H
#define Include_DCR_H

/////////////////////////////////////////////////////////////////////////////

// only compare pixels if the charset pixel is opaque
#define DCR_COMP_CHARSET_OPAQUE			0x00000001

// don't check for full-transparent "spacing" column to right of char
#define DCR_NO_RIGHT_SPACING			0x00000002

// pixel(s) on the rightmost edge of this char may overlap the next adjacent char
#define DCR_RIGHT_OVERLAP				0x00000004

// this char is adjacent to a previous overlap char
#define DCR_ADJACENT_RIGHT_OVERLAP		0x00000008

// pixels on the leftmost edge of this char may overlap the previous adjacent char
#define DCR_LEFT_OVERLAP				0x00000010

/////////////////////////////////////////////////////////////////////////////

#define DCR_GETTEXT_ALLOW_BAD           0x00010000

// determine surface opacity by comparing to a maximum transparency color
#define DCR_GETTEXT_MAX_TRANS_COLOR     0x00020000

/////////////////////////////////////////////////////////////////////////////

// Successful return value indicating that the next character to be scanned
// is adjacent to a DCR_RIGHT_OVERLAP character.
#define DCR_S_ADJACENT_RIGHT_OVERLAP	0x00000001

// Successful return value indicating that the next character to be scanned
// is believed to be a DCR_LEFT_OVERLAP character.
#define DCR_S_ADJACENT_LEFT_OVERLAP	    0x00000002

/////////////////////////////////////////////////////////////////////////////

class Rect_t;
class Charset_t;
class CSurface;
class TextTable_i;

class DCR
{
private:

    typedef std::vector<char>             LineData_t;
    typedef std::vector<const Charset_t*> CharsetVector_t;

private:

    static std::unique_ptr<tesseract::TessBaseAPI> tesseract_;

    int id_;
    bool useTesseract_;
    CharsetVector_t m_Charsets;

public:

    DCR(int id = 0);
    DCR(int id, bool useTesseract);
    virtual ~DCR();

    //
    // DCR virtual:
    //

    virtual
    bool
    Initialize()
    {
        return true;
    }

#if 0
    virtual bool
    PreTranslateSurface(
        CSurface* /*pSurface*/,
        Rect_t* /*pRect*/)
    {
        // TODO: Rect = pSurface->GetBltRect(); 
        return true;
    }
#endif

    int GetId() {
        return id_;
    }

    virtual bool
    TranslateSurface(
        CSurface* /*pSurface*/,
        const Rect_t& /*Rect*/)
    {
        return true;
    }

    void
    AddCharset(const Charset_t* pCharset) 
    {
        m_Charsets.push_back(pCharset);
    }

    const CharsetVector_t&
    GetCharsets() const
    {
        return m_Charsets;
    }

    HRESULT
    GetText(
        const CSurface* pSurface,
        const RECT*     pRect,
        LPTSTR          pszText,
        size_t          iMaxLen,
        const Charset_t* pCharSet,
        const CharsetVector_t& Charsets,
        DWORD            dwFlags = 0) const;

    HRESULT
    GetText(
        const CSurface* pSurface,
        const RECT*     pRect,
        LPTSTR          pszText,
        size_t          iMaxLen,
        const Charset_t* pCharSet,
        DWORD            dwFlags = 0) const;

    std::string
    TesseractGetText(
        const CSurface* pSurface,
        const Rect_t& rect) const;

    int
    ReadTable(
        const CSurface*  pSurface,
        const RECT&      rcTable,
        const int     RowHeight,
        const int        RowGapSize,
        const RECT*      pColumnRects,
        TextTable_i*     pText,
        const int     CharHeight,
        const Charset_t* pCharset) const;

    bool
    ReadTableRow(
        const CSurface*  pSurface,
        const RECT*      pRect,
        size_t           cColumns,
        const RECT*      prcColumns,
        const size_t*    pTextLengths,
        wchar_t*         pszText,
        size_t           iMaxLen,
        size_t           CharHeight, // could be part of Charset_t
        const Charset_t* pCharSet) const;

    bool
    UsingTesseract() {
        return useTesseract_;
    }

    auto Tesseract() const {
        return tesseract_.get();
    }

    static
    int
    InitTesseract(
        const char* dataPath,
        const char* languageCode)
    {
        if (tesseract_.get()) {
            throw new logic_error("Tesseract already initialized");
        }
        tesseract_ = std::make_unique<tesseract::TessBaseAPI>();
        return tesseract_->Init(dataPath, languageCode);
    }

    static
    void
    EndTesseract()
    {
        tesseract_->End();
    }

private:
    
    HRESULT
    ReadColumnLines(
        const CSurface*  pSurface,
        const RECT&      Rect,
              wchar_t*   pText,
              size_t     pTextLength,
              size_t     CharHeight,
              size_t     LineGapSize,
        const Charset_t* pCharset) const;

    auto
    GetLineCount(
        const CSurface* pSurface,
        const RECT&     Rect,
              size_t    CharHeight,
              size_t    LineGapSize) const;

    void
    InitLineData(
        const CSurface*   pSurface,
        const RECT&       Rect,
              LineData_t& LineData) const;

    void WriteBadBmp(const CSurface* pSurface, const RECT& rc, const wchar_t* pszText) const;
    wchar_t* CreateSpacedText(const wchar_t* pszText ) const;
//	HRESULT  CalcTextRect(const wchar_t* pszText, HFONT hFont, RECT& rc) const;
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_DCR_H_

/////////////////////////////////////////////////////////////////////////////
