//-----------------------------------------------------------------------------
// File: ddutil.h
//
// Desc: Routines for loading bitmap and palettes from resources
//
// Copyright (C) 1998-1999 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _DDUTIL_H
#define _DDUTIL_H

#include <ddraw.h>
#include <d3d9.h>
#include "Rect.h"

//-----------------------------------------------------------------------------
// Miscellaneous helper functions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Classes defined in this header file 
//-----------------------------------------------------------------------------
class CDisplay;
class CSurface;




//-----------------------------------------------------------------------------
// Flags for the CDisplay and CSurface methods
//-----------------------------------------------------------------------------
#define DSURFACELOCK_READ
#define DSURFACELOCK_WRITE




//-----------------------------------------------------------------------------
// Name: class CDisplay
// Desc: Class to handle all DDraw aspects of a display, including creation of
//         front and back buffers, creating offscreen surfaces and palettes,
//         and blitting surface and displaying bitmaps.
//-----------------------------------------------------------------------------
class CDisplay
{
protected:
    LPDIRECTDRAW7         m_pDD;
    LPDIRECTDRAWSURFACE7 m_pddsFrontBuffer;
    LPDIRECTDRAWSURFACE7 m_pddsBackBuffer;
    LPDIRECTDRAWSURFACE7 m_pddsBackBufferLeft; // For stereo modes

    HWND                 m_hWnd;
    RECT                 m_rcWindow;
    BOOL                 m_bWindowed;
    BOOL                 m_bStereo;

public:
    CDisplay();
    ~CDisplay();

    // Access functions
    HWND                 GetHWnd()             { return m_hWnd; }
    LPDIRECTDRAW7         GetDirectDraw()     { return m_pDD; }
    LPDIRECTDRAWSURFACE7 GetFrontBuffer()     { return m_pddsFrontBuffer; }
    LPDIRECTDRAWSURFACE7 GetBackBuffer()     { return m_pddsBackBuffer; }
    LPDIRECTDRAWSURFACE7 GetBackBufferLEft() { return m_pddsBackBufferLeft; }

    // Status functions
    BOOL    IsWindowed()                     { return m_bWindowed; }
    BOOL    IsStereo()                         { return m_bStereo; }

    // Creation/destruction methods
    HRESULT CreateFullScreenDisplay( HWND hWnd, DWORD dwWidth, DWORD dwHeight,
                                     DWORD dwBPP );
    HRESULT CreateWindowedDisplay( HWND hWnd, DWORD dwWidth, DWORD dwHeight, bool bNormal=true );
    HRESULT InitClipper();
    HRESULT UpdateBounds();
    virtual HRESULT DestroyObjects();

    // Methods to create child objects
    HRESULT CreateSurface( CSurface* pSurface, DWORD dwWidth, DWORD dwHeight );
//    HRESULT CreateSurface( CSurface** ppSurface, DWORD dwWidth, DWORD dwHeight );

    HRESULT
    CreateSurfaceFromBitmap(
        CSurface* pSurface,
        LPCTSTR strBMP,
        DWORD dwWidth=0,
        DWORD dwHeight=0 );

HRESULT
CreateSurfaceFromResource(HINSTANCE hInst, UINT resourceId);

//    HRESULT CreateSurfaceFromBitmap(
//        CSurface** ppSurface, LPCTSTR strBMP, DWORD dwWidth, DWORD dwHeight );

//    HRESULT CreateSurfaceFromText( CSurface** ppSurface, HFONT hFont, TCHAR* strText, 
//                                   COLORREF crBackground, COLORREF crForeground );
    HRESULT CreatePaletteFromBitmap( LPDIRECTDRAWPALETTE* ppPalette, const TCHAR* strBMP );

    // Display methods
    HRESULT Clear( DWORD dwColor = 0L );
    HRESULT ColorKeyBlt( DWORD x, DWORD y, LPDIRECTDRAWSURFACE7 pdds,
                         RECT* prc = nullptr );
    HRESULT Blt( DWORD x, DWORD y, LPDIRECTDRAWSURFACE7 pdds,
                 RECT* prc=nullptr, DWORD dwFlags=0 );
    HRESULT Blt( DWORD x, DWORD y, CSurface* pSurface, RECT* prc = nullptr );
    HRESULT ShowBitmap( HBITMAP hbm, LPDIRECTDRAWPALETTE pPalette=nullptr );
    HRESULT SetPalette( LPDIRECTDRAWPALETTE pPalette );
    HRESULT Present();
};

