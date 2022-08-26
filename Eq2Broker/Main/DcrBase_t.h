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

    typedef DcrTable_t Base_t;

    static const int     CharsetPointSize     = 9;
    static const wchar_t CharsetFacename[];

private:

    const TableWindow_t& m_tableWindow;
    size_t m_selectedRow;

public:

    //
    // Constructor and destructor:
    //
    DcrBase_t(
              TextTable_i*   pText,
        const TableWindow_t& tableWindow,
        const ScreenTable_t& ScreenTable);

    virtual
    ~DcrBase_t();

    //
    // DCR virtual
    //
    virtual
    bool
    Initialize() override;

    virtual 
    bool
    PreTranslateSurface(
        CSurface* pSurface,
        Rect_t&   rcSurface) override;

public:

    size_t GetSelectedRow() const { return  m_selectedRow; }

    static
    bool
    InitAllCharsets();

    static
    const Charset_t*
    GetCharset();

private:

    size_t
    GetSelectedRow(
        CSurface&     surface,
        const Rect_t& tableRect) const;

    static
    const Charset_t*
    InitCharset(
        const LOGFONT& LogFont);

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

private:

    DcrBase_t();
    DcrBase_t(const DcrBase_t&);
    DcrBase_t& operator=(const DcrBase_t&);
};

////////////////////////////////////////////////////////////////////////////////

} // Broker

#endif  // Include_DCRBASE_T_H

////////////////////////////////////////////////////////////////////////////////
