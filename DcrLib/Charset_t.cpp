/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-209 Mike McQueen.  All rights reserved.
//
// Charset_t.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Charset_t.h"
#include "LegacyDcrImpl_t.h"
#include "DdUtil.h"
#include "Macros.h"
#include "Log.h"
#include <intrin.h>
#include <algorithm>
#include "commctrl.h"

extern CDisplay * g_pDisplay;

/////////////////////////////////////////////////////////////////////////////

constexpr COLORREF DefaultBkColor = RGB(19, 11, 12);
constexpr COLORREF DefaultTextColor = RGB(255, 48, 48);
constexpr COLORREF DefaultShadowColor = RGB(0, 0, 0);
constexpr COLORREF kMaxTransColor = RGB(50, 50, 50);

constexpr int ASCII_SPACE  = 32;
constexpr int CHAR_SPACING = 2;

/////////////////////////////////////////////////////////////////////////////

Charset_t::
Charset_t(
    const LOGFONT& lf,
    const wchar_t* pCharset,
    unsigned flags /* = 0 */)
{
	HFONT hFont = CreateFontIndirect(&lf);
	if (!hFont)
    {
        wprintf(L"Charset_t::Charset(): CreateFontIndirect failed.");
        throw EVENT_E_COMPLUS_NOT_INSTALLED;
    }
    Init(hFont, pCharset, flags);
}

/////////////////////////////////////////////////////////////////////////////

Charset_t::
Charset_t(
    HFONT          hFont,
    const wchar_t* pCharset,
    unsigned flags /* = false */)
{
    Init(hFont, pCharset, flags);
}

/////////////////////////////////////////////////////////////////////////////

void
Charset_t::
Init(
    HFONT hFont,
    const wchar_t* pCharset,
    unsigned flags)
{
	assert(nullptr != hFont);
    assert(nullptr != pCharset);

    m_valid = false;
	m_iSpaceWidth = 0;

	int numChars = wcslen(pCharset);
	m_charset.resize(numChars);
	std::copy(pCharset, pCharset + numChars, m_charset.begin());
	m_vCharData.resize(numChars);
	m_charWidths.resize(numChars);

	SIZE size;
    bool shadow = 0 != (flags & (kDrawSimulatedShadowText | kDrawShadowText));
	InitFontInfo(hFont, shadow, size);

    m_pSurface = std::make_unique<CSurface>();
	HRESULT hr = g_pDisplay->CreateSurface(m_pSurface.get(), size.cx, size.cy );
	if( FAILED(hr) )
        return; // TODO: throw hr;
//	m_pSurface = pS;

    if (flags & kDrawShadowText) {
        DrawShadowTextToSurface(hFont);
    } else {
        if (shadow) {
            DrawTextToSurface(hFont, false, true);
        }
        DrawTextToSurface(hFont, shadow);
    }

	InitCharData();

	m_charWidths.clear();
    m_valid = true;
}

/////////////////////////////////////////////////////////////////////////////

void
Charset_t::
DrawShadowTextToSurface(
    HFONT hFont)
{
    HDC hDC = m_pSurface->GetDC();
    if (nullptr == hDC)
    {
        _putws(L"Charset_t::DrawTextToSurface(): hDC == nullptr");
        throw EVENT_E_ALL_SUBSCRIBERS_FAILED;
    }
    HFONT hOldFont = (HFONT)::SelectObject(hDC, hFont);
    COLORREF oldBkColor = ::SetBkColor(hDC, DefaultBkColor);
    RECT rc = { 0, 0, LONG(m_pSurface->GetWidth()), LONG(m_pSurface->GetHeight()) };
    ::DrawShadowText(hDC, &*m_charset.cbegin(), m_charset.size(), &rc, 0,
        DefaultTextColor, DefaultShadowColor, 0, 0);
    ::SetBkColor(hDC, oldBkColor); 
    ::SelectObject(hDC, hOldFont);
    m_pSurface->ReleaseDC(hDC);
}

/////////////////////////////////////////////////////////////////////////////