#define COMPARE_F_TRANSPARENT          (1<<0)        // both src & dest pixels must be same "transparent" color
#define COMPARE_F_SHOWDWORD            (1<<1)
#define COMPARE_F_NOTSRCTRANSPARENT    (1<<3)        // don't check transparent source pixels
#define COMPARE_F_SAMECOLOR            (1<<4)
#define COMPARE_F_NOTDSTTRANSPARENT    (1<<5)        // don't check transparent destination pixels

//-----------------------------------------------------------------------------
// Name: class CSurface
// Desc: Class to handle aspects of a DirectDrawSurface.
//-----------------------------------------------------------------------------
class CSurface
{
private:

    LPDIRECTDRAWSURFACE7  m_pdds;
    DDSURFACEDESC2        m_ddsd;
    DDPIXELFORMAT         m_pf;
    DWORD                 m_dwUserFlags;
    Rect_t                m_rcLastBlt;
    BOOL                  m_bColorKeyed;

public:

    enum
    {
        Left,
        Top,
        Right,
        Bottom
    };

public:
    CSurface();
    ~CSurface();

    const DDPIXELFORMAT& GetPixelFormat() const { return m_pf; }

    LPDIRECTDRAWSURFACE7 GetDDrawSurface() { return m_pdds; }
    BOOL                 IsColorKeyed()       { return m_bColorKeyed; }

    HRESULT DrawBitmap( HBITMAP hBMP, DWORD dwBMPOriginX = 0, DWORD dwBMPOriginY = 0, 
                        DWORD dwBMPWidth = 0, DWORD dwBMPHeight = 0 );
    HRESULT DrawBitmap( TCHAR* strBMP, DWORD dwDesiredWidth, DWORD dwDesiredHeight );
    HRESULT DrawText( HFONT hFont, LPCTSTR strText, DWORD dwOriginX, DWORD dwOriginY,
                      COLORREF crBackground, COLORREF crForeground );

    HRESULT BltWindow( HWND hWnd, const RECT *pRect=0 );

    HRESULT SetColorKey( DWORD dwColorKey );
    DWORD    ConvertGDIColor( COLORREF dwGDIColor );
    static HRESULT GetBitMaskInfo( DWORD dwBitMask, DWORD* pdwShift, DWORD* pdwBits );

    HRESULT Create( LPDIRECTDRAW7 pDD, DDSURFACEDESC2* pddsd );
    HRESULT Create( LPDIRECTDRAWSURFACE7 pdds );
    HRESULT Destroy();

    void                     GetPixelFormat(DDPIXELFORMAT& pf) const;
    DWORD                    GetWidth( void ) const            { return m_ddsd.dwWidth; }
    DWORD                    GetHeight( void ) const            { return m_ddsd.dwHeight; }

    HRESULT                    Lock( DDSURFACEDESC2 *pDDSD, RECT *pRect=0 ) const;
    HRESULT                    Unlock( RECT *pRect=0 ) const;

    HRESULT                    Blt( DWORD x, DWORD y, LPDIRECTDRAWSURFACE7 pdds,
                                 RECT* prc=nullptr, DWORD dwFlags=DDBLTFAST_NOCOLORKEY );
    HRESULT                    Blt( DWORD x, DWORD y, CSurface * pSurface, RECT* prc = 0 );

    HRESULT                    ColorFill( const RECT *pRect, COLORREF clr );

    bool                    Compare(DWORD dwDstX, DWORD dwDstY,
                                     const CSurface *pSrcSurface, DWORD dwSrcX, DWORD dwSrcY,
                                     DWORD dwWidth, DWORD dwHeight, DWORD dwFlags=0 ) const;
    bool                    Compare(DWORD dstX, DWORD dstY, const CSurface& surface) const;
    bool                    Compare(const CSurface& Surface, const Rect_t& Rect) const;


