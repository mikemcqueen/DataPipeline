////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DcrWindowPolicy.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DcrWindow.h"
#include "DcrWindowPolicy.h"
#include "DdUtil.h"
//#include "PipelineManager.h"
#include "Dcr.h"

extern CDisplay *g_pDisplay;
bool g_noDcrPost = false;

/////////////////////////////////////////////////////////////////////////////

namespace DcrWindow::Policy {

namespace Translate {

/////////////////////////////////////////////////////////////////////////////
//
// Translate::Many_t
//
/////////////////////////////////////////////////////////////////////////////

Many_t::Many_t(
    const DcrWindow::Translate::AbstractHandler_t& handler,
    DcrVector_t& DcrVector)
    :
    handler_(handler),
    m_DcrVector(DcrVector)
{ }

Many_t::~Many_t() = default;

bool Many_t::Initialize() {
    DcrVector_t::iterator it = m_DcrVector.begin();
    for (; m_DcrVector.end() != it; ++it) {
        if (!(*it)->Initialize())
            return false;
    }
    return true;
}

bool Many_t::Translate(const AcquireData_t& data) {
  LogInfo(L"Translate::Many_t::Translate()");
  CSurface* pSurface = data.pPoolItem->get();
  return Translate(*pSurface, m_DcrVector, data.WindowId, handler_);
}

// static
bool Many_t::Translate(CSurface& surface, DcrVector_t& dcrVector,
  Ui::WindowId_t windowId, const Handler_t& handler)
{
  auto it = dcrVector.begin();
  for (auto Index = 0; it != dcrVector.end(); ++it, ++Index) {
    DCR& dcr = **it;
    Rect_t rect;
    if (!handler.PreTranslateSurface(&surface, windowId, dcr.id(), &rect)
      || !dcr.TranslateSurface(&surface, rect))
    {
      LogError(L"Translate::Many_t::Translate() failed on index(%d) of size(%d)",
        Index, dcrVector.size());
      // TODO? TEMP? return false;
    }
  }
  return true;
}

} // Translate

#ifdef PENDING
/////////////////////////////////////////////////////////////////////////////
//
// ValidateWindow_t
//
/////////////////////////////////////////////////////////////////////////////

ValidateWindow_t::
ValidateWindow_t()
{
    Initialize();
}

/////////////////////////////////////////////////////////////////////////////

bool
ValidateWindow_t::
Initialize()
{
    if (!InitAllBitmaps())
    {
        LogError(L"TrWindow_t::InitAllBitmaps() failed.");
        return false;
    }
    return true;
}
/////////////////////////////////////////////////////////////////////////////

bool
ValidateWindow_t::
InitAllBitmaps()
{
  static int rgBitmapIds[BitmapCount] = {
      IDB_WINDOW_TOPLEFT,
      IDB_WINDOW_TOPRIGHT,
      IDB_WINDOW_BOTTOMLEFT,
      IDB_WINDOW_BOTTOMRIGHT
  };
  static int rgSideBitmapIds[BitmapCount] = {
      IDB_BITMAP1,// top
      IDB_BITMAP2,// right
      IDB_BITMAP3,// left
      IDB_BITMAP4 // bottom
  };

  for (size_t Bitmap = 0; Bitmap < BitmapCount; ++Bitmap)
  {
    if (nullptr == s_rgpCornerSurfaces[Bitmap])
    {
      CSurface* pSurface = new CSurface();
      HRESULT hr = g_pDisplay->CreateSurfaceFromBitmap(pSurface,
        MAKEINTRESOURCE(rgBitmapIds[Bitmap]));
      if (SUCCEEDED(hr))
        s_rgpCornerSurfaces[Bitmap] = pSurface;
      else
        delete pSurface;
    }
    if (nullptr == s_rgpSideSurfaces[Bitmap])
    {
      CSurface* pSurface = new CSurface();
      HRESULT hr = g_pDisplay->CreateSurfaceFromBitmap(pSurface,
        MAKEINTRESOURCE(rgSideBitmapIds[Bitmap]));
      if (SUCCEEDED(hr))
        s_rgpSideSurfaces[Bitmap] = pSurface;
      else
        delete pSurface;
    }
  }
  return true;
}

/////////////////////////////////////////////////////////////////////////////

bool
ValidateWindow_t::
Validate(
    const SsTrades_t::AcquireData_t* pData)
{
    CSurface* pSurface = pData->pPoolItem->get();
    RECT rcSurface;
    ::GetClientRect(LonWindow_t::GetTopWindow().hWnd, &rcSurface);
    if (!ValidateSides(pSurface, rcSurface))
    {
        LogInfo(L"ValidateWindow_t::ValidateSides() failed.");
        return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool
ValidateWindow_t::
ValidateSides(
  const CSurface* pSurface,
  const RECT& rcBounds)
{
  // Top
  CSurface* pSide = s_rgpSideSurfaces[0];
  if (!pSurface->Compare((RECTWIDTH(rcBounds) + pSide->GetWidth()) / 2, 0,
    pSide, 0, 0, pSide->GetWidth(), pSide->GetHeight(),
    COMPARE_F_NOTSRCTRANSPARENT))
    //                          COMPARE_F_NOTDSTTRANSPARENT))
  {
#if 1
    RECT rc;
    rc.left = (RECTWIDTH(rcBounds) + pSide->GetWidth()) / 2;
    rc.top = 0;
    rc.right = rc.left + pSide->GetWidth();
    rc.bottom = rc.top + pSide->GetHeight();
    pSurface->WriteBMP(L"bmp\\Top.bmp", rc);
#endif
    LogInfo(L"Top line mismatch");
    return false;
  }

  // Bottom
  pSide = s_rgpSideSurfaces[3];
  if (!pSurface->Compare((RECTWIDTH(rcBounds) + pSide->GetWidth()) / 2,
    RECTHEIGHT(rcBounds) - pSide->GetHeight(),
    pSide, 0, 0, pSide->GetWidth(), pSide->GetHeight(),
    COMPARE_F_NOTSRCTRANSPARENT))
    //                          COMPARE_F_NOTDSTTRANSPARENT))
  {
#if 1
    RECT rc;
    rc.left = (RECTWIDTH(rcBounds) + pSide->GetWidth()) / 2;
    rc.top = RECTHEIGHT(rcBounds) - pSide->GetHeight();
    rc.right = rc.left + pSide->GetWidth();
    rc.bottom = rc.top + pSide->GetHeight();
    pSurface->WriteBMP(L"bmp\\Bottom.bmp", rc);
#endif
    LogInfo(L"Bottom line mismatch");
    return false;
  }

  // Right
  pSide = s_rgpSideSurfaces[1];
  if (!pSurface->Compare(rcBounds.right - pSide->GetWidth(),
    (RECTHEIGHT(rcBounds) + pSide->GetHeight()) / 2,
    pSide, 0, 0, pSide->GetWidth(), pSide->GetHeight(),
    COMPARE_F_NOTSRCTRANSPARENT))
    //                          COMPARE_F_NOTDSTTRANSPARENT))
  {
#if 1
    RECT rc;
    rc.left = rcBounds.right - pSide->GetWidth();
    rc.top = (RECTHEIGHT(rcBounds) + pSide->GetHeight()) / 2;
    rc.right = rc.left + pSide->GetWidth();
    rc.bottom = rc.top + pSide->GetHeight();
    pSurface->WriteBMP(L"bmp\\Right.bmp", rc);
#endif
    LogInfo(L"Right line mismatch");
    return false;
  }

  // Left
  pSide = s_rgpSideSurfaces[2];
  if (!pSurface->Compare(0, (RECTHEIGHT(rcBounds) + pSide->GetHeight()) / 2,
    pSide, 0, 0, pSide->GetWidth(), pSide->GetHeight(),
    COMPARE_F_NOTSRCTRANSPARENT))
    //                          COMPARE_F_NOTDSTTRANSPARENT))
  {
#if 1
    RECT rc;
    rc.left = 0;
    rc.top = (RECTHEIGHT(rcBounds) + pSide->GetHeight()) / 2;
    rc.right = rc.left + pSide->GetWidth();
    rc.bottom = rc.top + pSide->GetHeight();
    pSurface->WriteBMP(L"bmp\\Left.bmp", rc);
#endif
    LogInfo(L"Left line mismatch");
    return false;
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////

bool
ValidateWindow_t::
ValidateCorners(
  const CSurface* pSurface,
  const RECT& rcBounds)
{
  // TopLeft
  CSurface* pCorner = s_rgpCornerSurfaces[0];
  if (!pSurface->Compare(0, 0, pCorner,
    0, 0, pCorner->GetWidth(), pCorner->GetHeight(),
    COMPARE_F_NOTSRCTRANSPARENT))
    //                          COMPARE_F_NOTDSTTRANSPARENT))
  {
    RECT rc;
    SetRect(&rc, 0, 0, pCorner->GetWidth(), pCorner->GetHeight());
    pSurface->WriteBMP(L"bmp\\topleft.bmp", rc);
    LogInfo(L"TopLeft corner mismatch");
    return false;
  }
  // TopRight
  pCorner = s_rgpCornerSurfaces[1];
  /*    if (!pCorner->Compare(0, 0, pSurface,
                            rc.right - pCorner->GetWidth(), 0,
                            pCorner->GetWidth(), pCorner->GetHeight(),
                            COMPARE_F_NOTSRCTRANSPARENT))
  */
  if (!pSurface->Compare(rcBounds.right - pCorner->GetWidth(), 0,
    pCorner, 0, 0, pCorner->GetWidth(), pCorner->GetHeight(),
    COMPARE_F_NOTDSTTRANSPARENT))
  {
    RECT rc;
    SetRect(&rc, rcBounds.right - pCorner->GetWidth(), 0, pCorner->GetWidth(), pCorner->GetHeight());
    pSurface->WriteBMP(L"bmp\\topright.bmp", rc);
    LogInfo(L"TopRight corner mismatch");
    return false;
  }
  // BottomLeft
  pCorner = s_rgpCornerSurfaces[2];
  /*
      if (!pCorner->Compare(0, 0, pSurface,
                            0, rc.bottom - pCorner->GetHeight(),
                            pCorner->GetWidth(), pCorner->GetHeight(),
                            COMPARE_F_NOTSRCTRANSPARENT))
  */
  if (!pSurface->Compare(0, rcBounds.bottom - pCorner->GetHeight(),
    pCorner, 0, 0, pCorner->GetWidth(), pCorner->GetHeight(),
    COMPARE_F_NOTDSTTRANSPARENT))
  {
    RECT rc;
    SetRect(&rc, 0, rcBounds.bottom - pCorner->GetHeight(), pCorner->GetWidth(), pCorner->GetHeight());
    pSurface->WriteBMP(L"bmp\\bottomleft.bmp", rc);
    LogInfo(L"BottomLeft corner mismatch");
    return false;
  }
  // BottomRight
  pCorner = s_rgpCornerSurfaces[3];
  /*
      if (!pCorner->Compare(0, 0, pSurface,
                            rc.right - pCorner->GetWidth(), rc.bottom - pCorner->GetHeight(),
                            pCorner->GetWidth(), pCorner->GetHeight(),
                            COMPARE_F_NOTSRCTRANSPARENT))
  */
  if (!pSurface->Compare(rcBounds.right - pCorner->GetWidth(), rcBounds.bottom - pCorner->GetHeight(),
    pCorner, 0, 0, pCorner->GetWidth(), pCorner->GetHeight(),
    COMPARE_F_NOTDSTTRANSPARENT))

  {
    //        SetRect(&rc, rcBounds.right - pCorner->GetWidth(), rcBounds.bottom - pCorner->GetHeight(),
    //                pCorner->GetWidth(), pCorner->GetHeight());

    LogInfo(L"BottomRight corner mismatch");
    return false;
  }
  return true;
}

/////////////////////////////////////////////////////////////////////////////
#endif // PENDING

} // DcrWindow::Policy
