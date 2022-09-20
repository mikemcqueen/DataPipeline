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

DcrTable_t::
DcrTable_t(
    TextTable_i* pTextTable,
    const TableParams_t& tableParams,
    std::span<const int> columnWidths,
    std::span<const RECT> textRects)
    :
    pTextTable_(pTextTable),
    screenTable_(tableParams, columnWidths, textRects)
{ }	

/////////////////////////////////////////////////////////////////////////////

bool
DcrTable_t::
Initialize()
{
    // TODO: excpetions probably more consistent here
    columnRects_ = InitColumnRects(screenTable_);
    if (columnRects_.empty()) {
        LogError(L"Empty columnRects");
        return false;
    }
    columnSurfaces_ = InitColumnSurfaces(columnRects_);
    if (columnSurfaces_.size() != columnRects_.size()) {
        LogError(L"Column surface count mismatch");
        return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////

std::vector<RECT>
DcrTable_t::
InitColumnRects(
    const ScreenTable_t& screenTable) const
{
    std::vector<RECT> columnRects(screenTable.GetColumnCount());
    for (auto x =0, column = 0; column < screenTable.GetColumnCount(); ++column)
    {
        auto width = screenTable.GetColumnWidth(column);
        if (0 == width) {
            throw std::invalid_argument("DcrTable_t::ReadTable() Zero pixel column width not allowed");
        }
        auto& rc = screenTable.GetTextRect(column);
        if (!::IsRectEmpty(&rc)) {
            columnRects[column] = rc; // copy
            ::OffsetRect(&columnRects[column], x, 0); // offset copy
        } else {
            ::SetRect(&columnRects[column], x, 0, x + width, screenTable.GetRowHeight());
        }
        x += width;
    }
    return columnRects;
}

////////////////////////////////////////////////////////////////////////////

std::vector<unique_ptr<CSurface>>
DcrTable_t::
InitColumnSurfaces(
    const std::vector<RECT>& columnRects) const
{
    extern CDisplay* g_pDisplay;
    std::vector<unique_ptr<CSurface>> surfaces;
    for (auto& rc : columnRects) {
        std::unique_ptr<CSurface> pSurface = std::make_unique<CSurface>();
        HRESULT hr = g_pDisplay->CreateSurface(pSurface.get(), RECTWIDTH(rc), RECTHEIGHT(rc));
        if (FAILED(hr)) {
            throw new runtime_error(std::format("CreateSurface failed {}x{}",
                RECTWIDTH(rc), RECTHEIGHT(rc)));
        }
        surfaces.push_back(std::move(pSurface));
    }
    return surfaces;
}

/////////////////////////////////////////////////////////////////////////////

int
DcrTable_t::
ReadTable(
    const CSurface* pSurface,
    const Rect_t& rcTable,
    TextTable_i* pText,
    const Charset_t* pCharset)
{
    std::vector<RECT> copyColumnRects{ columnRects_ };
    for (auto& rc : copyColumnRects) {
        ::OffsetRect(&rc, rcTable.left, rcTable.top);
    }
    if (!useTesseract()) {
        return DCR::ReadTable(
            pSurface,
            rcTable,
            screenTable_.GetRowHeight(),
            screenTable_.GetRowGapSize(),
            &copyColumnRects[0],
            pText,
            screenTable_.GetCharHeight(),
            pCharset);
    }
    return TesseractReadTable(
        pSurface,
        rcTable,
        screenTable_.GetRowHeight(),
        screenTable_.GetRowGapSize(),
        copyColumnRects,
        columnSurfaces_,
        pText);
}


/////////////////////////////////////////////////////////////////////////////

int
DcrTable_t::
TesseractReadTable(
    const CSurface* pSurface,
    const RECT& rcTable,
    const int rowHeight,
    const int rowGapSize,
    const std::vector<RECT>& columnRects,
    const std::vector<std::unique_ptr<CSurface>>& columnSurfaces,
    TextTable_i* pText) const
{
    RECT rcRow{ rcTable }; // copy
    rcRow.bottom = rcRow.top + rowHeight;

    auto row = 0;
    for (; row < (int)pText->GetRowCount(); ++row)
    {
        if (rcRow.bottom > rcTable.bottom)
        {
            LogWarning(L"bottom > rcTable.bottom: (rc.top=%d, rc.bottom=%d, pRect->bottom=%d, Row=%d",
                rcRow.top, rcRow.bottom, rcTable.bottom, row);
            break;
        }
#if 0
        // TODO: intersectrect.  or bounds checking.
        if (g_bWriteBmps) {
            WCHAR szFile[MAX_PATH];
            wsprintf(szFile, L"diag\\dcr_row_%d.bmp", row);
            pSurface->WriteBMP(szFile, rc);
        }
#endif
        wchar_t* pszRow = pText->GetRow(row);
        // TODO: This "verification" function should be virtual, dependent
        //       on the specific source window we took the screenshot of.
#if 0
        bool b = false;
        if (b && !VerifyBlankRows(pSurface, pColumnRects[0], MaxCharHeight, bBottom))
        {
            // TODO: text.SetInvalidRow(row);
            size_t Pos = 0;
            for (size_t Column = 0; Column < pText->GetColumnCount(); ++Column)
            {
                size_t Width = pText->GetColumnWidth(Column);
                wcscpy_s(&pszRow[Pos], Width, L"BAD");
                Pos += Width;
            }
            continue;
        }
#endif
        pszRow;
        pSurface;
        for (size_t column = 0; column < columnRects.size(); ++column) {
            auto& rc = columnRects[column];
            columnSurfaces[column]->Blt(0, 0, pSurface, &rc);
            columnSurfaces[column]->WriteBMP(std::format(L"diag\\table_row_{}_col_{}.bmp",
                row, column).c_str(), rc);
        }
        ::OffsetRect(&rcRow, 0, rowHeight + rowGapSize);
    }
    pText->SetEndRow(row);
    return row;
}

/////////////////////////////////////////////////////////////////////////////

bool
DcrTable_t::
TranslateSurface(
    CSurface* pSurface,
    Rect_t&   rcSurface)
{
    LogInfo(L"DcrTable_t::TranslateSurface");
    auto rowCount = ReadTable(pSurface, rcSurface, pTextTable_, nullptr);
    if (0 == rowCount) {
        LogInfo(L"ReadTable(): Table is empty.");
    }
#if 1
    static bool bFirst = true;
    if (bFirst ) {
        pSurface->WriteBMP(L"Diag\\DcrTable_t.bmp");
        bFirst = false;
    }
#endif
    return true;
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

