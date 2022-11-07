/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2022 Mike McQueen.  All rights reserved.
//
// TesseractDcrImpl_t.h
//
// Tesseract Digital Character Recognition implementation.
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TESSERACTDCRIMPL_T_H
#define Include_TESSERACTDCRIMPL_T_H

class Rect_t;
class Charset_t;
class CSurface;
class TableInfo_t;
class TextTable_i;

namespace tesseract {
    class TessBaseAPI;
}

class TesseractDcrImpl_t final
{
private:
    std::unique_ptr<tesseract::TessBaseAPI> tesseract_;

public:
    std::string GetText(
        const CSurface* pSurface,
        const Rect_t& rect) const;

    int GetTableText(
        const CSurface* pSurface,
        const Rect_t& rcTable,
        const TableInfo_t& tableInfo,
        const std::vector<Rect_t>& columnRects,
        const std::vector<std::unique_ptr<CSurface>>& columnSurfaces,
        TextTable_i* pTextTable) const;

    auto Tesseract() const {
        return tesseract_.get();
    }

    int InitTesseract(
        const char* dataPath,
        const char* languageCode);

    void EndTesseract();
};

#endif // Include_TESSERACTDCRIMPL_T_H