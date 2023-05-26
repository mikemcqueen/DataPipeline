////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2022 Mike McQueen.  All rights reserved.
//
// TesseractDcrImpl_t.cpp
//
// Tesseract Digital Character Recognition implementation.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "tesseract/baseapi.h"
#include "TesseractDcrImpl_t.h"
#include "DDUtil.h"
#include "Log.h"
#include "DcrTable_t.h"
#include "TextTable_t.h"
#include "Macros.h"

TesseractDcrImpl_t* impl_ = nullptr;

//static
void TesseractDcrImpl_t::Init() {
  if (impl_) {
    throw std::runtime_error("Tesseract already initialized");
  }
  auto tess{ std::make_unique<TesseractDcrImpl_t>() };
  if (auto result = tess->InitTesseract(nullptr, "eng"); result < 0) {
    throw std::runtime_error(std::format("InitTesseract() failed, {}", result));
  }
  auto temp = tess.get();
  DCR::add_impl<TesseractDcrImpl_t>(DcrImpl::Tesseract, std::move(tess));
  impl_ = temp;
}

//static
void TesseractDcrImpl_t::Cleanup() {
  auto temp = impl_;
  if (temp) {
    impl_ = nullptr;
    auto hold = std::move(DCR::remove_impl(DcrImpl::Tesseract));
    temp->EndTesseract();
  }
}

std::string TesseractDcrImpl_t::GetText(
  const CSurface* pSurface,
  const Rect_t& rect) const
{
  std::string str;
  DDSURFACEDESC2 ddsd;
  HRESULT hr = pSurface->Lock(&ddsd);
  if (FAILED(hr)) {
    LogError(L"Can't lock surface");
    return str;
  }
  auto unlock = gsl::finally([pSurface] { pSurface->Unlock(nullptr); });
  Tesseract()->SetImage((std::uint8_t*)GetBitsAt(&ddsd, rect.left, rect.top),
    rect.Width(), rect.Height(),
    4, (int)ddsd.lPitch);

//  Tesseract()->SetVariable("tessedit_char_whitelist", "1234567890");

  std::unique_ptr<char> pResult(Tesseract()->GetUTF8Text());
  if (auto text = pResult.get(); text && text[0]) {
    str.assign(text);
    str.erase(str.end() - 1);
  }
  return str;
}

int TesseractDcrImpl_t::GetTableText(
  const CSurface* pSurface,
  const Rect_t& rcTable,
  const TableInfo_t& tableInfo,
  const std::vector<Rect_t>& columnRects,
  const std::vector<std::unique_ptr<CSurface>>& columnSurfaces,
  TextTable_i* pTextTable) const
{
  columnSurfaces;

  static auto firstTime = true;
  auto writeBmps = true;

  RECT rcRow{ rcTable }; // copy
  rcRow.bottom = rcRow.top + tableInfo.GetRowHeight();

  DDSURFACEDESC2 ddsd;
  if (FAILED(pSurface->Lock(&ddsd))) {
    LogError(L"Can't lock surface");
    return 0;
  }
  auto unlock = gsl::finally([pSurface] { pSurface->Unlock(nullptr); });

  auto row = 0;
  for (auto yOffset = 0; row < pTextTable->GetRowCount(); ++row)
  {
    if (rcRow.bottom > rcTable.bottom) {
      LogWarning(L"bottom > rcTable.bottom: (rc.top=%d, rc.bottom=%d, pRect->bottom=%d, Row=%d",
        rcRow.top, rcRow.bottom, rcTable.bottom, row);
      break;
    }
    if (writeBmps && firstTime) {
      pSurface->WriteBMP(std::format(L"diag\\dcr_row_{}.bmp", row).c_str(), rcRow);
    }
#if 0   
    // TODO: This "verification" function should be virtual, dependent
    //       on the specific source window we took the screenshot of.

    bool b = false;
    if (b && !VerifyBlankRows(pSurface, pColumnRects[0], MaxCharHeight, bBottom))
    {
      // TODO: text.SetInvalidRow(row);
      size_t Pos = 0;
      for (size_t Column = 0; Column < pText->GetColumnCount(); ++Column)
      {
        size_t Width = pText->GetColumnWidth(Column);
        wcscpy_s(&pszRow[Pos], Width, L"BAD");
        Pos += Width;
      }
      continue;
    }
#endif
#define LOGROWTEXT 1
#if LOGROWTEXT
    std::stringstream text;
#endif
    pTextTable->SetRowRect(row, rcRow);
    pTextTable->ClearRow(row);
    for (size_t column = 0; column < columnRects.size(); ++column) {
      auto& rc = columnRects[column];
      if (writeBmps && firstTime) {
        RECT rcBmp{ rc }; // copy
        ::OffsetRect(&rcBmp, 0, yOffset);
        pSurface->WriteBMP(std::format(L"diag\\table_row_{}_col_{}.bmp",
          row, column).c_str(), rcBmp);
        WCHAR szFile[MAX_PATH];
        wsprintf(szFile, L"Diag\\dcr_row_%d_column_%d.bmp", row, column);
#if 0 // draw rectangles
        const_cast<CSurface*>(pSurface)->SlowRectangle(&rcBmp, RGB(0, 0, 255));
#endif
      }
      Tesseract()->SetImage((std::uint8_t*)GetBitsAt(&ddsd, rc.left, rc.top + yOffset),
        RECTWIDTH(rc), RECTHEIGHT(rc),
        4, (int)ddsd.lPitch);
      std::unique_ptr<char> pResult(Tesseract()->GetUTF8Text());
      if (auto columnText = pResult.get(); columnText) {
        if (columnText[0]) { // length > 0
          std::string str{ columnText };
          str.erase(str.end() - 1);
          pTextTable->SetText(row, column, str);
        }
#if LOGROWTEXT
        if (columnText[0]) {
          text << columnText;
          text.seekp(-1, std::ios_base::end);
        }
        else text << "[empty]";
        text << " | ";
#endif
      }
    }
#if LOGROWTEXT
    LogInfo(L"Text row %d: %S (%d)", row, text.str().c_str(), text.str().length());
#endif
    auto yExtent = tableInfo.GetRowHeight() + tableInfo.GetRowGapSize();
    ::OffsetRect(&rcRow, 0, yExtent);
    yOffset += yExtent;
  }
  // TODO ? pTextTable->SetEndRow(row);
  firstTime = false;
  return row;
}

int TesseractDcrImpl_t::InitTesseract(
  const char* dataPath,
  const char* languageCode)
{
  if (tesseract_) {
    throw new std::logic_error("Tesseract already initialized");
  }
  tesseract_ = std::make_unique<tesseract::TessBaseAPI>();
  return tesseract_->Init(dataPath, languageCode);
}

void 
TesseractDcrImpl_t::EndTesseract() {
  if (tesseract_) {
    tesseract_->End();
  }
}
