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

#include "Dcr.h"
#include "ScreenTable_t.h"
#include "ColorRange_t.h"

struct Rect_t;
class Charset_t;
class TextTable_i;

class TableInfo_t
{
public:
    const TableParams_t params_;
    const std::vector<int> columnWidths_;
    const std::vector<Rect_t> textRects_;

    TableInfo_t(
        const TableParams_t& params,
        std::span<const int> columnWidths, // std::range
        std::span<const Rect_t> textRects)
        :
        params_(params),
        columnWidths_(std::begin(columnWidths), std::end(columnWidths)),
        textRects_(std::begin(textRects), std::end(textRects))
    { }

    TableInfo_t() = delete;
    TableInfo_t(const TableInfo_t&) = delete;
    TableInfo_t& operator=(const TableInfo_t&) = delete;

    auto GetRowHeight() const { return params_.RowHeight; }
    auto GetCharHeight() const { return params_.CharHeight; }
    auto GetRowGapSize() const { return params_.RowGapSize; }
    auto GetColumnCount() const { return params_.ColumnCount; }
    auto GetColumnWidth(int column) const { return columnWidths_[column]; }
    const auto& GetTextRect(int column) const { return textRects_[column]; }
    auto GetTotalColumnWidths() const { return std::accumulate(std::cbegin(columnWidths_), std::cend(columnWidths_), 0); }
};

/////////////////////////////////////////////////////////////////////////////

class DcrTable_t : public DCR {
public:
  DcrTable_t(
    int id,
    std::optional<DcrImpl> method,
    TextTable_i* pText,
    const TableParams_t& tableParams,
    std::span<const int> columnWidths,
    std::span<const Rect_t> textRects);

  DcrTable_t() = delete;
  DcrTable_t(const DcrTable_t&) = delete;
  DcrTable_t& operator=(const DcrTable_t&) = delete;
  ~DcrTable_t() override;

  // DCR virtual:
  bool Initialize() override;
  bool TranslateSurface(CSurface* pSurface, const Rect_t& rect) override;

  //

  int GetSelectedRow(
    CSurface& surface,
    const Rect_t& tableRect,
    const ColorRange_t& colors) const;

  void SetTextTable(TextTable_i* pTextTable) { pTextTable_ = pTextTable; }

  const TextTable_i* GetTextTable() const { return pTextTable_; }
  const TableInfo_t& GetTableInfo() const { return tableInfo_; }

private:

  std::vector<Rect_t> InitColumnRects(const TableInfo_t& tableInfo) const;

  std::vector<unique_ptr<CSurface>> InitColumnSurfaces(
    const std::vector<RECT>& columnRects) const;

  int ReadTable(
    const CSurface* pSurface,
    const Rect_t& rcTable,
    TextTable_i* pText);

  int TesseractReadTable(
    const CSurface* pSurface,
    const RECT& rcTable,
    const int RowHeight,
    const int RowGapSize,
    const std::vector<RECT>& columnRects,
    const std::vector<std::unique_ptr<CSurface>>& columnSurfaces,
    TextTable_i* pText) const;

private:
  const TableInfo_t tableInfo_;
  TextTable_i* pTextTable_;
  std::vector<Rect_t> columnRects_;
  std::vector<std::unique_ptr<CSurface>> columnSurfaces_;
};

#endif // Include_DCRTABLE_T_H
