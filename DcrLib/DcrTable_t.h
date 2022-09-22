////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DcrTable_t.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_DCRTABLE_T_H_
#define Include_DCRTABLE_T_H_

/////////////////////////////////////////////////////////////////////////////

#include "Dcr.h"
#include "ScreenTable_t.h"

/////////////////////////////////////////////////////////////////////////////

class Rect_t;
class Charset_t;
class TextTable_i;

class DcrTable_t :
    public DCR
{
    const ScreenTable_t screenTable_;
    TextTable_i* pTextTable_;
    std::vector<RECT> columnRects_;
    std::vector<std::unique_ptr<CSurface>> columnSurfaces_;

public:

    DcrTable_t(
        int id,
        TextTable_i* pText,
        const TableParams_t& tableParams,
        std::span<const int> columnWidths,
        std::span<const RECT> textRects);

    ~DcrTable_t() override = default;

    DcrTable_t() = delete;
    DcrTable_t(const DcrTable_t&) = delete;
    DcrTable_t& operator=(const DcrTable_t&) = delete;

    //
    // DCR virtual:
    //

    bool
    Initialize() override;

    bool
    TranslateSurface(
        CSurface* pSurface,
        const Rect_t&   rect) override;

    //

    void
    SetTextTable(
        TextTable_i* pTextTable)
    {
        pTextTable_ = pTextTable;
    }

    const TextTable_i* GetTextTable() const { return pTextTable_; }
    const ScreenTable_t& GetScreenTable() const { return screenTable_; }

    //auto GetRowHeight() const { return GetScreenTable().RowHeight; }
    //auto GetRowGapSize() const { return GetScreenTable().RowGapSize; }
    //auto GetCharHeight() const { return GetScreenTable().CharHeight; }
    //auto GetColumnCount() const { return GetScreenTable().ColumnCount; }

private:

    // move to screentable_t
    //int
    //GetTotalColumnWidths() const;

    std::vector<RECT>
    InitColumnRects(
        const ScreenTable_t& screenTable) const;

    std::vector<unique_ptr<CSurface>>
    InitColumnSurfaces(
        const std::vector<RECT>& columnRects) const;
        
    int
    ReadTable(
        const CSurface* pSurface,
        const Rect_t& rcTable,
        TextTable_i* pText,
        const Charset_t* pCharset);

    int
    TesseractReadTable(
        const CSurface* pSurface,
        const RECT& rcTable,
        const int RowHeight,
        const int RowGapSize,
        const std::vector<RECT>& columnRects,
        const std::vector<std::unique_ptr<CSurface>>& columnSurfaces,
        TextTable_i* pText) const;
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_DCRTABLE_T_H

/////////////////////////////////////////////////////////////////////////////

