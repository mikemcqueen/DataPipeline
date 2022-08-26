////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DcrTrades_t.cpp
//
// This class consists of:
//   static Charset_t * 2 (Lucida Sans Unicode 8)
//   Common "Find top line" function
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DcrTrades_t.h"
#include "Charset_t.h"
#include "DdUtil.h"
#include "Log.h"

/////////////////////////////////////////////////////////////////////////////

extern CDisplay *g_pDisplay;

/*static*/ CSurface* DcrTrades_t::s_rgpCornerSurfaces[BitmapCount];
/*static*/ CSurface* DcrTrades_t::s_rgpSideSurfaces[BitmapCount];

/////////////////////////////////////////////////////////////////////////////
ScreenTable_t bogus;

DcrTrades_t::
DcrTrades_t(
    TextTable_i*  pText,
    const size_t* pPixelColumnWidths,
    size_t        ColumnCount,
    size_t        LineHeight,
    size_t        CharHeight,
    size_t        CharOffsetY)
:
    DcrTable_t(
        pText, 
		bogus)
{
// NOTE commented out these params to base constructor, and added bogus param, to get it compiling
	pPixelColumnWidths;
	ColumnCount;
	LineHeight;
	CharHeight;
	CharOffsetY;
}


/////////////////////////////////////////////////////////////////////////////

DcrTrades_t::
~DcrTrades_t()
{
}

////////////////////////////////////////////////////////////////////////////

bool
DcrTrades_t::
Initialize()
{
    if (!InitAllCharsets())
    {
        LogError(L"DcrTrades_t::InitAllCharsets() failed.");
        return false;
    }
    return true;
}

static const Charset_t* s_pCharset = NULL;
static const Charset_t* s_pBoldCharset = NULL;

/////////////////////////////////////////////////////////////////////////////

void
DcrTrades_t::
Shutdown()
{
    std::unique_ptr<const Charset_t> c(s_pCharset);
    std::unique_ptr<const Charset_t> bc(s_pBoldCharset);
}

/////////////////////////////////////////////////////////////////////////////

bool
DcrTrades_t::
InitAllCharsets()
{
    if (NULL == s_pCharset)
    {
        LOGFONT lf;
        InitLogFont(lf, 8);
        wcscpy_s(lf.lfFaceName, _countof(lf.lfFaceName), L"Lucida Sans Unicode");

        s_pCharset = InitCharset(lf);
        if (NULL == s_pCharset)
            return false;
#if 1
        s_pCharset->WriteBmp(L"DcrTrades_Normal");
#endif
    }
    AddCharset(s_pCharset);

    if (NULL == s_pBoldCharset)
    {
        LOGFONT lf;
        InitLogFont(lf, 8);
        wcscpy_s(lf.lfFaceName, _countof(lf.lfFaceName), L"Lucida Sans Unicode");
        lf.lfWeight = FW_BOLD;

        s_pBoldCharset = InitCharset(lf);
        if (NULL == s_pBoldCharset)
            return false;
#if 1
        s_pBoldCharset->WriteBmp(L"DcrTrades_Bold");
#endif
    }
    AddCharset(s_pBoldCharset);
    return true;
}

/////////////////////////////////////////////////////////////////////////////

const Charset_t*
DcrTrades_t::
InitCharset(
    const LOGFONT& LogFont)
{
    auto pCharset = std::make_unique<Charset_t>(
        LogFont,
        L"abcdefghijkmnopqrstuwvxyzABCDEFGHJKLMNOPQRSTUVWXYZ0123456789l:,'()_-."
    );
    
    if (!pCharset->IsValid())
    {
        LogError(L"DcrTrades_t: Charset invalid.");
        return false;
    }
    pCharset->SetCharFlags(L"afktyvzEKLPQRTVWXY(4", DCR_NO_RIGHT_SPACING);
    pCharset->SetCharFlags(L"jJ", DCR_LEFT_OVERLAP );
    return pCharset.release();
}

/////////////////////////////////////////////////////////////////////////////
#if 0
bool
LoadSurface( CSurface* pSurface, LPCTSTR pszFile, bool bLog )
{
	TCHAR szPath[MAX_PATH];
	wsprintf( szPath, _T("screens\\%s"), pszFile );
    HRESULT hr = g_pDisplay->CreateSurfaceFromBitmap( pSurface, szPath, 0, 0 );
	if( bLog )
	{
	if( SUCCEEDED(hr) )
		ShowLog( _T("Loaded: %s"), szPath );
	else
		ShowLog( _T("Failed: %s, hr=%08x"), szPath, hr );
	}
	return SUCCEEDED(hr);
}
#endif

/////////////////////////////////////////////////////////////////////////////

