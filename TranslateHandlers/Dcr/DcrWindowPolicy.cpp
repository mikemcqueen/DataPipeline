////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DcrWindow.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DcrWindow.h"
#include "DdUtil.h"
#include "PipelineManager.h"

extern CDisplay *g_pDisplay;
bool g_noDcrPost = false;

/////////////////////////////////////////////////////////////////////////////

namespace DcrWindow
{
namespace Policy
{

/////////////////////////////////////////////////////////////////////////////
//
// OneTable_t
//
/////////////////////////////////////////////////////////////////////////////
#if !TEMPLATEPOLICY

OneTable_t::
OneTable_t(
    Ui::WindowId_t TopWindowId,
    Ui::WindowId_t DcrWindowId,
    DCR&       Dcr)
//,    RECT*      pRect)
:
    m_TopWindowId(TopWindowId),
    m_DcrWindowId(DcrWindowId),
    m_Dcr(Dcr)
//,    m_pRect(pRect)
{
}

///////////////////////////////////////////////////////////////////////////////


OneTable_t::
~OneTable_t()
{
}

///////////////////////////////////////////////////////////////////////////////

bool
OneTable_t::
Initialize()
{
    return m_Dcr.Initialize();
}

///////////////////////////////////////////////////////////////////////////////

bool
OneTable_t::
PreTranslate(
    const AcquireData_t& Data)
{
#if PENDING
    if (Message::Id::ScreenShot != pData->Id)
        return false;
    if (m_TopWindowType != pData->WindowType)
        return false;
#else
Data;
#endif

#if 0
// some reason this isn't used?
    if (m_TopWindowType != LonWindow_t::GetTopWindow().Type)
        return false;
#endif
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
OneTable_t::
Translate(
    const AcquireData_t& Data)
{
    LogInfo(L"OneTablePolicy_t::Translate()");
    CSurface* pSurface = Data.pPoolItem->get();
    Rect_t Rect;
#if 0
    if (!LonWindow_t::GetWindowRect(
        GetDcrWindowType(),
        Rect,
        LonWindow_t::GetSsWindowType(m_TopWindowType)))
    {
        return false;
    }
#else
    pSurface->GetClientRect(&Rect);
#endif
    if (!m_Dcr.PreTranslateSurface(pSurface, Rect) ||
        !m_Dcr.TranslateSurface(pSurface, Rect))
    {
        LogError(L"OneTablePolicy_t::Translate() failed.");
        return false;
    }
//    if (nullptr != m_pRect)
//        *m_pRect = Rect;
    return true;
}

/////////////////////////////////////////////////////////////////////////////

#if 0
WindowId_t
OneTable_t::
GetDcrWindowType() const
{
    return m_DcrWindowId;
}
#endif

/////////////////////////////////////////////////////////////////////////////
//
// TranslateMany_t
//
/////////////////////////////////////////////////////////////////////////////

TranslateMany_t::
TranslateMany_t(
// TODO: put these back in 
//    WindowId_t TopWindowId,
//    WindowId_t DcrWindowId,
    DcrVector_t& DcrVector)
//,    RECT*        pRect)
:
//    m_TopWindowId(TopWindowId),
//    m_DcrWindowId(DcrWindowId),
    m_DcrVector(DcrVector)
//,    m_pRect(pRect)
{
}

///////////////////////////////////////////////////////////////////////////////


TranslateMany_t::
~TranslateMany_t()
{
}

///////////////////////////////////////////////////////////////////////////////

bool
TranslateMany_t::
Initialize()
{
    DcrVector_t::iterator it = m_DcrVector.begin();
    for (; m_DcrVector.end() != it; ++it)
    {
        if (!(*it)->Initialize())
            return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
TranslateMany_t::
PreTranslate(
    const AcquireData_t& /*Data*/)
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
TranslateMany_t::
Translate(
    const AcquireData_t& Data)
{
    LogInfo(L"TranslateMany_t::Translate()");
    CSurface* pSurface = Data.pPoolItem->get();
    DcrVector_t::iterator it = m_DcrVector.begin();
    for (size_t Index = 0; m_DcrVector.end() != it; ++it, ++Index)
    {
        DCR& Dcr = **it;
        // NOTE: pointless copy.  this rect is empty.
        Rect_t Rect;
        if (!Dcr.PreTranslateSurface(pSurface, Rect) ||
            !Dcr.TranslateSurface(pSurface, Rect))
        {
            LogError(L"TranslateMany_t::Translate() failed on index(%d) of size(%d)",
                     Index, m_DcrVector.size());
            return false;
        }
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////

#endif // !TEMPLATEPOLICY

#if PENDING
/////////////////////////////////////////////////////////////////////////////
//
// TwoTable_t
//
/////////////////////////////////////////////////////////////////////////////

TwoTable_t::
TwoTable_t(
    Window::Type_e TopWindowType,
    DcrTrades_t&        DcrFirst,
    Window::Type_e DcrFirstWindowType,
    DcrTrades_t&        DcrSecond,
    Window::Type_e DcrSecondWindowType)
:
    m_TopWindowType(TopWindowType),
    m_DcrFirst(DcrFirst),
    m_DcrFirstWindowType(DcrFirstWindowType),
    m_DcrSecond(DcrSecond),
    m_DcrSecondWindowType(DcrSecondWindowType)
{
}

/////////////////////////////////////////////////////////////////////////////

bool
TwoTable_t::
PreTranslate(
    const SsTrades_t::AcquireData_t* pData)
{
    if (Message::Id::ScreenShot != pData->Id)
        return false;
    if (m_TopWindowType != pData->WindowType)
        return false;
    if (m_TopWindowType != LonWindow_t::GetTopWindow().Type)
        return false;
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool
TwoTable_t::
Translate(
    const SsTrades_t::AcquireData_t* pData)
{
    LogInfo(L"TwoTable_t::Translate(%d)", pData->Id);

    CSurface* pSurface = pData->pPoolItem->get();
    RECT rcFirst;
    if (!LonWindow_t::GetWindowRect(
        m_DcrFirstWindowType,
        rcFirst,
        LonWindow_t::GetSsWindowType(m_TopWindowType)))
    {
        return false;
    }
    if (!m_DcrFirst.PreTranslateSurface(pSurface, rcFirst) ||
        !m_DcrFirst.TranslateSurface(pSurface, rcFirst))
    {
        LogError(L"TwoTable_t::TranslateFirst(%d) failed.", pData->Id);
        return false;
    }

    RECT rcSecond;
    if (!LonWindow_t::GetWindowRect(
        m_DcrSecondWindowType,
        rcSecond,
        LonWindow_t::GetSsWindowType(m_TopWindowType)))
    {
        return false;
    }
    if (!m_DcrSecond.PreTranslateSurface(pSurface, rcSecond) ||
        !m_DcrSecond.TranslateSurface(pSurface, rcSecond))
    {
        LogError(L"TwoTable_t::TranslateSecond(%d) failed.", pData->Id);
        return false;
    }
    return true;
}

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
    const RECT&     rcBounds)
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
    const RECT&     rcBounds)
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

} // Policy
} // DcrWindow