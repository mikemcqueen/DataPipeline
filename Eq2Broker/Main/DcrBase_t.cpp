////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// DcrBase_t.cpp
//
// EQ2 broker DCR base class.
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DcrBase_t.h"
#include "Charset_t.h"
#include "DdUtil.h"
#include "Log.h"
#include "Macros.h"
#include "BrokerUi.h"

namespace Broker
{

/////////////////////////////////////////////////////////////////////////////

// last char: 
// right single quotation mark (U+2019) 

constexpr wchar_t CharsetChars[] = L"!\"#$%&'()*+,-./0123456789:;<=>?@"
                                   L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
                                   L"abcdefghijklmnopqrstuvwxyz{|}~’";

constexpr wchar_t CharsetFacename[] = L"Zapf Calligraphic 801 Bold BT";
constexpr int CharsetPointSize = 10;
constexpr unsigned CharsetFlags = kDrawShadowText; // kDrawSimulatedShadowText

//static const Charset_t* s_pBoldCharset = nullptr;

/////////////////////////////////////////////////////////////////////////////

DcrBase_t::
DcrBase_t(
          TextTable_i*   pText,
    const TableWindow_t& tableWindow,
    const ScreenTable_t& ScreenTable)
:
    DcrTable_t(pText, ScreenTable),
    m_tableWindow(tableWindow)
{
}

/////////////////////////////////////////////////////////////////////////////

DcrBase_t::
~DcrBase_t()
{
}

////////////////////////////////////////////////////////////////////////////

bool
DcrBase_t::
Initialize()
{
#if 0
    if (!InitAllCharsets())
    {
        LogError(L"DcrBase_t::InitAllCharsets() failed.");
        return false;
    }
#endif
    AddCharset(GetCharset());
    return true;
}

/////////////////////////////////////////////////////////////////////////////

/* static */
const Charset_t*
DcrBase_t::
GetCharset()
{
    static shared_ptr<const Charset_t> spCharset;
    if (!spCharset)
    {
        LOGFONT lf;
        InitLogFont(lf, CharsetPointSize, CharsetFacename);
        spCharset.reset(InitCharset(lf, CharsetFlags));
        if (!spCharset)
        {
        }
        spCharset->WriteBmp(L"DcrBase");
    }
    return spCharset.get();
}

/////////////////////////////////////////////////////////////////////////////

bool
DcrBase_t::
InitAllCharsets()
{
#if 1
    GetCharset();
#else
    if (nullptr == s_pCharset)
    {
        LOGFONT lf;
        InitLogFont(lf, CharsetPointSize, CharsetFacename);
        s_pCharset = InitCharset(lf);
        if (nullptr == s_pCharset)
            return false;
        s_pCharset->WriteBmp(L"DcrBase");
    }
#endif
#if 0
    if (nullptr == s_pBoldCharset)
    {
        LOGFONT lf;
        InitLogFont(lf, 8, L"Lucida Sans Unicode"););
        wcscpy_s(lf.lfFaceName, _countof(lf.lfFaceName), 
        lf.lfWeight = FW_BOLD;

        s_pBoldCharset = InitCharset(lf);
        if (nullptr == s_pBoldCharset)
            return false;
        s_pBoldCharset->WriteBmp(L"DcrBase_Bold");
    }
    AddCharset(s_pBoldCharset);
#endif
    return true;
}

/////////////////////////////////////////////////////////////////////////////

/* static */
const Charset_t*
DcrBase_t::
InitCharset(
    const LOGFONT& LogFont,
    unsigned flags)
{
    Charset_t* pCharset = new Charset_t(LogFont, CharsetChars, flags);
    if (!pCharset->IsValid())
    {
        LogError(L"DcrBase_t: Charset invalid.");
        delete pCharset;
        return nullptr;
    }
    pCharset->SetCharFlags(L"abcdfhikmnpqtuwxzACGKOQTUVWX27':\",./[\\", DCR_NO_RIGHT_SPACING);
    ABC abc_s = { 0, 4, 0 };
    ABC abc_S = { 1, 6, 0 };
    pCharset->SetCharWidths(L's', abc_s);
    pCharset->SetCharWidths(L'S', abc_S);
    return pCharset;
}

/////////////////////////////////////////////////////////////////////////////

void
DcrBase_t::
InitLogFont(
          LOGFONT& lf,
          int      PointSize,
    const wchar_t* pszFaceName)
{
    static int logPixY = 0;
    if (0 == logPixY)
    {
        HDC hDC = GetDC(nullptr);
        logPixY = GetDeviceCaps(hDC, LOGPIXELSY);
        ReleaseDC(nullptr, hDC);
    }

    lf.lfHeight          = -MulDiv(int(PointSize), logPixY, 72);
	lf.lfWidth			 = 0; 
	lf.lfEscapement		 = 0;
	lf.lfOrientation	 = 0;
    lf.lfWeight =          FW_NORMAL;
	lf.lfItalic			 = FALSE;
	lf.lfUnderline		 = FALSE;
	lf.lfStrikeOut		 = FALSE;
	lf.lfCharSet		 = DEFAULT_CHARSET;
	lf.lfOutPrecision	 = OUT_TT_PRECIS;
	lf.lfClipPrecision	 = CLIP_DEFAULT_PRECIS;
    lf.lfQuality =         DEFAULT_QUALITY;
	lf.lfPitchAndFamily	 = DEFAULT_PITCH | FF_DONTCARE;
    wcscpy_s(lf.lfFaceName, pszFaceName);
}

////////////////////////////////////////////////////////////////////////////////

bool
DcrBase_t::
PreTranslateSurface(
    CSurface* pSurface,
    Rect_t&   rcSurface)
{
    extern bool g_bTableFixColor;

    Rect_t rect = m_tableWindow.GetClientRect();
    if (!IsRectEmpty(&rect))
    {
        rect.top += GetScreenTable().OffsetY;
        m_selectedRow = GetSelectedRow(*pSurface, rect);
        if (g_bTableFixColor)
        {
            // TODO: pSurface->ReplaceColorRange
            pSurface->FixColor(rect, BkLowColor, BkHighColor, Black);
        }
        rcSurface = rect;
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

size_t
DcrBase_t::
GetSelectedRow(
    CSurface& surface,
    const Rect_t&   tableRect) const
{
    const int rowHeight = GetScreenTable().RowHeight;
    const size_t width = 4;
    const size_t height = 1;
    Rect_t selectRect;
    selectRect.left = tableRect.left + tableRect.Width() / 2 - width / 2;
    selectRect.right = selectRect.left + width;
    selectRect.top = tableRect.top;
    selectRect.bottom = selectRect.top + height;
    size_t selectedRow = 0;
    for (size_t row = 1; selectRect.top + rowHeight <= tableRect.bottom; ++row)
    {
        using namespace Broker::Table;
        if (surface.CompareColorRange(selectRect, SelectedLowColor, SelectedHighColor))
        {
            if (0 < selectedRow)
            {
                LogError(L"DcrBase_t::GetSelectedRow() Two rows selected (%d,%d)",
                         selectedRow, row);
                return 0;
            }
            selectedRow = row;
        }
        ::OffsetRect(&selectRect, 0, rowHeight);
    }
    return selectedRow;
}

////////////////////////////////////////////////////////////////////////////////

} // Broker

/////////////////////////////////////////////////////////////////////////////
