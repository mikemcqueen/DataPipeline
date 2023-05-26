/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// DcrRect_t.h
//
// TODO: DcrWidget_t.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_DCRRECT_T_H
#define Include_DCRRECT_T_H

#include "Dcr.h"
#include "Rect.h"

class CSurface;

class DcrRect_t : public DCR {
public:
  explicit DcrRect_t(
    int id,
    std::optional<DcrImpl> method = {},
    bool checkCaret = false,
    COLORREF highBkColor = RGB(0, 0, 0));

  DcrRect_t(const DcrRect_t&) = delete;
  DcrRect_t& operator=(const DcrRect_t&) = delete;

  // DCR virtual:
  bool TranslateSurface(CSurface* pSurface, const Rect_t& rect) override;

  const std::string& GetText() const { return text_; }
  bool GetHasCaret() const { return m_hasCaret; }

private:
  bool TranslateRect(CSurface* pSurface, const Rect_t& surfaceRect);

  RelativeRect_t   m_Rect;
  std::string      text_;
  // These options are for LegacyDcr. Interface doesn't support passing them through (yet). TBD.
  bool             m_checkCaret;
  bool             m_hasCaret;
  COLORREF         m_highBkColor;
};

#endif // Include_DCRRECT_T_H