void
DcrTrades_t::
InitLogFont(
    LOGFONT& lf,
    size_t   PointSize)
{
    static int logPixY = 0;

    if (0 == logPixY)
    {
        HDC hDC = GetDC( NULL );
        logPixY = GetDeviceCaps( hDC, LOGPIXELSY );
        ReleaseDC( NULL, hDC );
    }

	lf.lfHeight			 = -MulDiv(int(PointSize), logPixY, 72 );;
	lf.lfWidth			 = 0; 
	lf.lfEscapement		 = 0;
	lf.lfOrientation	 = 0;
	lf.lfWeight			 = FW_NORMAL;
	lf.lfItalic			 = FALSE;
	lf.lfUnderline		 = FALSE;
	lf.lfStrikeOut		 = FALSE;
	lf.lfCharSet		 = DEFAULT_CHARSET; // ANSI_CHARSET
	lf.lfOutPrecision	 = OUT_TT_PRECIS;
	lf.lfClipPrecision	 = CLIP_DEFAULT_PRECIS;
	lf.lfQuality		 = DEFAULT_QUALITY;
	lf.lfPitchAndFamily	 = DEFAULT_PITCH | FF_DONTCARE;
	lf.lfFaceName[0]     = '\0';
}

/////////////////////////////////////////////////////////////////////////////

bool
DcrTrades_t::
PreTranslateSurface(
    CSurface* pSurface,
    Rect_t&     rcSurface)
{
    if (IsRectEmpty(&rcSurface))
        return false;

    if (!InitSurfaceRect(pSurface, rcSurface))
    {
        LogError(L"TrWindow_t::PreTranslate(): InitSurfaceRect() failed.");
        return false;
    }
    // TODO : take rect or just eliminate this with smarter transpixel checking
    pSurface->FixColor(rcSurface, 0, 0x00404040, 0);
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool
DcrTrades_t::
InitSurfaceRect(
    const CSurface* pSurface,
    RECT&           rcSurface)
{
    rcSurface.top = GetTopLine(pSurface, rcSurface);

#if 0
    static int num = 0;
    RECT rc = rcSurface;
    rc.bottom = rc.top + 30;
    wchar_t buf[256];
    wsprintf(buf, L"bmp\\TrWindow_top_line_%d.bmp", ++num);
    pSurface->WriteBMP(buf, rc);
#endif

    return !IsRectEmpty(&rcSurface);
}

/////////////////////////////////////////////////////////////////////////////

int
DcrTrades_t::
GetTopLine(
    const CSurface* pSurface,
    const RECT&     rcClient)
{
    enum State_e
    {
        Looking,
        FoundGap,
        ValidateBottom,
        Done
    } state = Looking;


    // TODO:
    // we have to limit the width of this compare due to the grid lines on 
    // tradebuilder.  gridlines on a table is probably pretty common though.
    // should ideally store a gridlinewidth & gridlineheight, and do a same-color
    // compare of each column for (column width - gridlinewidth).
/*
    RECT rc = rcClient;
    ++rc.left;
    rc.right = rc.left + int(GetColumnWidth(0)) - 2;
    rc.bottom = rc.top + 1;
*/

    // find blank-line separators at top and bottom
    int yTop = rcClient.top + 1;
    size_t yOffset = 1;

    RECT rcCopy = rcClient;

    // TODO: Make sense of this calculation. Should it not involve CharHeight?
    int yMax = min(pSurface->GetHeight() - GetRowHeight() + 1, GetRowHeight() + GetCharHeight());
    for (int y = 0; y < yMax; ++y)
    {
        if (y == 0) continue; // bug with rc.top -1 when rc.top == 0

        bool bGap = true;
        RECT rc = rcCopy;
        size_t ColumnStart = rc.left;
        for (size_t Column = 0; GetColumnCount() > Column; ++Column)
        {
            // Assign right before left.
            rc.right = rc.left + int(GetColumnWidth(Column)) - 1 * int(GetGridline());
            rc.left += (int)GetGridline();
            rc.bottom = rc.top + 1;
            if (!pSurface->CompareColor(rc, 0, COMPARE_F_SAMECOLOR))
            {
                bGap = false;
                break;
            }
            ColumnStart += GetColumnWidth(Column);
            rc.left = int(ColumnStart);
        }

        switch(state)
        {
        case Looking:
            if (bGap)
            {
                state = FoundGap;
            }
            break;

        case FoundGap:
            if (!bGap)
            {
                yTop = rc.top - 1;
                yOffset = GetCharHeight();
                state = ValidateBottom;
            }
            break;

        case ValidateBottom:
            if (!bGap)
            {
                yTop = rc.bottom;
            }
            return yTop;

        default:
            ASSERT(false);
            break;
        }
        OffsetRect(&rcCopy, 0, (int)yOffset);
    }
    return yTop;
}

/////////////////////////////////////////////////////////////////////////////
