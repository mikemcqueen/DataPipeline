/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// Dcr.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Dcr.h"
#include "Charset_t.h"
#include "DdUtil.h"
#include "Log.h"
#include "TextTable_t.h"
#include "Macros.h"
#include "Rect.h"

#undef DCR // ??

bool g_bWriteBmps = true;
bool g_bTableFixColor = true;

/*static*/
std::unique_ptr<tesseract::TessBaseAPI> DCR::tesseract_;

/////////////////////////////////////////////////////////////////////////////

DCR::
DCR(int id, bool useTesseract) :
    id_(id),
    useTesseract_(useTesseract)
{ }

/////////////////////////////////////////////////////////////////////////////

DCR::DCR(int id) : DCR(id, true)
{ }

/////////////////////////////////////////////////////////////////////////////

DCR::~DCR() { }
	
/////////////////////////////////////////////////////////////////////////////

HRESULT
DCR::
GetText(
    const CSurface*  pSurface,
    const RECT*      pRect,
	wchar_t*         pszText,
	size_t           iMaxLen,
    const Charset_t* pCharset,
    const CharsetVector_t& Charsets,
          DWORD            flags) const
{
    ASSERT(nullptr != pSurface);
    ASSERT(nullptr != pRect);
    ASSERT(nullptr != pszText);
	ASSERT(2 <= iMaxLen);

    if (nullptr != pCharset) {
        return GetText(pSurface, pRect, pszText, iMaxLen, pCharset, flags & DCR_GETTEXT_ALLOW_BAD);
    }
    auto it = Charsets.begin();
    for (; Charsets.end() != it; ++it) {
        HRESULT hr = GetText(pSurface, pRect, pszText, iMaxLen, *it, flags);
        if (SUCCEEDED(hr))
            return hr;
    }
    return E_FAIL;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
DCR::
GetText(
    const CSurface*  pSurface,
    const RECT*      pRect,
          wchar_t*   pszText,
          size_t     iMaxLen,
    const Charset_t* pCharset,
          DWORD      getTextFlags /*= 0*/) const
{
    ASSERT(nullptr != pSurface);
    ASSERT(nullptr != pRect);
    ASSERT(nullptr != pszText);
	ASSERT(2 <= iMaxLen);

	DWORD dwFlags = 0;
	int iSpaceWidth = pCharset->GetSpaceWidth();
	int x = pRect->left;
	int y = pRect->top;
	size_t iLen = 0;
    size_t cLeftPixels;
	int xEndGap = 0;
	int xStartGap = 0;
	unsigned iPrevChar = 0;
    bool bBadChar = false;
	
	iMaxLen--;

    HRESULT hr = S_OK;
	while(0 < (cLeftPixels = Charset_t::FindNextChar(pSurface, x, y, pRect, getTextFlags))) {
        dwFlags |= getTextFlags;
		xEndGap = x;
		unsigned iChar;
        hr = pCharset->Compare(cLeftPixels, pSurface, x, y, pRect, iChar, dwFlags);
		if (0 != (dwFlags & DCR_ADJACENT_RIGHT_OVERLAP)) {
			if (FAILED(hr)) {
				++x;
				y = pRect->top;
				continue;
			}
		}
        dwFlags = 0;
		wchar_t ch;
		if (SUCCEEDED(hr)) {
			ch = pCharset->GetChar(iChar);
			if (DCR_S_ADJACENT_RIGHT_OVERLAP == hr)
				dwFlags = DCR_ADJACENT_RIGHT_OVERLAP;
            else if (DCR_S_ADJACENT_LEFT_OVERLAP == hr)
				dwFlags = DCR_LEFT_OVERLAP;
		} else {
            bBadChar = true;
            if (0 == (getTextFlags & DCR_GETTEXT_ALLOW_BAD))
            {
                pszText[iLen] = 0;
                LogWarning(L"Dcr::GetText() Bad char after (%s) left (%d) x,y (%d,%d)",
                           pszText, cLeftPixels, x, y);
                break;
            }
            hr = S_OK;
            iChar = 0; // questionable
            ch = '?';
            Charset_t::SkipChar(pSurface, x, y, pRect);
        }
		if (0 < xStartGap) {
            // Calculate the width of the gap between the end of the last char,
            // and the start of this char.
			int iGap = xEndGap - xStartGap;
			iGap -= pCharset->GetKernAmount(iPrevChar, iChar);
            // TODO: kind hacky.  cLeading/trailing pixels should be used
            // instead of this flag existing at all
            if (0 != (pCharset->GetCharData(iChar).dwFlags & DCR_LEFT_OVERLAP))
                ++iGap;
            // TODO: seems right, test it with a RIGHTOVERLAP charset.
            if (0 != (pCharset->GetCharData(iPrevChar).dwFlags & DCR_RIGHT_OVERLAP))
                ++iGap;

            // Calculate the width of a space.
			int iSpace = iSpaceWidth;
			iSpace += pCharset->GetCharData(iPrevChar).ABCWidths.abcC;
			iSpace += pCharset->GetCharData(iChar).ABCWidths.abcA;
            // If the gap is as big as a space, add a space.
			if (iSpace <= iGap)
			{
				pszText[iLen++] = L' ';
				if (iMaxLen == iLen)
					break;
			}
		}
		pszText[iLen++] = ch;
		if (iMaxLen == iLen)
			break;
		xStartGap = x;
		iPrevChar = iChar;
	}
	ASSERT(iLen <= iMaxLen);
	pszText[iLen] = 0;
//    return ((0 < iLen) && (!bBadChar || bAllowBadChars)) ? iLen : 0;
    // Only return an error if there was an error translating a character
    // (pCharset->Compare() failed), and bAllowBadChars was false.
    // All other cases are success, specifically including "empty text".
    return hr;
}

/////////////////////////////////////////////////////////////////////////////

std::string
DCR::
TesseractGetText(
    const CSurface* pSurface,
    const Rect_t& rect) const
{
    std::string str;
    DDSURFACEDESC2 ddsd;
    HRESULT hr = pSurface->Lock(&ddsd);
    if (FAILED(hr)) {
        LogError(L"Can't lock surface");
        return str;
    }
    Tesseract()->SetImage((std::uint8_t*)GetBitsAt(&ddsd, rect.left, rect.top),
        rect.Width(), rect.Height(),
        4, (int)ddsd.lPitch);
    pSurface->Unlock(nullptr);
    std::unique_ptr<char> pResult(Tesseract()->GetUTF8Text());
    //auto text = pResult.get();
    if (auto text = pResult.get(); text && text[0]) {
        //auto ch = text[0];
        //if (text[0]) { // length > 0
            str.assign(text);
            str.erase(str.end() - 1);
        //}
    }
    return str;
}

/////////////////////////////////////////////////////////////////////////////

size_t g_iRow = 0;

bool
VerifyBlankRows(
    const CSurface* pSurface,
    const RECT&     rcRow,
    size_t          MaxCharHeight,
    bool            bAtBottom)
{
    bool bBlank;
    RECT rcBlank = rcRow;
    rcBlank.bottom = rcBlank.top + 1;
    bBlank = pSurface->CompareColor(rcBlank, 0, COMPARE_F_SAMECOLOR);
    if (!bBlank) {
        return false;
    }
    if (!bAtBottom) {
        OffsetRect(&rcBlank, 0, int(MaxCharHeight) - 1);
        bBlank = pSurface->CompareColor(rcBlank, 0, COMPARE_F_SAMECOLOR);
        if (!bBlank)
            return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////

int g_Bad = 0;

int
DCR::
ReadTable(
    const CSurface*  pSurface,
    const RECT&      rcTable,
    const int        RowHeight,
    const int        RowGapSize,
    const RECT*      pColumnRects,
    TextTable_i*     pText,
    const int        CharHeight,
    const Charset_t* pCharset) const
{
    ASSERT(nullptr != pSurface);
    ASSERT(nullptr != pColumnRects);
    ASSERT(nullptr != pText);
    ASSERT(0 < RowHeight);

    int MaxCharHeight = 0;
    for (auto col = 0; col < pText->GetColumnCount(); ++col) {
        if (pColumnRects[col].bottom > MaxCharHeight) {
            MaxCharHeight = pColumnRects[col].bottom;
        }
    }

    RECT rc = rcTable;

    // NOTE: could be a MinCharTop too
    rc.bottom = rc.top + MaxCharHeight;

g_Bad = 0;

    auto RowCount = pText->GetRowCount();
    auto iRow = 0;
    for (; iRow < RowCount; ++iRow) {
        // TODO: Hack. not needed? verifyblankRows doesn't compare bottom Row?  huh?
        bool bBottom = false;
        if (rc.bottom > rcTable.bottom) {
            bBottom = true;
            LogInfo(L"rc.bottom > rcTable.bottom: (rc.top=%d, rc.bottom=%d, pRect->bottom=%d, Row=%d",
                    rc.top, rc.bottom, rcTable.bottom, iRow);
            break;
        }
        // TODO: intersectrect.  or bounds checking.
        if (g_bWriteBmps) {
            WCHAR szFile[MAX_PATH];
            wsprintf( szFile, L"diag\\dcr_row_%d.bmp", iRow);
            pSurface->WriteBMP(szFile, rc);
        }
        wchar_t *pszRow = pText->GetRow(iRow);
        // HACKO: ?
#ifdef _DEBUG
        memset(pszRow, 0xae, sizeof(wchar_t) * pText->GetRowWidth());
#else
        memset(pszRow, 0, sizeof(wchar_t) * pText->GetRowWidth());
#endif
        // TODO: This "verification" function should be virtual, dependent
        //       on the specific source window we took the screenshot of.
        bool b = false;
        if (b && !VerifyBlankRows(pSurface, pColumnRects[0], MaxCharHeight, bBottom)) {
            // TODO: text.SetInvalidRow(iRow);
            auto Pos = 0;
            for (auto Column = 0; Column < pText->GetColumnCount(); ++Column) {
                size_t Width = pText->GetColumnWidths()[Column];
                wcscpy_s(&pszRow[Pos], Width, L"BAD");
                Pos += Width;
++g_Bad;
            }
            continue;
        }
g_iRow = iRow;

        bool validRow = false;
        validRow = ReadTableRow(
            pSurface,
            &rc,
            pText->GetColumnCount(),
            pColumnRects,
            pText->GetColumnWidths(),
            pszRow,
            pText->GetRowWidth(),
            CharHeight,
            pCharset);
        if (!validRow)
            break;

        //LogInfo(L"DCRRow%02: '%ls'", iRow, pszRow);
        OffsetRect(&rc, 0, RowHeight + RowGapSize);
    }
    pText->SetEndRow(iRow);

#if 1
    static size_t LastBad = 0;
    if (0 < g_Bad || 0 < LastBad) {
        static size_t badnum = 0;
        if (50 > ++badnum) {
            wchar_t buf[256];
            wsprintf(buf, L"diag\\bad_table_%d.bmp", badnum);
            pSurface->WriteBMP(buf, rcTable);
            LastBad = g_Bad;
        }
    }
#endif

    return iRow;
}

/////////////////////////////////////////////////////////////////////////////

bool
DCR::
ReadTableRow(
    const CSurface* pSurface,
    const RECT*     pRect,
    size_t          cColumns,
    const RECT*     prcColumns,
    const size_t*   pTextLengths,
	wchar_t*        pszText,
    size_t          iMaxLen,
    size_t          CharHeight,
    const Charset_t* pCharset) const
{
    ASSERT(nullptr != pSurface);
    ASSERT(nullptr != pRect);
    ASSERT(nullptr != prcColumns);
    ASSERT(nullptr != pTextLengths);
    ASSERT(nullptr != pszText);

    bool bFail = false;
    bool bAnyText = false;
    size_t totalTextLen = 0;
    for (size_t iColumn = 0; iColumn < cColumns; ++iColumn) {
        RECT rc;
        CopyRect(&rc, &prcColumns[iColumn]);
        OffsetRect(&rc, pRect->left, pRect->top);

        // TODO: intersectrect.  or bounds checking.
        if (g_bWriteBmps) {
            WCHAR szFile[MAX_PATH];
            wsprintf(szFile, L"Diag\\dcr_row_%d_column_%d.bmp", g_iRow, iColumn);
            pSurface->WriteBMP(szFile, rc);
            const_cast<CSurface*>(pSurface)->SlowRectangle(&rc, RGB(0, 0, 0));
        }

        size_t TextLen = pTextLengths[iColumn];
        if (0 == TextLen)
            continue;
        // 4 = vertical gapsize specific to Eq2Broker
        // TODO: factor out to ColumnData_t
        HRESULT hr = ReadColumnLines(pSurface, rc, pszText, TextLen, CharHeight, 4, pCharset);
        if (FAILED(hr)) {
//            bFail = true;
//            LogInfo(L"GetText(%d, %d, %d) failed.", g_iRow, iColumn, TextLen);
#if 1
            ++g_Bad;
            static size_t num = 0;
            if (50 > ++num) {
                wchar_t buf[256];
                wsprintf(buf, L"diag\\gettext_fail_%d.bmp", num);
                pSurface->WriteBMP(buf, rc);
            }
#endif
        }
        else if (L'\0' != pszText[0]) {
            bAnyText = true;
        }
        if (g_bWriteBmps && (nullptr != wcschr(pszText, L'?'))) {
            WriteBadBmp(pSurface, rc, pszText);
        }
        pszText += TextLen;
        totalTextLen += TextLen;
        if (totalTextLen > iMaxLen) {
            bFail = true;
            LogError(L"Column (%d) Total (%d) Max (%d)", iColumn, totalTextLen, iMaxLen);
            break;
        }
    }
    return !bFail && bAnyText;
}

/////////////////////////////////////////////////////////////////////////////
// Count the lines of text in a column (identified by Rect) of a table row.
auto
DCR::
GetLineCount(
    const CSurface* pSurface,
    const RECT& Rect,
    size_t    CharHeight,
    size_t    LineGapSize) const
{
    LineData_t LineData;
    InitLineData(pSurface, Rect, LineData);
    LineData_t::const_iterator itFirst = find(LineData.begin(), LineData.end(), 1);
    if (LineData.end() == itFirst)
        return 0;
    // TODO: totally insufficient for arbitrary # of lines.
    // we walk backwards from bottom of rect because individual "samecolor" lines
    // are possible in a block of text, e.g. "'.", the lines between apostrophe
    // and period don't delineate a "new line" even if the gapsize is sufficient.
    LineData_t::const_reverse_iterator ritFirst(itFirst);
    LineData_t::const_reverse_iterator ritLast = find(LineData.rbegin(), LineData.rend(), 1);
    size_t TotalHeight = ritFirst - ritLast;
    if (TotalHeight <= CharHeight)
        return 1;
    // ValidateGap();
    if (TotalHeight <= CharHeight * 2 + LineGapSize)
        return 2;
    return 3;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
DCR::
ReadColumnLines(
    const CSurface*  pSurface,
    const RECT&      Rect,
          wchar_t*   pText,
          size_t     TextLength,
          size_t     CharHeight,
          size_t     LineGapSize,
    const Charset_t* pCharset) const
{
    pText[0] = '\0';

    size_t LineCount = GetLineCount(pSurface, Rect, CharHeight, LineGapSize);
    // if linecount is zero (no lines identified), it is an error if text for
    // this column is not 'optional'; otherwise, it's not an error
    // TODO: support 'optional', until then, it's not an error
    if (0 == LineCount) {
        return S_OK;
    }
    // 2 line max specific to Eq2Broker
    // TODO: REFACTOR: probably part of "columndata"
    if (2 < LineCount) {
        return CO_E_PATHTOOLONG;
    }
    // HACK: not dealing with remainder lines correctly
    RECT TextRect = Rect;
    size_t LineHeight = RECTHEIGHT(TextRect) / LineCount;
    if (1 < LineCount) {
        TextRect.bottom = TextRect.top + LineHeight;
    }
    size_t RemainingLength = TextLength;
    wchar_t* pLine = pText;
    HRESULT hr = S_OK;
    for (size_t Line = 0; Line < LineCount;) {
        DWORD flags = g_bTableFixColor ? 0 : DCR_GETTEXT_MAX_TRANS_COLOR;
        hr = GetText(pSurface, &TextRect, pLine, int(RemainingLength), pCharset, m_Charsets, flags);
        if (FAILED(hr))
            break;
        if (++Line < LineCount) {
            ASSERT(pLine[0] != L'\0');
            size_t LineLength = wcslen(pLine) + 1;
            if (LineLength >= RemainingLength)
                throw std::logic_error("ReadColumnLines(): LineLength > RemainingLength");
            RemainingLength -= LineLength;
            lstrcat(pLine, L" ");
            pLine += LineLength;
            OffsetRect(&TextRect, 0, LineHeight);
        }
    }
    return hr;
}


/////////////////////////////////////////////////////////////////////////////

void
DCR::
InitLineData(
    const CSurface*   pSurface,
    const RECT&       Rect,
          LineData_t& LineData) const
{
    const COLORREF kMaxTransColor = RGB(50,50,50);
    const COLORREF Black = RGB(0,0,0);
    size_t LineCount = RECTHEIGHT(Rect);
    LineData.resize(LineCount);
    for (size_t Line = 0; Line < LineCount; ++Line) {
        RECT LineRect = Rect;
        LineRect.bottom = LineRect.top + 1;
        OffsetRect(&LineRect, 0, Line);
        if (pSurface->CompareColorRange(LineRect, Black, kMaxTransColor))
            LineData[Line] = 0;
        else
            LineData[Line] = 1;
    }
}

/////////////////////////////////////////////////////////////////////////////

void
DCR::
WriteBadBmp(
    const CSurface* pSurface,
    const RECT& rc,
    const wchar_t* pszText) const
{
    WCHAR szFile2[MAX_PATH];
    wsprintf(szFile2, L"Diag\\dcr_bad_%s.bmp", pszText);
    for (WCHAR* p = szFile2; L'\0' != *p; ++p) {
        if (nullptr != wcschr(L",'?", *p)) {
            *p = L'_';
        }
    }
    pSurface->WriteBMP(szFile2, rc);
}
