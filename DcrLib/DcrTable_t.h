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

private:

    const ScreenTable_t& m_ScreenTable;
    TextTable_i*         m_pText;
    size_t               m_Gridline;

public:

    DcrTable_t(
              TextTable_i* pText,
        const ScreenTable_t& ScreenTable);

    ~DcrTable_t() override = default;

    //
    // DCR virtual:
    //

    bool
    TranslateSurface(
        CSurface* pSurface,
        Rect_t&   rcSurface) override;

    //

    void
    SetTextTable(
        TextTable_i* pTextTable)
    {
        m_pText = pTextTable;
    }

    const TextTable_i*
    GetTextTable() const { return m_pText; }

    const ScreenTable_t& GetScreenTable() const { return m_ScreenTable; }

    auto GetRowHeight() const { return GetScreenTable().RowHeight; }
    auto GetRowGapSize() const { return GetScreenTable().RowGapSize; }
    auto GetCharHeight() const { return GetScreenTable().CharHeight; }
    auto GetColumnCount() const { return GetScreenTable().ColumnCount; }

    auto
    GetColumnWidth(
        int Column) const;

#if 0
    void
    GetRect(
        const RECT&  rcBounds,
              size_t StartLine,
              size_t EndLine,
              RECT&  rc) const;
#endif

    void
    SetColumnWidths(
        const int* pPixelColumnWidths,
              int  ColumnCount);

    void
    SetGridline(
        size_t Size)
    {
        m_Gridline = Size;
    }

    size_t
    GetGridline() const
    {
        return m_Gridline;
    }

private:

    int
    GetTotalColumnWidths() const;

    void
    Shutdown();

    int
    ReadTable(
        const CSurface* pSurface,
        const Rect_t& rcTable,
        TextTable_i* pText,
        const Charset_t* pCharset);

private:

    DcrTable_t() = delete;
    DcrTable_t(const DcrTable_t&) = delete;
    DcrTable_t& operator=(const DcrTable_t&) = delete;
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_DCRTABLE_T_H

/////////////////////////////////////////////////////////////////////////////