void
Charset_t::
WriteBmp(
    const wchar_t* pszFile,
    const RECT*    pRect /* = nullptr */) const
{
    ASSERT(nullptr != pszFile);

    WCHAR szFile[MAX_PATH];
    wsprintf(szFile, L"Diag\\Charset_%s.bmp", pszFile);

    RECT rc = { 0 };
    if (nullptr == pRect)
    {
        rc.right = m_pSurface->GetWidth();
        rc.bottom = m_pSurface->GetHeight();
        pRect = &rc;
    }
    m_pSurface->WriteBMP(szFile, *pRect);
}

/////////////////////////////////////////////////////////////////////////////

Charset_t::
~Charset_t()
{
}

/////////////////////////////////////////////////////////////////////////////

void
Charset_t::
InitFontInfo(HFONT hFont, bool shadow, SIZE& size)
{
	HDC hDC = ::GetDC( 0 );
	HFONT hOldFont = (HFONT)SelectObject( hDC, hFont );

	int totalWidth = 0;
    for (size_t index = 0; index < m_charset.size(); ++index) {
        auto ch = m_charset.at(index);
        ABC abc;
		GetCharABCWidths(hDC, ch, ch, &abc);
        m_vCharData[index].ABCWidths = abc;
		GetTextExtentPoint32(hDC, &ch, 1, &size);
		int width = size.cx;
        if (shadow) width += 1;
		width += CHAR_SPACING;
		m_charWidths[index] = width;
		totalWidth += width;
	}

    const wchar_t space = L' ';
#if 0 // test
    ABC abc;
    GetCharABCWidths(hDC, space, space, &abc);
#endif
	GetTextExtentPoint32(hDC, &space, 1, &size);
	m_iSpaceWidth = size.cx;

	GetKernPairs(hDC);

	TEXTMETRIC tm;
	GetTextMetrics(hDC, &tm);
	size.cx = totalWidth;
	size.cy = tm.tmHeight + (shadow ? 1 : 0);

	SelectObject(hDC, hOldFont);
	ReleaseDC(0, hDC);
}

/////////////////////////////////////////////////////////////////////////////

//#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

