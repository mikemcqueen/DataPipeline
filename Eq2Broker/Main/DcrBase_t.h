////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// DcrBase_t.h
//
// EQ2 Broker window DCR base class.
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_DCRBASE_T_H
#define Include_DCRBASE_T_H

#include "DcrTable_t.h"
#include "TabWindow.h"

namespace Broker
{

////////////////////////////////////////////////////////////////////////////////

class DcrBase_t :
    public DcrTable_t
{
private:

    using Base_t = DcrTable_t;

    const TableWindow_t& m_tableWindow;
    size_t m_selectedRow;

public:

    DcrBase_t(
        int id,
        TextTable_i* pText,
        const TableWindow_t& tableWindow,
        const TableParams_t& tableParams,
        std::span<const int> columnWidths,
        std::span<const RECT> textRects);

    ~DcrBase_t() override;

    DcrBase_t() = delete;
    DcrBase_t(const DcrBase_t&) = delete;
    DcrBase_t& operator=(const DcrBase_t&) = delete;

    //
    // DCR virtual
    //

    bool
    Initialize() override;

public:

    size_t GetSelectedRow() const { return  m_selectedRow; }


private:

    size_t
    GetSelectedRow(
        CSurface&     surface,
        const Rect_t& tableRect) const;

    static
    const Charset_t*
    InitCharset(
        const LOGFONT& LogFont,
        unsigned flags);

    static
    const Charset_t*
    InitCharset(
        const wchar_t* pCharsetName);

    static 
    void
    InitLogFont(
              LOGFONT& lf,
              int      Height,
        const wchar_t* pszFaceName);
};

////////////////////////////////////////////////////////////////////////////////

} // Broker

#endif  // Include_DCRBASE_T_H

////////////////////////////////////////////////////////////////////////////////
