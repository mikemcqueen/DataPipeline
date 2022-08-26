/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TrScrollThumb_t.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TRSCROLLTHUMB_T_H_
#define Include_TRSCROLLTHUMB_T_H_

#include "TrSurface_t.h"
#include "LonTypes.h"

/////////////////////////////////////////////////////////////////////////////

// or just put ddraw.h in pch
struct _DDSURFACEDESC2;
typedef struct _DDSURFACEDESC2 DDSURFACEDESC2;

class TrScrollThumb_t :
    public TrSurface_t
{
public:

private:

    Lon::Window::Type_e m_WindowType;
    Lon::ScrollBar_t    m_Horz;
    Lon::ScrollBar_t    m_Vert;

public:

    TrScrollThumb_t();
	virtual ~TrScrollThumb_t();

    //
    // DP::Handler_t virtual:
    //

    HRESULT
    MessageHandler(
        const DP::Message::Data_t* pMessage) override;

    //
    // TrSurface_t virtual:
    //

    bool
    IsSupportedWindowType(
        Lon::Window::Type_e WindowType) const override;

    void
    PostData(
        DWORD AcquireId) override;

private:

    void
    InitScrollPosition(
              Lon::ScrollBar_t& ScrollBar,
              Lon::Window::Type_e   WindowType,
        const CSurface*        pSurface);

    void
    GetScrollRect(
              Lon::Window::Type_e          WindowType,
        const Lon::ScrollBar_t::Type_e ScrollType,
              RECT&                   Rect) const;

    Lon::Window::Type_e
    GetScrollWindow(
        Lon::Window::Type_e          WindowType,
        Lon::ScrollBar_t::Type_e ScrollType) const;


    Lon::ScrollBar_t::ThumbPosition_e
    GetScrollPosition(
              Lon::ScrollBar_t::Type_e ScrollType,
        const CSurface* pSurface,
              RECT&     rcSurface) const;

    void
    GetTopPoint(
              Lon::ScrollBar_t::Type_e ScrollType,
        const RECT&                   Rect,
              POINT&                  Point) const;

    void
    GetBottomPoint(
              Lon::ScrollBar_t::Type_e ScrollType,
        const RECT&                   Rect,
              POINT&                  Point) const;

    size_t
    GetSameColorPixelCount(
        const DDSURFACEDESC2& ddsd,
              POINT           StartPoint,
              size_t          PixelCount,
              size_t          PixelIncrement) const;

private:

    TrScrollThumb_t(const TrScrollThumb_t&);
    TrScrollThumb_t& operator=(const TrScrollThumb_t&);
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TrScrollThumb_t_H

/////////////////////////////////////////////////////////////////////////////

