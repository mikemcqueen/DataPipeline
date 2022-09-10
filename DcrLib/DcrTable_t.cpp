/////////////////////////////////////////////////////////////////////////////
//
// DcrTable_t.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DcrTable_t.h"
#include "DdUtil.h"
#include "Log.h"
#include "Macros.h"
#include "Charset_t.h"
#include "TextTable_t.h"
#include "Rect.h"

/////////////////////////////////////////////////////////////////////////////
//
// DcrTable_t
//
/////////////////////////////////////////////////////////////////////////////

DcrTable_t::
DcrTable_t(
    TextTable_i* pText,
    const ScreenTable_t& ScreenTable)
:
    m_pText(pText),
    m_ScreenTable(ScreenTable),
    m_Gridline(0)
{
}	

/////////////////////////////////////////////////////////////////////////////

void
DcrTable_t::
Shutdown()
{
}

/////////////////////////////////////////////////////////////////////////////

void
DcrTable_t::
SetColumnWidths(
    const size_t* pPixelColumnWidths,
    size_t        ColumnCount)
{
#if 1
    pPixelColumnWidths; ColumnCount;
    throw logic_error("DcrTable_t::SetColumnWidths() not implemented");
#else
    m_ColumnWidths.reserve(ColumnCount);
    m_ColumnWidths.assign(pPixelColumnWidths, pPixelColumnWidths + ColumnCount);
#endif
}

/////////////////////////////////////////////////////////////////////////////

bool
DcrTable_t::
TranslateSurface(
    CSurface* pSurface,
    Rect_t&   rcSurface)
{
    LogInfo(L"DcrTable_t::TranslateSurface");
    size_t RowCount = ReadTable(pSurface, rcSurface, m_pText, nullptr);
    if (0 == RowCount)
    {
        LogInfo(L"  ReadTable(): Table is empty.");
    }
#if 1
    static bool bFirst = true;
    if (bFirst )
    {
        pSurface->WriteBMP(L"Diag\\DcrTable_t.bmp");
        bFirst = false;
    }
#endif
    return true;
}

////////////////////////////////////////////////////////////////////////////

size_t
DcrTable_t::
ReadTable(
    const CSurface*  pSurface,
    const Rect_t&      rcTable,
    TextTable_i*     pText,
    const Charset_t* pCharset)
{
    const size_t ColumnCount = GetScreenTable().ColumnCount;
    std::vector<RECT> ColumnRects;
    ColumnRects.resize(ColumnCount);
    size_t x = 0;
    bool bZeroColumnWidthUsed = false;
    for (size_t Column = 0; Column < ColumnCount; ++Column)
    {
        size_t Width = GetColumnWidth(Column);
        // Zero as final column width means "use the remaining rectangle width"
        if (0 == Width)
        {
            if (bZeroColumnWidthUsed)
            {
                throw std::invalid_argument("DcrTable_t::ReadTable() Only one zero pixel column width allowed");
            }
            bZeroColumnWidthUsed = true;
            Width = RECTWIDTH(rcTable) - GetTotalColumnWidths();
        }
        const RECT* pCustomRect = &GetScreenTable().pTextRects[Column];
        if (!IsRectEmpty(pCustomRect))
        {
            ColumnRects[Column] = *pCustomRect;
            OffsetRect(&ColumnRects[Column], x, 0);
            ASSERT(x + pCustomRect->left + RECTWIDTH(*pCustomRect) <= Width);
            ASSERT(pCustomRect->top + RECTHEIGHT(*pCustomRect) <= int(GetRowHeight()));
        }
        else
        {
            SetRect(
                &ColumnRects[Column],
                int(x + m_Gridline),
                0,
                int(x + Width - 2 * m_Gridline),
                int(GetRowHeight()));
        }
        x += Width;
    }
    ASSERT(RECTWIDTH(rcTable) >= int(x));
    if (RECTWIDTH(rcTable) < int(x))
        LogError(L"rcTable.Width (%d) < (%d)", RECTWIDTH(rcTable), x);

    const size_t LineCount =
        DCR::ReadTable(
            pSurface,
            rcTable,
            GetRowHeight(),
            &ColumnRects[0],
            pText,
            GetCharHeight(),
            pCharset);
    return LineCount;
}

/////////////////////////////////////////////////////////////////////////////

#if 0
void
DcrTable_t::
GetRect(
    const RECT&  rcBounds,
          size_t StartLine,
          size_t EndLine,
          RECT&  rc) const
{
    ASSERT(m_pText->GetRowCount() > StartLine);
    ASSERT(m_pText->GetRowCount() >= EndLine);
    ASSERT(EndLine > StartLine);
    rc = rcBounds;
    rc.top += int(StartLine * GetRowHeight());
    ASSERT(rc.top < rcBounds.bottom);
    const size_t LineCount = EndLine - StartLine;
#if OLD
    rc.bottom = rc.top + int((LineCount - 1) * GetRowHeight() +
        GetScreenTable().GetCharHeight());
    if (rc.bottom > rcBounds.bottom)
        rc.bottom = rcBounds.bottom;
#else
    rc.bottom = rc.top + int((LineCount - 1) * GetRowHeight());
    if (rc.bottom > rcBounds.bottom)
        throw std::logic_error("DcrTable_t::GetRect()");
#endif
}
#endif

/////////////////////////////////////////////////////////////////////////////

size_t
DcrTable_t::
GetColumnWidth(
    size_t Column) const
{
    if (GetScreenTable().ColumnCount <= Column)
    {
        throw std::invalid_argument("DcrTable_t::GetColumnWidth()");
    }
    return GetScreenTable().pColumnWidths[Column];
}

/////////////////////////////////////////////////////////////////////////////

size_t
DcrTable_t::
GetTotalColumnWidths() const
{
    return std::accumulate(&GetScreenTable().pColumnWidths[0],
                           &GetScreenTable().pColumnWidths[GetScreenTable().ColumnCount],
                           0);
}

/////////////////////////////////////////////////////////////////////////////
