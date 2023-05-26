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
#include "Timer_t.h"

DcrTable_t::DcrTable_t(
  int id,
  std::optional<DcrImpl> method,
  TextTable_i* pTextTable,
  const TableParams_t& tableParams,
  std::span<const int> columnWidths,
  std::span<const Rect_t> textRects) :
    DCR(id, method),
    pTextTable_(pTextTable),
    tableInfo_(tableParams, columnWidths, textRects)
{}

DcrTable_t::~DcrTable_t() = default;

bool DcrTable_t::Initialize() {
  // TODO: exceptions probably more consistent here
  columnRects_ = InitColumnRects(tableInfo_);
  if (columnRects_.empty()) {
    LogError(L"Empty columnRects");
    return false;
  }
#if 0
  columnSurfaces_ = InitColumnSurfaces(columnRects_);
  if (columnSurfaces_.size() != columnRects_.size()) {
      LogError(L"Column surface count mismatch");
      return false;
  }
#endif
  return true;
}

bool DcrTable_t::TranslateSurface(CSurface* pSurface, const Rect_t& rcSurface) {
  LogInfo(L"DcrTable_t::TranslateSurface");
  auto start = std::chrono::high_resolution_clock::now();

  Timer_t t("DcrTable_t::ReadTable");
  auto rowCount = ReadTable(pSurface, rcSurface, pTextTable_);
  t.done();

  if (0 == rowCount) {
    LogInfo(L"ReadTable(): Table is empty.");
    selected_row_ = std::nullopt;
  }
  else {
    ColorRange_t colors{ RGB(45, 35, 40), RGB(50, 40, 45) };
    selected_row_ = GetSelectedRow(*pSurface, rcSurface, colors);
  }
#if 1
  static bool firstTime = true;
  if (firstTime) {
    pSurface->WriteBMP(L"Diag\\DcrTable_t.bmp", rcSurface);
    firstTime = false;
  }
#endif
  return true;
}

std::optional<int> DcrTable_t::GetSelectedRow(
  CSurface& surface,
  const Rect_t& tableRect,
  const ColorRange_t& colors) const
{
  auto rowHeight = tableInfo_.GetRowHeight() + tableInfo_.GetRowGapSize();
  const size_t width = 4;
  const size_t height = 1;
  Rect_t selectRect;
  selectRect.left = tableRect.left + tableRect.Width() / 2 - width / 2;
  selectRect.right = selectRect.left + width;
  selectRect.top = tableRect.top + 1; // +1 in case we have SlowRectangle turned on. Hack. TODO.
  selectRect.bottom = selectRect.top + height;
  std::optional<int> selectedRow{};
  for (int row = 0; selectRect.top + rowHeight <= tableRect.bottom; ++row) {
    if (surface.CompareColorRange(selectRect, colors.low, colors.high)) {
      if (selectedRow) {
        LogError(L"Dcr::Table_t::GetSelectedRow() Two rows selected (%d,%d)",
          selectedRow.value(), row);
        return std::nullopt;
      }
      selectedRow.emplace(row);
    }
    //surface.SlowRectangle(&selectRect, RGB(255, 0, 0));
    ::OffsetRect(&selectRect, 0, rowHeight);
  }
  return selectedRow;
}

std::vector<Rect_t> DcrTable_t::InitColumnRects(
  const TableInfo_t& tableInfo) const
{
  std::vector<Rect_t> columnRects(tableInfo_.GetColumnCount());
  for (auto x = 0, column = 0; column < tableInfo_.GetColumnCount(); ++column) {
    auto width = tableInfo.GetColumnWidth(column);
    if (0 == width) {
      throw std::invalid_argument("Dcr::Table_t::ReadTable() Zero pixel column width not allowed");
    }
    auto& rc = tableInfo.GetTextRect(column);
    if (!::IsRectEmpty(&rc)) {
      columnRects[column] = rc; // copy
      //::OffsetRect(&columnRects[column], x, 0); // offset copy
    } else {
      ::SetRect(&columnRects[column], x, 0, x + width, tableInfo.GetRowHeight());
    }
    x += width;
  }
  return columnRects;
}

std::vector<std::unique_ptr<CSurface>> DcrTable_t::InitColumnSurfaces(
  const std::vector<RECT>& columnRects) const
{
  extern CDisplay* g_pDisplay;
  std::vector<std::unique_ptr<CSurface>> surfaces;
  for (auto& rc : columnRects) {
    std::unique_ptr<CSurface> pSurface = std::make_unique<CSurface>();
    HRESULT hr = g_pDisplay->CreateSurface(pSurface.get(), RECTWIDTH(rc), RECTHEIGHT(rc));
    if (FAILED(hr)) {
      throw new std::runtime_error(std::format("CreateSurface failed {}x{}",
        RECTWIDTH(rc), RECTHEIGHT(rc)));
    }
    surfaces.push_back(std::move(pSurface));
  }
  return surfaces;
}

int DcrTable_t::ReadTable(
  const CSurface* pSurface,
  const Rect_t& rcTable,
  TextTable_i* pText)
{
  std::vector<Rect_t> copyColumnRects{ columnRects_ };
  for (auto& rc : copyColumnRects) {
    ::OffsetRect(&rc, rcTable.left, rcTable.top);
  }
  return impl().GetTableText(
    pSurface,
    rcTable,
    tableInfo_,
    copyColumnRects,
    columnSurfaces_,
    pText);
}
