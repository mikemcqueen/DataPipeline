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
              TextTable_i*  pText,
        const ScreenTable_t& ScreenTable);

	virtual
    ~DcrTable_t();

    virtual bool
    TranslateSurface(
        CSurface* pSurface,
        Rect_t&   rcSurface) override;

    //
    // Helpers.
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

    size_t GetRowHeight() const   { return GetScreenTable().RowHeight; }
    size_t GetCharHeight() const  { return GetScreenTable().CharHeight; }

    size_t
    GetColumnCount() const
    {
        return GetScreenTable().ColumnCount;
    }

    size_t
    GetColumnWidth(
        size_t Column) const;

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
        const size_t* pPixelColumnWidths,
              size_t  ColumnCount);

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

    size_t
    GetTotalColumnWidths() const;

    void
    Shutdown();

    size_t
    ReadTable(
        const CSurface*    pSurface,
        const Rect_t&        rcTable,
              TextTable_i* pText,
        const Charset_t*   pCharset);

private:

    // Explicitly disabled:
    DcrTable_t();
    DcrTable_t(const DcrTable_t&);
    DcrTable_t& operator=(const DcrTable_t&);
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_DCRTABLE_T_H

/////////////////////////////////////////////////////////////////////////////