    bool                    CompareColor( const RECT& rc, COLORREF clr, DWORD dwFlags=0 ) const;
    bool                    CompareColorRange( const RECT& rc, COLORREF clrLow, COLORREF clrHigh, DWORD dwFlags=0 ) const;
    bool                    CompareColor( const POINT& pt, const SIZE& size, COLORREF clr, DWORD dwFlags=0 ) const;

    size_t                    FixColor(const RECT& rc, COLORREF low, COLORREF hi, COLORREF set);

    HRESULT                    WriteBMP( LPCTSTR pszFilename) const;
    HRESULT                    WriteBMP( LPCTSTR pszFilename, const RECT& rc ) const;

    HDC                        GetDC();
    void                    ReleaseDC( HDC hDC );

    void                    SetUserFlags( DWORD dwFlags )    { m_dwUserFlags = dwFlags; }
    DWORD                    GetUserFlags( void ) const        { return m_dwUserFlags; }

    HRESULT                    FindPixel( const POINT& pt, const SIZE& size, COLORREF clr,

                                       int iDirection, POINT& ptResult ) const;
    HRESULT                    FindPixel( const RECT *pRect, COLORREF clr, int iDirection,
                                       POINT& ptResult ) const;

    void                    GetClientRect(RECT* pRect) const;
    const Rect_t& GetBltRect() const;

    COLORREF GetPixel(int x, int y) const;

    bool
    FindSurfaceInRect(
        const CSurface& surface,
        const Rect_t&   findRect,
              POINT&    ptOrigin,
        const POINT*    pptHint = nullptr) const;

    size_t
    GetWidthInColorRange(
        int x, int y,
        COLORREF low,
        COLORREF hi,
        DWORD    Flags = 0) const;

    size_t
    GetIntensityCount(const Rect_t& Rect, size_t Intensity) const;

    bool
    CompareColorIntensityFunc(COLORREF color1, COLORREF color2) const;
};


void *                GetBitsAt( const DDSURFACEDESC2 *pDDSD, int x, int y );

#if 0
DWORD GetMaskValue(DWORD Pix, DWORD Mask);
#else
#define GetShiftBits2(Mask) (((Mask) == 0xff00) ? 8 : 16)
#define GetShiftBits(Mask)  (((Mask) == 0xff) ? 0 : GetShiftBits2(Mask))
#define GetMaskValue(Pix, Mask)  ((Pix & Mask) >> GetShiftBits(Mask))
#endif

//#define CompareColorMask(mask, color1, color2)        
//#define CompareBits2(mask, color1, color2)       
#define CompareBlue(pf, color1, color2)           (GetMaskValue(color1, pf.dwBBitMask) <= GetMaskValue(color2, pf.dwBBitMask))
#define CompareGreen(pf, color1, color2)          (GetMaskValue(color1, pf.dwGBitMask) <= GetMaskValue(color2, pf.dwGBitMask))
#define CompareRed(pf, color1, color2)            (GetMaskValue(color1, pf.dwRBitMask) <= GetMaskValue(color2, pf.dwRBitMask))
#define CompareColorIntensity(pf, color1, color2) (CompareRed(pf, color1, color2) && CompareGreen(pf, color1, color2) && CompareBlue(pf, color1, color2))
#define IsTransparentPixel(pf, pix, maxTransColor)(CompareColorIntensity(pf, pix, maxTransColor))

#if 0
{
    const DWORD red1   = GetMaskValue(color1, m_pf.dwRBitMask);
    const DWORD red2   = GetMaskValue(color2, m_pf.dwRBitMask);
    if (red1 >= red2) return false;
    const DWORD green1 = GetMaskValue(color1, m_pf.dwGBitMask);
    const DWORD green2 = GetMaskValue(color2, m_pf.dwGBitMask);
    if (green1 >= green2) return false;
    const DWORD blue1  = GetMaskValue(color1, m_pf.dwBBitMask);
    const DWORD blue2  = GetMaskValue(color2, m_pf.dwBBitMask);
    return (blue1 < blue2);
}
#endif


#endif // _DDUTIL_H

