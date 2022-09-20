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

struct TableParams_t {
    int RowHeight;
    int CharHeight;
    int RowGapSize;
    int ColumnCount;
};

class ScreenTable_t
{
public:
    TableParams_t params_;
    std::vector<int> columnWidths_;
    std::vector<RECT> textRects_;

    ScreenTable_t(
        const TableParams_t& params,
        std::span<const int> columnWidths, // std::range
        std::span<const RECT> textRects)
        :
        params_(params),
        columnWidths_(begin(columnWidths), end(columnWidths)),
        textRects_(begin(textRects), end(textRects))
    { }

    ScreenTable_t() = delete;
    ScreenTable_t(const ScreenTable_t&) = delete;
    ScreenTable_t& operator=(const ScreenTable_t&) = delete;

    auto GetRowHeight() const { return params_.RowHeight; }
    auto GetCharHeight() const { return params_.CharHeight; }
    auto GetRowGapSize() const { return params_.RowGapSize; }
    auto GetColumnCount() const { return params_.ColumnCount; }
    auto GetColumnWidth(int column) const { return columnWidths_[column]; }
    auto& GetTextRect(int column) const { return textRects_[column]; }
    auto GetTotalColumnWidths() const { return std::accumulate(begin(columnWidths_), end(columnWidths_), 0); }
};

#endif // Include_SCREENTABLE_T_H
