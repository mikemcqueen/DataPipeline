////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TrWindow_t.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TrWindow_t.h"
#include "DdUtil.h"
#include "TiBase_t.h"
#include "PipelineManager.h"
#include "LonWindow_t.h"
#include "resource.h"
#include "LonTypes.h"

using namespace Lon;

/////////////////////////////////////////////////////////////////////////////

extern CDisplay *g_pDisplay;

static const size_t BitmapCount = 4;
CSurface* s_rgpCornerSurfaces[BitmapCount];
CSurface* s_rgpSideSurfaces[BitmapCount];

/////////////////////////////////////////////////////////////////////////////
//
// OneTablePolicy_t
//
/////////////////////////////////////////////////////////////////////////////

OneTablePolicy_t::
OneTablePolicy_t(
    Window::Type_e TopWindowType,
    DcrTrades_t&        DcrTrades,
    Window::Type_e DcrWindowType,
    RECT*               pRect)
:
    m_TopWindowType(TopWindowType),
    m_Dcr(DcrTrades),
    m_DcrWindowType(DcrWindowType),
    m_pRect(pRect)
{
}

///////////////////////////////////////////////////////////////////////////////


OneTablePolicy_t::
~OneTablePolicy_t()
{
}

///////////////////////////////////////////////////////////////////////////////

bool
OneTablePolicy_t::
PreTranslate(
    const SsTrades_t::AcquireData_t* pData)
{
    if (Message::Id::ScreenShot != pData->Id)
        return false;
    if (m_TopWindowType != pData->WindowType)
        return false;
#if 0
// some reason this isn't used?
    if (m_TopWindowType != LonWindow_t::GetTopWindow().Type)
        return false;
#endif
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
OneTablePolicy_t::
Translate(
    const SsTrades_t::AcquireData_t* pData)
{
    LogInfo(L"OneTablePolicy_t::Translate()");

    RECT Rect;
    if (!LonWindow_t::GetWindowRect(
        GetDcrWindowType(),
        Rect,
        LonWindow_t::GetSsWindowType(m_TopWindowType)))
    {
        return false;
    }
	Rect_t r(Rect); // NOTE added to get it compiling
	CSurface* pSurface = pData->pPoolItem->get();
    if (!m_Dcr.PreTranslateSurface(pSurface, r) ||
        !m_Dcr.TranslateSurface(pSurface, r))
    {
        LogError(L"OneTablePolicy_t::Translate() failed.");
        return false;
    }
    if (NULL != m_pRect)
        *m_pRect = Rect;
    return true;
}

/////////////////////////////////////////////////////////////////////////////

Window::Type_e
OneTablePolicy_t::
GetDcrWindowType() const
{
    return m_DcrWindowType;
}

/////////////////////////////////////////////////////////////////////////////
//
// TwoTablePolicy_t
//
/////////////////////////////////////////////////////////////////////////////

TwoTablePolicy_t::
TwoTablePolicy_t(
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
TwoTablePolicy_t::
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
TwoTablePolicy_t::
Translate(
    const SsTrades_t::AcquireData_t* pData)
{
    LogInfo(L"TwoTablePolicy_t::Translate(%d)", pData->Id);

    CSurface* pSurface = pData->pPoolItem->get();
    RECT rcFirst;
    if (!LonWindow_t::GetWindowRect(
        m_DcrFirstWindowType,
        rcFirst,
        LonWindow_t::GetSsWindowType(m_TopWindowType)))
    {
        return false;
    }
	Rect_t r(rcFirst); // NOTE added to get it compiling
    if (!m_DcrFirst.PreTranslateSurface(pSurface, r) ||
        !m_DcrFirst.TranslateSurface(pSurface, r))
    {
        LogError(L"TwoTablePolicy_t::TranslateFirst(%d) failed.", pData->Id);
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
	Rect_t r2(rcSecond);
    if (!m_DcrSecond.PreTranslateSurface(pSurface, r2) ||
        !m_DcrSecond.TranslateSurface(pSurface, r2))
    {
        LogError(L"TwoTablePolicy_t::TranslateSecond(%d) failed.", pData->Id);
        return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////
//
// ValidateWindowPolicy_t
//
/////////////////////////////////////////////////////////////////////////////

ValidateWindowPolicy_t::
ValidateWindowPolicy_t()
{
    Initialize();
}

/////////////////////////////////////////////////////////////////////////////

bool
ValidateWindowPolicy_t::
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
ValidateWindowPolicy_t::
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
        if (NULL == s_rgpCornerSurfaces[Bitmap])
        {
            CSurface* pSurface = new CSurface();
            HRESULT hr = g_pDisplay->CreateSurfaceFromBitmap(pSurface,
                                                             MAKEINTRESOURCE(rgBitmapIds[Bitmap]));
            if (SUCCEEDED(hr))
                s_rgpCornerSurfaces[Bitmap] = pSurface;
            else
                delete pSurface;
        }
        if (NULL == s_rgpSideSurfaces[Bitmap])
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
ValidateWindowPolicy_t::
Validate(
    const SsTrades_t::AcquireData_t* pData)
{
    CSurface* pSurface = pData->pPoolItem->get();
    RECT rcSurface;
    ::GetClientRect(LonWindow_t::GetTopWindow().hWnd, &rcSurface);
    if (!ValidateSides(pSurface, rcSurface))
    {
        LogInfo(L"ValidateWindowPolicy_t::ValidateSides() failed.");
        return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool
ValidateWindowPolicy_t::
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
ValidateWindowPolicy_t::
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