void
Charset_t::
GetKernPairs(const HDC hDC)
{
	std::size_t numPairs = (std::size_t)::GetKerningPairs(hDC, 0, nullptr);
    m_vKernPairs.resize(numPairs);
    //std::vector<KERNINGPAIR>::iterator iter = m_vKernPairs.begin();
    unique_ptr<KERNINGPAIR[]>
    /*auto*/ pkPairs(make_unique<KERNINGPAIR[]>(numPairs));
    if (numPairs > 0) {
        DWORD ret = ::GetKerningPairs(hDC, numPairs, pkPairs.get()); //  m_vKernPairs.data());
        if (0 == ret) {
            char msg[256];
            sprintf_s(msg, 256, "GetKerningPairs failed, gle = %d, numPairs = %d, hDC = %08x\n",
                GetLastError(), numPairs, (ULONG)hDC);
            wprintf(L"GetKerningPairs2 failed\n");
            throw std::runtime_error(msg); // "GetKerningPairs failed");
        }
    }
	for (size_t iChar = 0; iChar < m_charset.size(); ++iChar) {
		m_vCharData[iChar].iFirstKernPair = -1;
		for (size_t pair = 0; pair < numPairs; ++pair) {
			if (m_vKernPairs[pair].wFirst == (WORD)m_charset[iChar]) {
				m_vCharData[iChar].iFirstKernPair = pair;
				break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void
Charset_t::
DrawTextToSurface(
    HFONT hFont,
    bool transparent /* = false */,
    bool shadow /* = false */)
{
	HDC hDC = m_pSurface->GetDC();
	if (nullptr == hDC)
    {
        _putws(L"Charset_t::DrawTextToSurface(): hDC == nullptr");
		throw EVENT_E_ALL_SUBSCRIBERS_FAILED;
    }
	HFONT hOldFont = (HFONT)SelectObject( hDC, hFont );

    COLORREF oldBkColor = ::SetBkColor(hDC, DefaultBkColor);
	COLORREF oldTextColor = ::SetTextColor(hDC, shadow
        ? DefaultShadowColor : DefaultTextColor);

    int oldBkMode = 0;
    if (transparent) {
        oldBkMode = ::SetBkMode(hDC, TRANSPARENT);
    }

    auto x = shadow ? 1 : 0;
    auto y = shadow ? 1 : 0;
    BOOL bGood = ::ExtTextOut(hDC, x, y, 0, 0, &*m_charset.begin(),
        m_charset.size(), &*m_charWidths.begin());

	::SelectObject(hDC, hOldFont);
    ::SetBkColor(hDC, oldBkColor);
    ::SetTextColor(hDC, oldTextColor);
    if (0 != oldBkMode) {
        ::SetBkMode(hDC, oldBkMode);
    }
	m_pSurface->ReleaseDC(hDC);

	#if 0 && defined(_DEBUG)
    {
        static int iCharset = 0;
        WCHAR szFile[32];
        swprintf_s(szFile, L"Charset%d.bmp", iCharset++);
        RECT rc = { 0, 0, (LONG)m_pSurface->GetWidth(), (LONG)m_pSurface->GetHeight() };
		m_pSurface->WriteBMP( szFile, rc );
	}
	#endif

	if (!bGood)
    {
        _putws(L"Charset_t::DrawTextToSurface(): !bGood");
		throw EVENT_E_QUERYFIELD;
    }
}

/////////////////////////////////////////////////////////////////////////////

void
Charset_t::
InitCharData( void )
{
	int xPrev = 0;
	int x;
	int y;
    
    RECT rc = { 0 };
    rc.bottom  = m_pSurface->GetHeight();

    for (size_t iChar = 0; iChar < m_charset.size(); ++iChar)
    {
        x = xPrev;
        y = 0;

        // TODO: some way to specify charset flags early,
        // if flags & left_overlap, x--

        size_t cLeftPixels = FindNextChar(m_pSurface.get(), x, y);
        if (0 == cLeftPixels)
            break;
#if 0
        rc.left = x;
        rc.right = min((x + 20), int(m_pSurface->GetWidth()));
        wchar_t szFile[10] = { 0 };
        _itow_s(int(iChar), szFile, 10);
        WriteBmp(szFile, &rc);
#endif
		CharData_t &cd		= m_vCharData[iChar];
		cd.ptOffset.x		= x;
		cd.ptOffset.y		= y;
		cd.LeftPixelCount   = cLeftPixels;

		cd.cLeadingPixels	= char(x - xPrev);

        x += cd.ABCWidths.abcB;
        y = 0;
		cd.Width			= SkipChar(m_pSurface.get(), x, y) + 1;

        // CHAR_SPACING doesn't take into account shadow here
//		cd.cTrailingPixels	= char(m_charWidths[iChar] - cd.Width - cd.cLeadingPixels - CHAR_SPACING);

		cd.dwFlags			= 0;

		xPrev += m_charWidths[iChar];
	}
}

/////////////////////////////////////////////////////////////////////////////

int
Charset_t::
GetKernAmount(
    unsigned FirstChar,
    unsigned SecondChar) const
{
	size_t iPair = GetCharData(FirstChar).iFirstKernPair;
	for (; 0 < iPair && iPair < m_vKernPairs.size(); ++iPair)
	{
		if (FirstChar != m_vKernPairs[iPair].wFirst)
            break;
		if (SecondChar == m_vKernPairs[iPair].wSecond)
			return m_vKernPairs[iPair].iKernAmount;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////

int
Charset_t::
GetCharIndex(
    wchar_t ch)
{
    std::vector<wchar_t>::const_iterator it =
        std::find(m_charset.begin(), m_charset.end(), ch);
    if (m_charset.end() == it)
    {
        LogError(L"Can't find '%c'", ch);
        throw std::invalid_argument("Charset_t::GetCharIndex");
    }
    return it - m_charset.begin();
}

/////////////////////////////////////////////////////////////////////////////

const CharData_t&
Charset_t::
GetCharData(
    int index) const
{
	assert(index < int(m_vCharData.size()));
	return m_vCharData.at(index);
}

/////////////////////////////////////////////////////////////////////////////

wchar_t
Charset_t::
GetChar(
    int index) const
{
    assert((0 <= index) && (index < int(m_charset.size())));
	return m_charset.at(index);
}

/////////////////////////////////////////////////////////////////////////////

void
Charset_t::
SetCharFlags(
    wchar_t ch,
    DWORD   dwFlags )
{
/*
        it = std::find(m_charset.begin(), m_charset.end(), Char);
        if (m_charset.end() == it)
        {
            LogError(L"Can't find '%c'", Char);
            return false;
        }
        int iChar = (int)(it - m_charset.begin());
*/

	for (size_t index = 0; index < m_charset.size(); ++index) {
		if (m_charset[index] == ch)
		{
			m_vCharData[index].dwFlags |= dwFlags;
            return;
		}
	}
    LogError(L"SetCharFlags(%d): Char not in charset (%d)", ch);
    ASSERT(0);
}

/////////////////////////////////////////////////////////////////////////////

void
Charset_t::
SetCharFlags(
    const wchar_t* pszChars,
    DWORD          dwFlags)
{
	ASSERT(nullptr != pszChars);
    // TODO: this is retarded.
	const int cChars = wcslen(pszChars);
	for (int iChar=0; iChar<cChars; ++iChar )
	{
		SetCharFlags(pszChars[iChar], dwFlags);
	}
}

/////////////////////////////////////////////////////////////////////////////

void
Charset_t::
SetCharWidths(
    wchar_t    ch,
    const ABC& abc)
{
    m_vCharData.at(GetCharIndex(ch)).ABCWidths = abc;
}

/////////////////////////////////////////////////////////////////////////////

size_t
Charset_t::
FindNextChar(
    const CSurface* pSurface,
    int&            x,
    int&            y,
    const RECT*     pRect /*=0*/,
    DWORD           dwFlags)
{
	DDSURFACEDESC2 ddsd;
	HRESULT hr = pSurface->Lock( &ddsd );
	if( FAILED(hr) )
		return 0;

    const DDPIXELFORMAT& pf = pSurface->GetPixelFormat();

	RECT rc;
	if (!pRect)
	{
		SetRect( &rc, 0, 0, ddsd.dwWidth, ddsd.dwHeight );
		pRect = &rc;
	}
    const size_t BitsPerLine = ddsd.lPitch / 4; // ack! 32-bit color

	size_t cLeftPixels = 0;	// # of pixels in leftmost vertical column of a char
	int ySave = -1;
	DWORD dwTransPixel = (*(DWORD *)GetBitsAt(&ddsd, pRect->left, pRect->top)) & 0x00ffffff;
	const DWORD *pBits = (DWORD *)GetBitsAt(&ddsd, x, y);
	for (; x < pRect->right; ++x, ++pBits)
	{
 		for (; y < pRect->bottom; ++y)
		{
            bool bOpaque = false;
            DWORD pixel = (*pBits & 0x00ffffff);
            if (0 == (dwFlags & DCR_GETTEXT_MAX_TRANS_COLOR))
            {
                bOpaque = pixel != dwTransPixel;
            }
            else
            {
#if 0
                bOpaque = pSurface->CompareColorIntensity(kMaxTransColor, pixel);
#else
                bOpaque = !IsTransparentPixel(pf, pixel, kMaxTransColor);
#endif
            }
            if (bOpaque)
			{
				if (-1 == ySave)
					ySave = y;
				++cLeftPixels;
			}
			pBits += BitsPerLine;
		}
		if (0 < cLeftPixels)
		{
			y = ySave;
			break;
		}
		pBits -= BitsPerLine * RECTHEIGHT(*pRect);
		y = pRect->top;
	}
	pSurface->Unlock();
	return cLeftPixels;
}

/////////////////////////////////////////////////////////////////////////////

int
Charset_t::
SkipChar(
    const CSurface* pSurface,
    int&			x,
    int&			y,
    const RECT*		pRect /*=0*/)
{
	DDSURFACEDESC2 ddsd;
	HRESULT hr = pSurface->Lock(&ddsd);
	if (FAILED(hr))
		return 0;

	RECT rc;
	if (nullptr == pRect)
	{
		SetRect(&rc, 0, 0, ddsd.dwWidth, ddsd.dwHeight);
		pRect = &rc;
	}
    const size_t BitsPerLine = ddsd.lPitch / 4; // ack! 32-bit color

	DWORD dwTransPixel = (*(DWORD *)GetBitsAt(&ddsd, pRect->left, pRect->top)) & 0x00ffffff;
	const DWORD *pBits = (DWORD *)GetBitsAt(&ddsd, x, y);
    int iWidth = 0;
	for (; x<pRect->right; ++x, ++pBits, ++iWidth)
	{
		for (; y < pRect->bottom; ++y)
		{
			if( (*pBits & 0x00ffffff) != dwTransPixel )
				break;
			pBits += BitsPerLine;
		}
		if (y == pRect->bottom )
			break;
		pBits -= BitsPerLine * (y - pRect->top);
		y = pRect->top;
	}
	pSurface->Unlock();
	return iWidth;
}

/////////////////////////////////////////////////////////////////////////////

bool
Charset_t::
MatchOverlapPointY(
    const POINT& pt,
    int&         y) const
{
	std::vector<POINT>::iterator iter = m_vecOverlapPoints.begin();
	for( ; iter!=m_vecOverlapPoints.end(); ++iter )
	{
		if( pt.y == iter->y )
		{
			y = iter->x;
			return true;
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
Charset_t::
Compare(
    size_t          cLeftPixels,
    const CSurface* pSurface,
    int&			x,
    int&			y,
    const RECT*		pRect,
    unsigned&       iChar,
    DWORD			dwFlags /* =0 */) const
{
    size_t BestMatchCount = 0;
    int xBest = -1;
    int yBest = -1 ;
    unsigned BestChar = 0;
    HRESULT hrBest = E_UNEXPECTED;
	
    for (iChar = 0; iChar < m_charset.size(); ++iChar)
	{
		const CharData_t& cd = m_vCharData[iChar];
        if (0 != (dwFlags & DCR_ADJACENT_RIGHT_OVERLAP))
		{
			if (!MatchOverlapPointY(cd.ptOffset, y))
				continue;
		}
		else
        {
            // Left pixel count must match.
            if (cLeftPixels != cd.LeftPixelCount)
                continue;
            // If LEFT_OVERLAP flag is specified, only use characters with that flag.
            if ((0 != (dwFlags & DCR_LEFT_OVERLAP) && (0 == (cd.dwFlags & DCR_LEFT_OVERLAP))))
                continue;
        }
        size_t MatchCount = 0;
        int xSave = x;
        int ySave = y;
		HRESULT hr = CompareChar(pSurface, x, y, pRect, iChar, dwFlags, MatchCount);
		if (SUCCEEDED(hr))
        {
            if (MatchCount == BestMatchCount)
            {
                LogError(L"Two chars have match count %d", MatchCount);
            }
            else if (MatchCount > BestMatchCount)
            {
                BestMatchCount = MatchCount;
                xBest = x;
                yBest = y;
                BestChar = iChar;
                hrBest = hr;
            }
            x = xSave;
            y = ySave;
        }
	}
    if (BestMatchCount > 0)
    {
        x = xBest;
        y = yBest;
        iChar = BestChar;
        return hrBest;
    }
	return EVENT_E_INTERNALERROR;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
Charset_t::
CompareChar(
    const CSurface* pSurface,
          int&      x,
          int&      y,
    const RECT*     pRect,
          unsigned  iChar,
          DWORD     dwFlags,
          size_t&   MatchCount) const
{
	HRESULT hr;
	DDSURFACEDESC2 ddsdSurface;
	hr = pSurface->Lock(&ddsdSurface);
	if( FAILED(hr) )
		return hr;

	DDSURFACEDESC2 ddsdCharset;
	hr = m_pSurface->Lock(&ddsdCharset);
	if( FAILED(hr) )
	{
		pSurface->Unlock();
		return hr;
	}

    const DDPIXELFORMAT& pf = pSurface->GetPixelFormat();

    const size_t CharsetBitsPerLine = ddsdCharset.lPitch / 4; // ack!! 32 bit color
    const size_t SurfaceBitsPerLine = ddsdSurface.lPitch / 4; // ack!! 32 bit color
    const CharData_t& cd            = m_vCharData[iChar];
    size_t iCharsetWidth            = cd.ABCWidths.abcB; 
//    size_t iCharsetWidth            =  cd.iWidth;
    POINT ptCharset                 = cd.ptOffset;

    if (0 != (cd.dwFlags & DCR_LEFT_OVERLAP))
    {
        // We're about to compare to a character with the LEFT_OVERLAP flag.
        // If the LEFT_OVERLAP flag wasn't specified, try to match the "whole" character.
        if (0 == (dwFlags & DCR_LEFT_OVERLAP))
        {
            --ptCharset.x;
            ++iCharsetWidth;
        }
    }

    RECT rcCharset;
    SetRect(&rcCharset, 0, 0, ddsdCharset.dwWidth, ddsdCharset.dwHeight);

    const DWORD *pSurfaceBits = (DWORD *)GetBitsAt(&ddsdSurface, x, y);
    DWORD dwSurfaceTransPixel = (*(DWORD *)GetBitsAt(&ddsdSurface, pRect->left, pRect->top)) & 0x00ffffff;
    const DWORD *pCharsetBits = (DWORD *)GetBitsAt(&ddsdCharset, ptCharset.x, ptCharset.y);
    DWORD dwCharsetTransPixel = (*(DWORD *)GetBitsAt(&ddsdCharset, 0, 0)) & 0x00ffffff;

    int xSurfaceStart = x;
    int yCharsetStart = ptCharset.y;

    bool bEndOfChar = false;
    bool bMisMatch = false;
    bool bMatch = false;
    bool bAdjacentOverlap = false;
    bool bOverlap = false;
    size_t MismatchedSurfaceOpaquePixels = 0;
//    int yMismatchSurfaceOpaque = pRect->top - 1;

    MatchCount = 0;
    for (size_t iWidth = 1; x < pRect->right; ++x, ++ptCharset.x)
    {
        for (; ptCharset.y < rcCharset.bottom; ++ptCharset.y, pCharsetBits += CharsetBitsPerLine)
        {
            const bool bCharsetOpaque = (*pCharsetBits & 0x00ffffff) != dwCharsetTransPixel;
            const bool bTestCharsetOpaquePixelsOnly = (0 != (dwFlags & DCR_COMP_CHARSET_OPAQUE));

            if (!bCharsetOpaque && bTestCharsetOpaquePixelsOnly && !bOverlap)
                continue;

            const int yOffset = ptCharset.y - yCharsetStart;
            const int yPos    = y + yOffset;
            if (yPos < pRect->top || pRect->bottom < yPos)
            {
                // if y-position is out of bounds, and Charset pixel is opaque, it's a mismatch
                if( bCharsetOpaque )
                {
                    bMisMatch = true;
                    break;
                }
                // skip this Charset-transparent, pSurface-out-of-bounds pixel
                continue;
            }
            if (bCharsetOpaque && bOverlap)
            {
                ASSERT(iWidth == iCharsetWidth);
                POINT pt = { yPos, ptCharset.y };
                //pt.x = yPos;
                //pt.y = ptCharset.y;
                m_vecOverlapPoints.push_back(pt);
            }
            const DWORD dwSurfacePixel = *(pSurfaceBits + yOffset * SurfaceBitsPerLine) & 0x00ffffff;
            bool bSurfaceOpaque = false;
            if (0 == (dwFlags & DCR_GETTEXT_MAX_TRANS_COLOR))
            {
                bSurfaceOpaque = dwSurfacePixel != dwSurfaceTransPixel;
            }
            else
            {
#if 0
                bSurfaceOpaque = pSurface->CompareColorIntensityFunc(kMaxTransColor, dwSurfacePixel);
#else
                
                bSurfaceOpaque = !IsTransparentPixel(pf, dwSurfacePixel, kMaxTransColor);
#endif
            }
            if (bCharsetOpaque == bSurfaceOpaque)
            {
                ++MatchCount;
            }
            else
            {
                // State: surface & charset pixels don't match

                // If surface pixel is opaque 
                if (bSurfaceOpaque)
                {
                    if (bOverlap)
                    {
                        bAdjacentOverlap = true;
                        if (iWidth == iCharsetWidth)
                        {
                            POINT pt = { yPos, ptCharset.y };
                            //pt.x = yPos;
                            //pt.y = ptCharset.y;
                            m_vecOverlapPoints.push_back(pt);
                        }
                    }
                    //LogAlways(L"mismatch char='%c', width=%d, charsetwidth=%d",
                    //            m_charset[iChar], iWidth, iCharsetWidth);

                    // If we're on the last column, and the character we're matching has
                    // no right spacing, we can allow up to 1 mismatched surface-opaque
                    // pixel, as we may be next to a LEFT_OVERLAP character.
                    if ((iWidth == iCharsetWidth) &&
                        (0 != (cd.dwFlags & DCR_NO_RIGHT_SPACING)) &&
                        (1 == ++MismatchedSurfaceOpaquePixels))
                    {
                        continue;
                    }
                    // If we're past the last column, the character we're matching must
                    // have right spacing; we're doing the "right column is blank" check.
                    // We can allow up to 1 mismatched surface-opaque pixel in this
                    // column, as we may be next to a LEFT_OVERLAP character.
                    else if ((iWidth > iCharsetWidth) && (1 == ++MismatchedSurfaceOpaquePixels))
                        continue;
                }

                // If we're testing all pixels, or if we're testing only charset-opaque
                // pixels and the charset pixel is opaque, it's a mismatch.
                if (!bTestCharsetOpaquePixelsOnly || bCharsetOpaque)
                {
                    bMisMatch = true;
                    break;
                }
                ASSERT(2 > MismatchedSurfaceOpaquePixels);
            }
        }
        if (bMisMatch)
            break;
        if (iWidth > iCharsetWidth)
        {
            bEndOfChar = true;
            bMatch = true;
            break;
        }
        ++pSurfaceBits;
        ++pCharsetBits;
        pCharsetBits -= CharsetBitsPerLine * ptCharset.y;
        ptCharset.y = 0;

        ++iWidth;
// TODO: if (abcC < 0)
        if ((iWidth == iCharsetWidth) && (0 != (cd.dwFlags & DCR_RIGHT_OVERLAP)))
        {
            // BUG: woops!  what about two overlap chars in a row.
            // well, that'd be a problem, if there were (or ever are) two different
            // overlap chars.  as it currently stands, only the letter 'f' is, so,
            // its *always* going to be a match, negating the need to save the overlap
            // points for subsequent calls. i suppose what i really ought to do is
            // store overlap points in function (locally) and copy to class member
            // when we've determined its a successful match.
            m_vecOverlapPoints.clear();
            bOverlap = true;
            dwFlags |= DCR_COMP_CHARSET_OPAQUE;
        }
        else if ((iWidth > iCharsetWidth) && (0 == cd.ABCWidths.abcC))
        {
            if (!bOverlap || bAdjacentOverlap)
            {
                bMatch = true;
                break;
            }
        }
    }
    m_pSurface->Unlock();
    pSurface->Unlock();

    if (bMatch)
    {
        if (bAdjacentOverlap)
        {
            hr = DCR_S_ADJACENT_RIGHT_OVERLAP;
            if (bEndOfChar)
                --x;
        }
        else if (!bEndOfChar)
        {
            ASSERT(0 == (cd.dwFlags & DCR_RIGHT_OVERLAP));
            ++x;

            // If we found a mismatched surface-opaque pixel on last opaque column
            // of a NO_RIGHT_SPACING character, then we expect the next character
            // to be LEFT_OVERLAP.
            if (1 == MismatchedSurfaceOpaquePixels)
            {
                ASSERT(S_OK == hr);
                hr = DCR_S_ADJACENT_LEFT_OVERLAP;
            }
        }
        y = pRect->top;
        return hr;
    }
    x = xSurfaceStart;
    return EVENT_E_USER_EXCEPTION;
}

/////////////////////////////////////////////////////////////////////////////
