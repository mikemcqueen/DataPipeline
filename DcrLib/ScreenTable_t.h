////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// ScreenTable_t.h
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_SCREENTABLE_T_H
#define Include_SCREENTABLE_T_H

struct ColumnData_t
{
    typedef unsigned Flag_t;
    struct Flag
    {
        enum E : Flag_t
        {
            MultiLine     = 0x0001,  // Text may span multiple lines on a single row
            Optional      = 0x0002,  // Text may not be present
            NumericOnly   = 0x0004,  // Only numeric digits
            CharsetOpaque = 0x0008,  // Only compare opaque pixels in charset, ignore
        };
    };

    size_t  Width;
    RECT    TextRect; // can be empty, in which case it is calculated
    Flag_t  Flags;
};

struct ScreenTable_t
{
    size_t        RowHeight;
    size_t        CharHeight;
    size_t        OffsetY;
    size_t        ColumnCount;

    const int*    pColumnWidths;
    const RECT*   pTextRects;
};

#endif // Include_SCREENTABLE_T_H
