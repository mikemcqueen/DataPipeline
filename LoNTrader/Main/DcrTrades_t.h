////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DcrTrades_t.h
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_DCRTRADES_T_H
#define Include_DCRTRADES_T_H

#include "DcrTable_t.h"
#include "LonWindow_t.h"

////////////////////////////////////////////////////////////////////////////////

class DcrTrades_t final :
    public DcrTable_t
{
private:

    typedef DcrTable_t Base_t;

    static const int    DefaultLineHeight    = 16;
    static const size_t DefaultCharHeight    = 12; // 13
    static const size_t DefaultCharOffsetY   = 0;

    static const size_t BitmapCount          = 4;

private:

    static CSurface* s_rgpCornerSurfaces[BitmapCount];
    static CSurface* s_rgpSideSurfaces[BitmapCount];

    //
    // Members:
    //

public:

    //
    // Constructor and destructor:
    //

    DcrTrades_t(
              TextTable_i* pText,
        const size_t*      pColumnWidths,
              size_t       ColumnCount,
              size_t       LineHeight  = DefaultLineHeight,
              size_t       CharHeight  = DefaultCharHeight,
              size_t       CharOffsetY = DefaultCharOffsetY);

    virtual
    ~DcrTrades_t();

    bool
    Initialize();

    // TODO: make part of DpHandler_t, called by PM, then call here via TablePolicy
    // TODO: make non-static
    static
    void
    Shutdown();

    bool
    PreTranslateSurface(
        CSurface* pSurface,
        Rect_t&   rcSurface) override;

private:

    bool
    InitAllCharsets();

    const Charset_t*
    InitCharset(
        const LOGFONT& LogFont);

    void
    InitLogFont(
        LOGFONT& lf,
        size_t   PointSize);

    bool
    InitSurfaceRect(
        const CSurface* pSurface,
        RECT&           rcSurface);

    int
    GetTopLine(
        const CSurface* pSurface,
        const RECT&     rcSurface);

private:

    DcrTrades_t() = delete;
    DcrTrades_t(const DcrTrades_t&) = delete;
    DcrTrades_t& operator=(const DcrTrades_t&) = delete;
};

////////////////////////////////////////////////////////////////////////////////

#endif  // Include_DCRTRADES_T_H

////////////////////////////////////////////////////////////////////////////////
