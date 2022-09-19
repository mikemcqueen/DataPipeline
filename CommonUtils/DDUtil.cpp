//-----------------------------------------------------------------------------
// File: ddutil.cpp
//
// Desc: DirectDraw framewark classes. Feel free to use this class as a 
//		 starting point for adding extra functionality.
//
//
// Copyright (c) 1995-1999 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "DDUtil.h"
#include "Macros.h"
#include "Log.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#endif

//-----------------------------------------------------------------------------
// Name: CDisplay()
// Desc:
//-----------------------------------------------------------------------------
CDisplay::CDisplay()
{
	m_pDD				 = nullptr;
	m_pddsFrontBuffer	 = nullptr;
	m_pddsBackBuffer	 = nullptr;
	m_pddsBackBufferLeft = nullptr;
}




//-----------------------------------------------------------------------------
// Name: ~CDisplay()
// Desc:
//-----------------------------------------------------------------------------
CDisplay::~CDisplay()
{
	DestroyObjects();
}




//-----------------------------------------------------------------------------
// Name: DestroyObjects()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CDisplay::DestroyObjects()
{
	SAFE_RELEASE( m_pddsBackBufferLeft );
	SAFE_RELEASE( m_pddsBackBuffer );
	SAFE_RELEASE( m_pddsFrontBuffer );

	if( m_pDD )
		m_pDD->SetCooperativeLevel( m_hWnd, DDSCL_NORMAL );

	SAFE_RELEASE( m_pDD );

	return S_OK;
}


#if 0
DWORD
GetMaskValue(DWORD Pix, DWORD Mask)
{
    Pix &= Mask;
    switch(Mask)
    {
    case 0x000000ff: return Pix;
    case 0x0000ff00: return Pix >> 8;
    case 0x00ff0000: return Pix >> 16;
    default:         
        throw std::logic_error("DdUtil::GetMaskValue");
    }
}
#endif



//-----------------------------------------------------------------------------
// Name: CreateFullScreenDisplay()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CDisplay::CreateFullScreenDisplay( HWND hWnd, DWORD dwWidth,
										   DWORD dwHeight, DWORD dwBPP )
{
	HRESULT hr;

	// Cleanup anything from a previous call
	DestroyObjects();

	// DDraw stuff begins here
	if( FAILED( hr = DirectDrawCreateEx( nullptr, (VOID**)&m_pDD,
										 IID_IDirectDraw7, nullptr ) ) )
		return E_FAIL;

	// Set cooperative level
	hr = m_pDD->SetCooperativeLevel( hWnd, DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN );
	if( FAILED(hr) )
		return E_FAIL;

	// Set the display mode
	if( FAILED( m_pDD->SetDisplayMode( dwWidth, dwHeight, dwBPP, 0, 0 ) ) )
		return E_FAIL;

	// Create primary surface (with backbuffer attached)
	DDSURFACEDESC2 ddsd;
	ZeroMemory( &ddsd, sizeof( ddsd ) );
	ddsd.dwSize			   = sizeof( ddsd );
	ddsd.dwFlags		   = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps	   = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP |
							 DDSCAPS_COMPLEX | DDSCAPS_3DDEVICE;
	ddsd.dwBackBufferCount = 1;

	if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &m_pddsFrontBuffer,
										   nullptr ) ) )
		return E_FAIL;

	// Get a pointer to the back buffer
	DDSCAPS2 ddscaps;
	ZeroMemory( &ddscaps, sizeof( ddscaps ) );
	ddscaps.dwCaps = DDSCAPS_BACKBUFFER;

	if( FAILED( hr = m_pddsFrontBuffer->GetAttachedSurface( &ddscaps,
															&m_pddsBackBuffer ) ) )
		return E_FAIL;

	m_pddsBackBuffer->AddRef();

	m_hWnd		= hWnd;
	m_bWindowed = FALSE;
	UpdateBounds();

	return S_OK;
}
	



//-----------------------------------------------------------------------------
// Name: CreateWindowedDisplay()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CDisplay::CreateWindowedDisplay( HWND hWnd, DWORD dwWidth, DWORD dwHeight, bool bNormal /*=true*/ )
{
	HRESULT hr;

	// Cleanup anything from a previous call
	DestroyObjects();

	// DDraw stuff begins here
	if( FAILED( hr = DirectDrawCreateEx( nullptr, (VOID**)&m_pDD,
										 IID_IDirectDraw7, nullptr ) ) )
		return E_FAIL;

	// Set cooperative level
	hr = m_pDD->SetCooperativeLevel( hWnd, DDSCL_NORMAL );
	if( FAILED(hr) )
		return E_FAIL;

if( bNormal )
{
	RECT  rcWork;
	RECT  rc;
	DWORD dwStyle;

	// If we are still a WS_POPUP window we should convert to a normal app
	// window so we look like a windows app.
	dwStyle	 = GetWindowStyle( hWnd );
	dwStyle &= ~WS_POPUP;
	dwStyle |= WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX;
	SetWindowLong( hWnd, GWL_STYLE, dwStyle );

	// Aet window size
	SetRect( &rc, 0, 0, dwWidth, dwHeight );

	AdjustWindowRectEx( &rc, GetWindowStyle(hWnd), GetMenu(hWnd) != nullptr,
						GetWindowExStyle(hWnd) );

	SetWindowPos( hWnd, nullptr, 0, 0, rc.right-rc.left, rc.bottom-rc.top,
				  SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );

	SetWindowPos( hWnd, HWND_NOTOPMOST, 0, 0, 0, 0,
				  SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE );

	//	Make sure our window does not hang outside of the work area
	SystemParametersInfo( SPI_GETWORKAREA, 0, &rcWork, 0 );
	GetWindowRect( hWnd, &rc );
	if( rc.left < rcWork.left ) rc.left = rcWork.left;
	if( rc.top	< rcWork.top )	rc.top	= rcWork.top;
	SetWindowPos( hWnd, nullptr, rc.left, rc.top, 0, 0,
				  SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

	LPDIRECTDRAWCLIPPER pcClipper;
	
	// Create the primary surface
	DDSURFACEDESC2 ddsd;
	ZeroMemory( &ddsd, sizeof( ddsd ) );
	ddsd.dwSize			= sizeof( ddsd );
	ddsd.dwFlags		= DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	if( FAILED( m_pDD->CreateSurface( &ddsd, &m_pddsFrontBuffer, nullptr ) ) )
		return E_FAIL;

	// Create the backbuffer surface
	ddsd.dwFlags		= DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;	   
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
	ddsd.dwWidth		= dwWidth;
	ddsd.dwHeight		= dwHeight;

	if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &m_pddsBackBuffer, nullptr ) ) )
		return E_FAIL;

	if( FAILED( hr = m_pDD->CreateClipper( 0, &pcClipper, nullptr ) ) )
		return E_FAIL;

	if( FAILED( hr = pcClipper->SetHWnd( 0, hWnd ) ) )
	{
		pcClipper->Release();
		return E_FAIL;
	}

	if( FAILED( hr = m_pddsFrontBuffer->SetClipper( pcClipper ) ) )
	{
		pcClipper->Release();
		return E_FAIL;
	}

	// Done with clipper
	pcClipper->Release();
}

	m_hWnd		= hWnd;
	m_bWindowed = TRUE;
	UpdateBounds();

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

/*
HRESULT
CDisplay::CreateSurface( CSurface** ppSurface, DWORD dwWidth, DWORD dwHeight )
{
	if( !m_pDD )
		return E_POINTER;
	if( !ppSurface )
		return E_INVALIDARG;

	*ppSurface = new CSurface();
	HRESULT hr = CreateSurface( *ppSurface, dwWidth, dwHeight );
	if( FAILED(hr) )
		delete *ppSurface;

	return hr;
}
*/

/////////////////////////////////////////////////////////////////////////////

HRESULT
CDisplay::CreateSurface( CSurface *pSurface, DWORD dwWidth, DWORD dwHeight )
{
	if( !m_pDD )
		return E_POINTER;
	if( !pSurface )
		return E_INVALIDARG;

	DDSURFACEDESC2 ddsd;
	ZeroMemory( &ddsd, sizeof( ddsd ) );
	ddsd.dwSize			= sizeof( ddsd );
	ddsd.dwFlags		= DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT; 
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	ddsd.dwWidth		= dwWidth;
	ddsd.dwHeight		= dwHeight;

	return pSurface->Create( m_pDD, &ddsd );
}

/////////////////////////////////////////////////////////////////////////////
//
// Name: CDisplay::CreateSurfaceFromBitmap()
// Desc: Create a DirectDrawSurface from a bitmap resource or bitmap file.
//		 Use MAKEINTRESOURCE() to pass a constant into strBMP.
//

#if 0
HRESULT
CDisplay::CreateSurfaceFromBitmap
(
CSurface **	ppSurface,
LPCTSTR		strBMP,											   
DWORD		dwDesiredWidth, 
DWORD		dwDesiredHeight
)
{
	HRESULT		   hr;
	HBITMAP		   hBMP = nullptr;
	BITMAP		   bmp;

	if( m_pDD == nullptr || strBMP == nullptr || ppSurface == nullptr ) 
		return E_INVALIDARG;

	*ppSurface = 0;

	//	Try to load the bitmap as a resource, if that fails, try it as a file
	hBMP = (HBITMAP) LoadImage( GetModuleHandle(nullptr), strBMP, 
								IMAGE_BITMAP, dwDesiredWidth, dwDesiredHeight, 
								LR_CREATEDIBSECTION );
	if( hBMP == nullptr )
	{
		hBMP = (HBITMAP) LoadImage( nullptr, strBMP, 
									IMAGE_BITMAP, dwDesiredWidth, dwDesiredHeight, 
									LR_LOADFROMFILE | LR_CREATEDIBSECTION );
		if( hBMP == nullptr )
			return E_FAIL;
	}

	// Get size of the bitmap
	GetObject( hBMP, sizeof(bmp), &bmp );

#if 0
	// Create a DirectDrawSurface for this bitmap
	DDSURFACEDESC2 ddsd;
	ZeroMemory( &ddsd, sizeof(ddsd) );
	ddsd.dwSize			= sizeof(ddsd);
	ddsd.dwFlags		= DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	ddsd.dwWidth		= bmp.bmWidth;
	ddsd.dwHeight		= bmp.bmHeight;

	(*ppSurface) = new CSurface();
	if( FAILED( hr = (*ppSurface)->Create( m_pDD, &ddsd ) ) )
	{
		delete (*ppSurface);
		return hr;
	}
#else
	hr = CreateSurface( *ppSurface, bmp.bmWidth, bmp.bmHeight );
	if( FAILED(hr) )
    {
		DeleteObject( hBMP );
		return hr;
    }
#endif

	// Draw the bitmap on this surface
	if( FAILED( hr = (*ppSurface)->DrawBitmap( hBMP, 0, 0, 0, 0 ) ) )
	{
		DeleteObject( hBMP );
		return hr;
	}

	DeleteObject( hBMP );

	return S_OK;
}
#endif


/////////////////////////////////////////////////////////////////////////////
//
// Name: CDisplay::CreateSurfaceFromBitmap()
// Desc: Create a DirectDrawSurface from a bitmap resource or bitmap file.
//		 Use MAKEINTRESOURCE() to pass a constant into strBMP.
//

HRESULT
CDisplay::CreateSurfaceFromBitmap
(
CSurface *	pSurface,
LPCTSTR		strBMP,											   
DWORD		dwDesiredWidth, 
DWORD		dwDesiredHeight
)
{
	HRESULT		   hr;
	HBITMAP		   hBMP = nullptr;
	BITMAP		   bmp;

	if( m_pDD == nullptr || strBMP == nullptr || pSurface == nullptr ) 
		return E_INVALIDARG;

	//	Try to load the bitmap as a resource, if that fails, try it as a file
	hBMP = (HBITMAP) LoadImage( GetModuleHandle(nullptr), strBMP, 
								IMAGE_BITMAP, dwDesiredWidth, dwDesiredHeight, 
								LR_CREATEDIBSECTION );
	if( hBMP == nullptr )
	{
		hBMP = (HBITMAP) LoadImage( nullptr, strBMP, 
									IMAGE_BITMAP, dwDesiredWidth, dwDesiredHeight, 
									LR_LOADFROMFILE | LR_CREATEDIBSECTION );
		if( hBMP == nullptr )
			return E_FAIL;
	}

	// Get size of the bitmap
	GetObject( hBMP, sizeof(bmp), &bmp );

	hr = CreateSurface( pSurface, bmp.bmWidth, bmp.bmHeight );
	if( FAILED(hr) )
    {
        DeleteObject(hBMP);
		return hr;
    }

	// Draw the bitmap on this surface
	if( FAILED( hr = pSurface->DrawBitmap( hBMP, 0, 0, 0, 0 ) ) )
	{
		DeleteObject( hBMP );
		return hr;
	}

	DeleteObject( hBMP );

	return S_OK;
}


#if 0

//-----------------------------------------------------------------------------
// Name: CDisplay::CreateSurfaceFromText()
// Desc: Creates a DirectDrawSurface from a text string using hFont or the default 
//		 GDI font if hFont is nullptr.
//-----------------------------------------------------------------------------
HRESULT CDisplay::CreateSurfaceFromText( CSurface** ppSurface,
										 HFONT hFont, wchar_t* strText, 
										 COLORREF crBackground, COLORREF crForeground )
{
	HDC					 hDC  = nullptr;
	HRESULT				 hr;
	DDSURFACEDESC2		 ddsd;
	SIZE				 sizeText;

	if( m_pDD == nullptr || strText == nullptr || ppSurface == nullptr )
		return E_INVALIDARG;

	*ppSurface = nullptr;

	hDC = GetDC( nullptr );

	HFONT hOldFont = 0;
	if( hFont )
		hOldFont = (HFONT)SelectObject( hDC, hFont );

	GetTextExtentPoint32( hDC, strText, int(wcslen(strText)), &sizeText );

	if( hOldFont )
		SelectObject( hDC, hOldFont );
	ReleaseDC( nullptr, hDC );

	// Create a DirectDrawSurface for this bitmap
	ZeroMemory( &ddsd, sizeof(ddsd) );
	ddsd.dwSize			= sizeof(ddsd);
	ddsd.dwFlags		= DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	ddsd.dwWidth		= sizeText.cx;
	ddsd.dwHeight		= sizeText.cy;

	(*ppSurface) = new CSurface();
	if( FAILED( hr = (*ppSurface)->Create( m_pDD, &ddsd ) ) )
	{
		delete (*ppSurface);
		return hr;
	}

	if( FAILED( hr = (*ppSurface)->DrawText( hFont, strText, 0, 0, 
											 crBackground, crForeground ) ) )
		return hr;

	return S_OK;
}



#endif
//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CDisplay::Present()
{
	HRESULT hr;

	if( nullptr == m_pddsFrontBuffer && nullptr == m_pddsBackBuffer )
		return E_POINTER;

	for (;;)
	{
		if( m_bWindowed )
			hr = m_pddsFrontBuffer->Blt( &m_rcWindow, m_pddsBackBuffer,
										 nullptr, DDBLT_WAIT, nullptr );
		else
			hr = m_pddsFrontBuffer->Flip( nullptr, 0 );

		if( hr == DDERR_SURFACELOST )
		{
			m_pddsFrontBuffer->Restore();
			m_pddsBackBuffer->Restore();
		}

		if( hr != DDERR_WASSTILLDRAWING )
			return hr;
	}
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CDisplay::ShowBitmap( HBITMAP hbm, LPDIRECTDRAWPALETTE pPalette )
{
	if( nullptr == m_pddsFrontBuffer ||  nullptr == m_pddsBackBuffer )
		return E_POINTER;

	// Set the palette before loading the bitmap
	if( pPalette )
		m_pddsFrontBuffer->SetPalette( pPalette );

	CSurface backBuffer;
	backBuffer.Create( m_pddsBackBuffer );

	if( FAILED( backBuffer.DrawBitmap( hbm, 0, 0, 0, 0 ) ) )
		return E_FAIL;

	return Present();
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CDisplay::ColorKeyBlt( DWORD x, DWORD y, LPDIRECTDRAWSURFACE7 pdds,
							   RECT* prc )
{
	if( nullptr == m_pddsBackBuffer )
		return E_POINTER;

	return m_pddsBackBuffer->BltFast( x, y, pdds, prc, DDBLTFAST_SRCCOLORKEY );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CDisplay::Blt( DWORD x, DWORD y, LPDIRECTDRAWSURFACE7 pdds, RECT* prc,
					   DWORD dwFlags )
{
	if( nullptr == m_pddsBackBuffer )
		return E_POINTER;

	return m_pddsBackBuffer->BltFast( x, y, pdds, prc, dwFlags );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CDisplay::Blt( DWORD x, DWORD y, CSurface* pSurface, RECT* prc )
{
	if( nullptr == pSurface )
		return E_INVALIDARG;

	if( pSurface->IsColorKeyed() )
		return Blt( x, y, pSurface->GetDDrawSurface(), prc, DDBLTFAST_SRCCOLORKEY );
	else
		return Blt( x, y, pSurface->GetDDrawSurface(), prc, 0L );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CDisplay::Clear( DWORD dwColor )
{
	if( nullptr == m_pddsBackBuffer )
		return E_POINTER;

	// Erase the background
	DDBLTFX ddbltfx;
	ZeroMemory( &ddbltfx, sizeof(ddbltfx) );
	ddbltfx.dwSize		= sizeof(ddbltfx);
	ddbltfx.dwFillColor = dwColor;

	return m_pddsBackBuffer->Blt( nullptr, nullptr, nullptr, DDBLT_COLORFILL, &ddbltfx );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CDisplay::SetPalette( LPDIRECTDRAWPALETTE pPalette )
{
	if( nullptr == m_pddsFrontBuffer )
		return E_POINTER;

	return m_pddsFrontBuffer->SetPalette( pPalette );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CDisplay::CreatePaletteFromBitmap( LPDIRECTDRAWPALETTE* ppPalette,
										   const TCHAR* strBMP )
{
	HRSRC			  hResource		 = nullptr;
	RGBQUAD*		  pRGB			 = nullptr;
	BITMAPINFOHEADER* pbi = nullptr;
	PALETTEENTRY	  aPalette[256];
	HANDLE			  hFile = nullptr;
	DWORD			  iColor;
	DWORD			  dwColors;
	BITMAPFILEHEADER  bf;
	BITMAPINFOHEADER  bi;
	DWORD			  dwBytesRead;

	if( m_pDD == nullptr || strBMP == nullptr || ppPalette == nullptr )
		return E_INVALIDARG;

	*ppPalette = nullptr;

	//	Try to load the bitmap as a resource, if that fails, try it as a file
	hResource = FindResource( nullptr, strBMP, RT_BITMAP );
	if( hResource )
	{
		pbi = (LPBITMAPINFOHEADER) LockResource( LoadResource( nullptr, hResource ) );		  
		if( nullptr == pbi )
			return E_FAIL;

		pRGB = (RGBQUAD*) ( (BYTE*) pbi + pbi->biSize );

		// Figure out how many colors there are
		if( pbi == nullptr || pbi->biSize < sizeof(BITMAPINFOHEADER) )
			dwColors = 0;
		else if( pbi->biBitCount > 8 )
			dwColors = 0;
		else if( pbi->biClrUsed == 0 )
			dwColors = 1 << pbi->biBitCount;
		else
			dwColors = pbi->biClrUsed;

		//	A DIB color table has its colors stored BGR not RGB
		//	so flip them around.
		for( iColor = 0; iColor < dwColors; iColor++ )
		{
			aPalette[iColor].peRed	 = pRGB[iColor].rgbRed;
			aPalette[iColor].peGreen = pRGB[iColor].rgbGreen;
			aPalette[iColor].peBlue	 = pRGB[iColor].rgbBlue;
			aPalette[iColor].peFlags = 0;
		}

		return m_pDD->CreatePalette( DDPCAPS_8BIT, aPalette, ppPalette, nullptr );
	}

	// Attempt to load bitmap as a file
	hFile = CreateFile( strBMP, GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr );
	if( nullptr == hFile )
		return E_FAIL;

	// Read the BITMAPFILEHEADER
	ReadFile( hFile, &bf, sizeof(bf), &dwBytesRead, nullptr );
	if( dwBytesRead != sizeof(bf) )
	{
		CloseHandle( hFile );
		return E_FAIL;
	}

	// Read the BITMAPINFOHEADER
	ReadFile( hFile, &bi, sizeof(bi), &dwBytesRead, nullptr );
	if( dwBytesRead != sizeof(bi) )
	{
		CloseHandle( hFile );
		return E_FAIL;
	}

	// Read the PALETTEENTRY 
	ReadFile( hFile, aPalette, sizeof(aPalette), &dwBytesRead, nullptr );
	if( dwBytesRead != sizeof(aPalette) )
	{
		CloseHandle( hFile );
		return E_FAIL;
	}

	CloseHandle( hFile );

	// Figure out how many colors there are
	if( bi.biSize != sizeof(BITMAPINFOHEADER) )
		dwColors = 0;
	else if (bi.biBitCount > 8)
		dwColors = 0;
	else if (bi.biClrUsed == 0)
		dwColors = 1 << bi.biBitCount;
	else
		dwColors = bi.biClrUsed;

	//	A DIB color table has its colors stored BGR not RGB
	//	so flip them around since DirectDraw uses RGB
	for( iColor = 0; iColor < dwColors; iColor++ )
	{
		BYTE r = aPalette[iColor].peRed;
		aPalette[iColor].peRed	= aPalette[iColor].peBlue;
		aPalette[iColor].peBlue = r;
	}

	return m_pDD->CreatePalette( DDPCAPS_8BIT, aPalette, ppPalette, nullptr );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CDisplay::UpdateBounds()
{
	if( m_bWindowed )
	{
		GetClientRect( m_hWnd, &m_rcWindow );
		ClientToScreen( m_hWnd, (POINT*)&m_rcWindow );
		ClientToScreen( m_hWnd, (POINT*)&m_rcWindow+1 );
	}
	else
	{
		SetRect( &m_rcWindow, 0, 0, GetSystemMetrics(SM_CXSCREEN),
				 GetSystemMetrics(SM_CYSCREEN) );
	}

	return S_OK;
}





//-----------------------------------------------------------------------------
// Name: CDisplay::InitClipper
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CDisplay::InitClipper()
{
	LPDIRECTDRAWCLIPPER pClipper;
	HRESULT hr;

	// Create a clipper when using GDI to draw on the primary surface 
	if( FAILED( hr = m_pDD->CreateClipper( 0, &pClipper, nullptr ) ) )
		return hr;

	pClipper->SetHWnd( 0, m_hWnd );

	if( FAILED( hr = m_pddsFrontBuffer->SetClipper( pClipper ) ) )
		return hr;

	// We can release the clipper now since g_pDDSPrimary 
	// now maintains a ref count on the clipper
	SAFE_RELEASE( pClipper );

	return S_OK;
}





//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
CSurface::CSurface()
{
	m_pdds = nullptr;
	m_bColorKeyed = NULL;
    ::SetRect(&m_rcLastBlt, 0, 0, 0, 0);
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
CSurface::~CSurface()
{
	SAFE_RELEASE( m_pdds );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CSurface::Create( LPDIRECTDRAWSURFACE7 pdds )
{
	m_pdds = pdds;

	if( m_pdds )
	{
		m_pdds->AddRef();

		// Get the DDSURFACEDESC structure for this surface
		m_ddsd.dwSize = sizeof(m_ddsd);
		m_pdds->GetSurfaceDesc(&m_ddsd );
        GetPixelFormat(m_pf);
	}

	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CSurface::Create( LPDIRECTDRAW7 pDD, DDSURFACEDESC2* pddsd )
{
	HRESULT hr;

	// Create the DDraw surface
	if( FAILED( hr = pDD->CreateSurface( pddsd, &m_pdds, nullptr ) ) )
		return hr;

	m_ddsd.dwSize = sizeof(m_ddsd);
	m_pdds->GetSurfaceDesc( &m_ddsd );
    GetPixelFormat(m_pf);

	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CSurface::Destroy()
{
	SAFE_RELEASE( m_pdds );
	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CSurface::DrawBitmap()
// Desc: Draws a bitmap over an entire DirectDrawSurface, stretching the 
//		 bitmap if nessasary
//-----------------------------------------------------------------------------
HRESULT CSurface::DrawBitmap( HBITMAP hBMP, 
							  DWORD dwBMPOriginX, DWORD dwBMPOriginY, 
							  DWORD dwBMPWidth, DWORD dwBMPHeight )
{
	HDC			   hDCImage;
	HDC			   hDC;
	BITMAP		   bmp;
	DDSURFACEDESC2 ddsd;
	HRESULT		   hr;

	if( hBMP == nullptr || m_pdds == nullptr )
		return E_INVALIDARG;

	// Make sure this surface is restored.
	if( FAILED( hr = m_pdds->Restore() ) )
		return hr;

	// Get the surface.description
	ddsd.dwSize	 = sizeof(ddsd);
	m_pdds->GetSurfaceDesc( &ddsd );

	if( ddsd.ddpfPixelFormat.dwFlags == DDPF_FOURCC )
		return E_NOTIMPL;

	// Select bitmap into a memoryDC so we can use it.
	hDCImage = CreateCompatibleDC( nullptr );
	if( nullptr == hDCImage )
		return E_FAIL;

	SelectObject( hDCImage, hBMP );

	// Get size of the bitmap
	GetObject( hBMP, sizeof(bmp), &bmp );

	// Use the passed size, unless zero
	dwBMPWidth	= ( dwBMPWidth	== 0 ) ? bmp.bmWidth  : dwBMPWidth;		
	dwBMPHeight = ( dwBMPHeight == 0 ) ? bmp.bmHeight : dwBMPHeight;

	// Stretch the bitmap to cover this surface
	if( FAILED( hr = m_pdds->GetDC( &hDC ) ) )
		return hr;

	StretchBlt( hDC, 0, 0, 
				ddsd.dwWidth, ddsd.dwHeight, 
				hDCImage, dwBMPOriginX, dwBMPOriginY,
				dwBMPWidth, dwBMPHeight, SRCCOPY );

	if( FAILED( hr = m_pdds->ReleaseDC( hDC ) ) )
		return hr;

	DeleteDC( hDCImage );

    ::SetRect(&m_rcLastBlt, 0, 0, dwBMPWidth, dwBMPHeight);

	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CSurface::DrawText()
// Desc: Draws a text string on a DirectDraw surface using hFont or the default
//		 GDI font if hFont is nullptr.	 
//-----------------------------------------------------------------------------
HRESULT CSurface::DrawText( HFONT hFont, LPCTSTR strText, 
							DWORD dwOriginX, DWORD dwOriginY,
							COLORREF crBackground, COLORREF crForeground )
{
	HDC		hDC = nullptr;
	HRESULT hr;

	if( m_pdds == nullptr || strText == nullptr )
		return E_INVALIDARG;

	// Make sure this surface is restored.
	if( FAILED( hr = m_pdds->Restore() ) )
		return hr;

	if( FAILED( hr = m_pdds->GetDC( &hDC ) ) )
		return hr;

	// Set the background and foreground color
	SetBkColor( hDC, crBackground );
	SetTextColor( hDC, crForeground );

	HFONT hOldFont = 0;
	if( hFont )
		hOldFont = (HFONT)SelectObject( hDC, hFont );

	// Use GDI to draw the text on the surface
#if 1
	ExtTextOut( hDC, dwOriginX, dwOriginY, 0, 0, strText, static_cast<UINT>(wcslen(strText)), 0 );
#else
	RECT rc;
	SetRect( &rc, dwOriginX, dwOriginY, GetWidth()-dwOriginX, GetHeight()-dwOriginY );
	::DrawText( hDC, strText, _tcslen(strText), &rc,
				DT_SINGLELINE|DT_NOCLIP|DT_EXTERNALLEADING|DT_NOPREFIX );
#endif

	if( hFont )
		SelectObject( hDC, hOldFont );

	if( FAILED( hr = m_pdds->ReleaseDC( hDC ) ) )
		return hr;

	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CSurface::ReDrawBitmapOnSurface()
// Desc: Load a bitmap from a file or resource into a DirectDraw surface.
//		 normaly used to re-load a surface after a restore.
//-----------------------------------------------------------------------------
HRESULT CSurface::DrawBitmap( TCHAR* strBMP, 
							  DWORD dwDesiredWidth, DWORD dwDesiredHeight  )
{
	HBITMAP hBMP;
	HRESULT hr;

	if( m_pdds == nullptr || strBMP == nullptr )
		return E_INVALIDARG;

	//	Try to load the bitmap as a resource, if that fails, try it as a file
	hBMP = (HBITMAP) LoadImage( GetModuleHandle(nullptr), strBMP, 
								IMAGE_BITMAP, dwDesiredWidth, dwDesiredHeight, 
								LR_CREATEDIBSECTION );
	if( hBMP == nullptr )
	{
		hBMP = (HBITMAP) LoadImage( nullptr, strBMP, IMAGE_BITMAP, 
									dwDesiredWidth, dwDesiredHeight, 
									LR_LOADFROMFILE | LR_CREATEDIBSECTION );
		if( hBMP == nullptr )
			return E_FAIL;
	}

	// Draw the bitmap on this surface
	if( FAILED( hr = DrawBitmap( hBMP, 0, 0, 0, 0 ) ) )
	{
		DeleteObject( hBMP );
		return hr;
	}

	DeleteObject( hBMP );

	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CSurface::SetColorKey( DWORD dwColorKey )
{
	if( nullptr == m_pdds )
		return E_POINTER;

	m_bColorKeyed = TRUE;

	DDCOLORKEY ddck;
	ddck.dwColorSpaceLowValue  = ConvertGDIColor( dwColorKey );
	ddck.dwColorSpaceHighValue = ConvertGDIColor( dwColorKey );
	
	return m_pdds->SetColorKey( DDCKEY_SRCBLT, &ddck );
}





//-----------------------------------------------------------------------------
// Name: CSurface::ConvertGDIColor()
// Desc: Converts a GDI color (0x00bbggrr) into the equivalent color on a 
//		 DirectDrawSurface using its pixel format.	
//-----------------------------------------------------------------------------
DWORD CSurface::ConvertGDIColor( COLORREF dwGDIColor )
{
	if( m_pdds == nullptr )
		return 0x00000000;

	COLORREF	   rgbT = 0;
	HDC			   hdc;
	DWORD		   dw = CLR_INVALID;
	DDSURFACEDESC2 ddsd;
	HRESULT		   hr;

	//	Use GDI SetPixel to color match for us
	if( dwGDIColor != CLR_INVALID && m_pdds->GetDC(&hdc) == DD_OK)
	{
        rgbT = ::GetPixel(hdc, 0, 0);		// Save current pixel value
		SetPixel(hdc, 0, 0, dwGDIColor);	   // Set our value
		m_pdds->ReleaseDC(hdc);
	}

	// Now lock the surface so we can read back the converted color
	ddsd.dwSize = sizeof(ddsd);
	hr = m_pdds->Lock( nullptr, &ddsd, DDLOCK_WAIT, nullptr );
	if( hr == DD_OK)
	{
		dw = *(DWORD *) ddsd.lpSurface; 
		if( ddsd.ddpfPixelFormat.dwRGBBitCount < 32 ) // Mask it to bpp
			dw &= ( 1 << ddsd.ddpfPixelFormat.dwRGBBitCount ) - 1;	
		m_pdds->Unlock(nullptr);
	}

	//	Now put the color that was there back.
	if( dwGDIColor != CLR_INVALID && m_pdds->GetDC(&hdc) == DD_OK )
	{
		SetPixel( hdc, 0, 0, rgbT );
		m_pdds->ReleaseDC(hdc);
	}
	
	return dw;	  
}




//-----------------------------------------------------------------------------
// Name: CSurface::GetBitMaskInfo()
// Desc: Returns the number of bits and the shift in the bit mask
//-----------------------------------------------------------------------------
HRESULT CSurface::GetBitMaskInfo( DWORD dwBitMask, DWORD* pdwShift, DWORD* pdwBits )
{
	DWORD dwShift = 0;
	DWORD dwBits  = 0; 

	if( pdwShift == nullptr || pdwBits == nullptr )
		return E_INVALIDARG;

	if( dwBitMask )
	{
		while( (dwBitMask & 1) == 0 )
		{
			dwShift++;
			dwBitMask >>= 1;
		}
	}

	while( (dwBitMask & 1) != 0 )
	{
		dwBits++;
		dwBitMask >>= 1;
	}

	*pdwShift = dwShift;
	*pdwBits  = dwBits;

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////

HRESULT CSurface::Lock( DDSURFACEDESC2 *pDDSD, RECT *pRect/*=0*/ ) const
{
	ASSERT( nullptr != pDDSD );
	pDDSD->dwSize = sizeof(DDSURFACEDESC2);
	return m_pdds->Lock( pRect, pDDSD, 0, 0 );
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CSurface::Unlock( RECT *pRect/*=0*/ ) const
{
	return m_pdds->Unlock( pRect );
}

/////////////////////////////////////////////////////////////////////////////
		
bool
CSurface::
Compare(
    const CSurface& Surface,
    const Rect_t&   Rect) const
{
    return Compare(Rect.left, Rect.top, &Surface, 0, 0, Rect.Width(), Rect.Height());
}

/////////////////////////////////////////////////////////////////////////////

bool 
CSurface::
Compare(
          DWORD     dstX,
          DWORD     dstY,
    const CSurface& surface) const
{
    return Compare(dstX, dstY, &surface, 0, 0, surface.GetWidth(), surface.GetHeight());
}

/////////////////////////////////////////////////////////////////////////////

bool
CSurface::Compare
(
DWORD			dwThisX,
DWORD			dwThisY,
const CSurface *pSurface,
DWORD			dwSurfaceX,
DWORD			dwSurfaceY,
DWORD			dwWidth,
DWORD			dwHeight,
DWORD			dwFlags /* =0 */
)
const
{
	bool bRet = false;
	bool bFail = false;

	DDSURFACEDESC2	ddsdThis;
	DDSURFACEDESC2	ddsdSurface;
	HRESULT hr;
	DWORD dwTransPixel = 0;

bool bCount = false;
size_t MismatchCount = 0;
size_t MatchCount = 0;

	hr = Lock( &ddsdThis );
	if( FAILED(hr) )
		goto fail;

	hr = pSurface->Lock( &ddsdSurface );
	if( FAILED(hr) )
		goto fail;

	if( dwFlags&(COMPARE_F_TRANSPARENT|COMPARE_F_NOTSRCTRANSPARENT|COMPARE_F_NOTDSTTRANSPARENT) )
	{
		dwTransPixel = *(DWORD *)GetBitsAt( &ddsdSurface, 0, 0 );
		dwTransPixel &= 0x00ffffff;
	}

	DWORD dwLine;
	for( dwLine=0; dwLine<dwHeight; ++dwLine )
	{
		DWORD *pThisBits = (DWORD *)GetBitsAt( &ddsdThis, dwThisX, dwThisY+dwLine );
		DWORD *pSurfaceBits = (DWORD *)GetBitsAt( &ddsdSurface, dwSurfaceX, dwSurfaceY+dwLine );

		DWORD dwPixel;
		for( dwPixel=0; dwPixel<dwWidth; ++dwPixel )
		{
			DWORD dwThisPixel = pThisBits[dwPixel] & 0x00ffffff;
			DWORD dwSurfacePixel = pSurfaceBits[dwPixel] & 0x00ffffff;

			if( dwFlags&COMPARE_F_TRANSPARENT )
			{
				bool bThisTrans = dwThisPixel == dwTransPixel;
				bool bSurfaceTrans = dwSurfacePixel == dwTransPixel;
				if( bThisTrans^bSurfaceTrans )
					goto fail;
				continue;
			}
			// don't check pixels which are transparent on source
			if( dwFlags&COMPARE_F_NOTSRCTRANSPARENT )
			{
				if( dwSurfacePixel==dwTransPixel )
					continue;
			}
			// don't check pixels which are transparent on destination
			if( dwFlags&COMPARE_F_NOTDSTTRANSPARENT )
			{
				if( dwThisPixel==dwTransPixel )
					continue;
			}

			if( dwThisPixel != dwSurfacePixel )
            {
                if (!bCount)
                    goto fail;
                bFail = true;
                ++MismatchCount;
            }
            else 
                ++MatchCount;
		}
	}
	bRet = !bFail;

fail:;
	pSurface->Unlock();
	Unlock();

    if (0 < MismatchCount)
        LogInfo(L"Mismatch=%d, Match=%d", MismatchCount, MatchCount);

	return bRet;
}

/////////////////////////////////////////////////////////////////////////////

bool
CSurface::CompareColor( const POINT& pt, const SIZE& size, COLORREF clr, DWORD dwFlags/*=0*/ ) const
{
	RECT rc;
	SetRect( &rc, pt.x, pt.y, pt.x+size.cx, pt.y+size.cy );
	return CompareColor( rc, clr, dwFlags );
}

/////////////////////////////////////////////////////////////////////////////

bool 
CSurface::CompareColor( const RECT& rc, COLORREF clr, DWORD dwFlags/*=0*/ ) const
{
	bool bRet = false;

	DDSURFACEDESC2	ddsd;
	HRESULT hr;
	hr = Lock( &ddsd );
	if( FAILED(hr) )
		return false;

	if( dwFlags&COMPARE_F_SAMECOLOR )
		clr = (*(DWORD *)GetBitsAt(&ddsd, rc.left, rc.top)) & 0x00ffffff;

	DWORD dwHeight = RECTHEIGHT( rc );
	DWORD dwWidth = RECTWIDTH( rc );
	for( DWORD dwLine=0; dwLine<dwHeight; ++dwLine )
	{
		DWORD *pThisBits = (DWORD *)GetBitsAt( &ddsd, rc.left, rc.top+dwLine );
		for( DWORD dwPixel=0; dwPixel<dwWidth; ++dwPixel )
		{
			COLORREF clrPixel = (COLORREF)(pThisBits[dwPixel] & 0x00ffffff);
			if( clrPixel!=clr )
				goto fail;
		}
	}
	bRet = true;

fail:;
	Unlock();
	return bRet;
}

/////////////////////////////////////////////////////////////////////////////

bool
CSurface::
CompareColorRange(
    const RECT& rc,
    COLORREF low,
    COLORREF hi,
    DWORD dwFlags) const
{
    dwFlags;
	bool bRet = false;

	DDSURFACEDESC2	ddsd;
	HRESULT hr;
	hr = Lock( &ddsd );
	if( FAILED(hr) )
		return false;

    const DWORD lowRed   = GetRValue(low);
    const DWORD lowGreen = GetGValue(low);
    const DWORD lowBlue  = GetBValue(low);
    const DWORD hiRed    = GetRValue(hi);
    const DWORD hiGreen  = GetGValue(hi);
    const DWORD hiBlue   = GetBValue(hi);

#if 0
    DDPIXELFORMAT pf;
    GetPixelFormat(pf);
#endif

	DWORD dwHeight = RECTHEIGHT( rc );
	DWORD dwWidth = RECTWIDTH( rc );
	for( DWORD dwLine=0; dwLine<dwHeight; ++dwLine )
	{
        const DWORD *pThisBits = (DWORD *)GetBitsAt( &ddsd, rc.left, rc.top+dwLine );
		for( DWORD dwPixel=0; dwPixel<dwWidth; ++dwPixel )
		{
            DWORD pix = pThisBits[dwPixel] & 0x00ffffff;
            const DWORD pixRed   = GetMaskValue(pix, m_pf.dwRBitMask);
            const DWORD pixGreen = GetMaskValue(pix, m_pf.dwGBitMask);
            const DWORD pixBlue  = GetMaskValue(pix, m_pf.dwBBitMask);
            if ((pixRed   < lowRed)   || (pixRed   > hiRed) ||
                (pixGreen < lowGreen) || (pixGreen > hiGreen) ||
                (pixBlue  < lowBlue)  || (pixBlue  > hiBlue))
            {
                goto fail;
            }
		}
	}
	bRet = true;

fail:;
	Unlock();
	return bRet;
}

/////////////////////////////////////////////////////////////////////////////

COLORREF
CSurface::GetPixel(int x, int y) const
{
	DDSURFACEDESC2	ddsd;
	HRESULT hr;
	hr = Lock( &ddsd );
	if( FAILED(hr) )
		return false;

    COLORREF clr = *(COLORREF*)GetBitsAt(&ddsd, x, y);

	Unlock();
    return clr;
}

/////////////////////////////////////////////////////////////////////////////

void
CSurface::
GetPixelFormat(DDPIXELFORMAT& pf) const
{
    ZeroMemory(&pf,sizeof(pf));
    pf.dwSize=sizeof(pf);
    m_pdds->GetPixelFormat(&pf);
}


size_t
CSurface::FixColor(const RECT& rc, COLORREF low, COLORREF hi, COLORREF set)
{
	DDSURFACEDESC2	ddsd;
	HRESULT hr;
	hr = Lock( &ddsd );
	if( FAILED(hr) )
		return false;

    DWORD lowRed   = GetRValue(low);
    DWORD lowGreen = GetGValue(low);
    DWORD lowBlue  = GetBValue(low);
    DWORD hiRed    = GetRValue(hi);
    DWORD hiGreen  = GetGValue(hi);
    DWORD hiBlue   = GetBValue(hi);

    size_t count = 0;
	DWORD dwHeight = RECTHEIGHT( rc );
	DWORD dwWidth = RECTWIDTH( rc );
	for( DWORD dwLine=0; dwLine<dwHeight; ++dwLine )
	{
		DWORD *pThisBits = (DWORD *)GetBitsAt( &ddsd, rc.left, rc.top+dwLine );
		for( DWORD dwPixel=0; dwPixel<dwWidth; ++dwPixel )
		{
			DWORD pix = pThisBits[dwPixel] & 0x00ffffff;
            DWORD pixRed   = GetMaskValue(pix, m_pf.dwRBitMask);
            DWORD pixGreen = GetMaskValue(pix, m_pf.dwGBitMask);
            DWORD pixBlue  = GetMaskValue(pix, m_pf.dwBBitMask);
            if ((pixRed   >= lowRed   && pixRed   <= hiRed) &&
                (pixGreen >= lowGreen && pixGreen <= hiGreen) &&
                (pixBlue  >= lowBlue  && pixBlue  <= hiBlue))
            {
                pThisBits[dwPixel] &= 0xff000000;
                pThisBits[dwPixel] |= (DWORD)set;
                ++count;
            }
		}
	}
	Unlock();
	return count;
}

/////////////////////////////////////////////////////////////////////////////

void*
GetBitsAt( const DDSURFACEDESC2 *pDDSD, int x, int y )
{
	// HACK: assume 32bpp
	// also -- questionable what (if anything) an ASSERT will do while we have a surface locked.
	ASSERT( 32==pDDSD->ddpfPixelFormat.dwRGBBitCount );

	DWORD *pBits = (DWORD *)pDDSD->lpSurface;
	pBits += y * (pDDSD->lPitch/4); // hmm! assumes lPitch is DWORD divisible
	pBits += x;

	return pBits;
}

/////////////////////////////////////////////////////////////////////////////

HDC CSurface::GetDC()
{
	HDC hDC = 0;
	HRESULT hr = m_pdds->GetDC( &hDC );
	if( FAILED(hr) )
		hDC = 0;
	return hDC;
}

/////////////////////////////////////////////////////////////////////////////

void CSurface::ReleaseDC( HDC hDC )
{
	m_pdds->ReleaseDC( hDC );
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
CSurface::Blt( DWORD x, DWORD y, LPDIRECTDRAWSURFACE7 pdds, RECT* prc, DWORD dwFlags )
{
	if( !m_pdds )
		return E_POINTER;
	return m_pdds->BltFast( x, y, pdds, prc, dwFlags );
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
CSurface::Blt( DWORD x, DWORD y, CSurface* pSurface, RECT* prc )
{
	if( !pSurface )
		return E_INVALIDARG;

	DWORD dwFlags = pSurface->IsColorKeyed() ? DDBLTFAST_SRCCOLORKEY : DDBLTFAST_NOCOLORKEY;
	return Blt( x, y, pSurface->GetDDrawSurface(), prc, dwFlags );
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
CSurface::ColorFill( const RECT *pRect, COLORREF clr )
{
	DDBLTFX fx;
	ZeroMemory( &fx, sizeof(fx) );
	fx.dwSize		= sizeof( fx );
	fx.dwFillColor	= clr;
	return m_pdds->Blt( const_cast<RECT*>(pRect), 0, 0, DDBLT_COLORFILL, &fx );
}

//

HRESULT
CSurface::SlowRectangle(const RECT* pRect, COLORREF clr)
{
	DDBLTFX fx;
	ZeroMemory(&fx, sizeof(fx));
	fx.dwSize = sizeof(fx);
	fx.dwFillColor = clr;
	RECT left = { pRect->left, pRect->top, pRect->left + 1, pRect->top + RECTHEIGHT(*pRect) };
	m_pdds->Blt(&left, 0, 0, DDBLT_COLORFILL, &fx);
	RECT top = { pRect->left, pRect->top, pRect->right, pRect->top + 1 };
	m_pdds->Blt(&top, 0, 0, DDBLT_COLORFILL, &fx);
	RECT right = { pRect->right - 1, pRect->top, pRect->right, pRect->top + RECTHEIGHT(*pRect) };
	m_pdds->Blt(&right, 0, 0, DDBLT_COLORFILL, &fx);
	RECT bottom = { pRect->left, pRect->bottom - 1, pRect->right, pRect->bottom };
	return m_pdds->Blt(&bottom, 0, 0, DDBLT_COLORFILL, &fx);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
CSurface::BltWindow( HWND hWnd, const RECT *pRect )
{
	HRESULT hr = S_OK;

	HDC hdcWnd = ::GetDC( hWnd );
	HDC hdcSurface = GetDC();
	ASSERT( hdcWnd && hdcSurface );
	RECT rc;
	if (nullptr == pRect)
	{
		// min(rcclient.width(),getwidth()), min(rcclient.height(),getheight())
		SetRect( &rc, 0, 0, GetWidth(), GetHeight() );
		pRect = &rc;
	}
	BOOL bGood = ::BitBlt( hdcSurface, 0, 0, RECTWIDTH(*pRect), RECTHEIGHT(*pRect),
						   hdcWnd, pRect->left, pRect->top, SRCCOPY );
	DWORD dwError = 0;
	if (bGood)
    {
        m_rcLastBlt = *pRect;
    }
    else
	{
		dwError = GetLastError();
		hr = E_FAIL;
	}
	ReleaseDC( hdcSurface );
	::ReleaseDC( hWnd, hdcWnd );
	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
CSurface::WriteBMP( LPCTSTR pszFilename) const
{
    RECT rc;
    SetRect(&rc, 0, 0, GetWidth(), GetHeight());
    return WriteBMP(pszFilename, rc);
}


HRESULT
CSurface::WriteBMP( LPCTSTR pszFilename, const RECT& rc ) const
{
	HRESULT hr = E_FAIL;
	DWORD *pDst = 0;
	DWORD *pSaveDst = 0;

	HANDLE hFile = CreateFile( pszFilename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0 );
	if( INVALID_HANDLE_VALUE==hFile )
		return E_FAIL;

	DWORD dwWidth = RECTWIDTH(rc);
	DWORD dwHeight = RECTHEIGHT(rc);
	DWORD dwPitch = dwWidth;
#if 0
	if( dwPitch&3 )
		dwPitch += 4-(dwPitch&3);
#endif
	DWORD dwSizeImage = dwPitch*dwHeight;
	DWORD dwImageBytes = dwSizeImage * 4;
	DWORD dwBytes;
	BITMAPFILEHEADER bfh;
	bfh.bfType		 = 0x4d42;
	bfh.bfSize		 = dwImageBytes + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bfh.bfOffBits	 = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bfh.bfReserved1	 = 0;
	bfh.bfReserved2	 = 0;

	WriteFile( hFile, &bfh, sizeof(bfh), &dwBytes, 0 );
	if( dwBytes!=sizeof(bfh) )
		goto fail;

	BITMAPINFOHEADER bih;
	bih.biSize				= sizeof(bih);
	bih.biWidth				= dwWidth;
	bih.biHeight			= dwHeight; //-(int)dwHeight;
	bih.biPlanes			= 1;
	bih.biBitCount			= 32;
	bih.biCompression		= BI_RGB;
	bih.biSizeImage			= 0; // dwImageBytes;
	bih.biXPelsPerMeter		= 3780;
	bih.biYPelsPerMeter		= 3780;
	bih.biClrUsed			= 0;
	bih.biClrImportant		= 0;

	WriteFile( hFile, &bih, sizeof(bih), &dwBytes, 0 );
	if( dwBytes!=sizeof(bih) )
		goto fail;

	DDSURFACEDESC2	ddsd;
	hr = Lock( &ddsd );
	if( FAILED(hr) )
		goto fail;

	pDst = new DWORD[dwSizeImage];
	pSaveDst = pDst;
	DWORD dwLine;
	for( dwLine=dwHeight; 0<dwLine; --dwLine )
	{
		DWORD *pSrc;
		pSrc = (DWORD *)GetBitsAt( &ddsd, rc.left, rc.top+dwLine-1 );

		DWORD dwPixel;
		for( dwPixel=0; dwPixel<dwWidth; ++dwPixel )
		{
			*(pDst++) = pSrc[dwPixel];
		}
		for( ; dwPixel<dwPitch; ++dwPixel )
		{
			*(pDst++) = 0;
		}
	}

	WriteFile( hFile, pSaveDst, dwImageBytes, &dwBytes, 0 );
	if( dwBytes!=dwImageBytes )
		goto fail;

	hr = S_OK;

fail:;
	Unlock();
	if( pSaveDst )
		delete [] pSaveDst;
	CloseHandle( hFile );
	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
CSurface::FindPixel
(
const POINT&	pt,
const SIZE&		size,
COLORREF		clr,
int				iDirection,
POINT&			ptResult
)
const
{
	RECT rc;
	SetRect( &rc, pt.x, pt.y, pt.x+size.cx, pt.y+size.cy );
	return FindPixel( &rc, clr, iDirection, ptResult );
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
CSurface::FindPixel(
    const RECT * pRect,
    COLORREF     clr,
    int          iDirection,
    POINT&       ptResult
)
const
{
	switch( iDirection )
	{
	case Left:
	case Right:
		break;
	default:
		ASSERT( 0 );
		return E_FAIL;
	}

	DDSURFACEDESC2 ddsd;
	HRESULT hr = Lock( &ddsd );
	if( FAILED(hr) )
		return hr;

	RECT rc;
	if( !pRect )
	{
		SetRect( &rc, 0, 0, GetWidth(), GetHeight() );
		pRect = &rc; 
	}
	bool bFound = false;
	int x;
	int xEnd;
	int xInc;
	int yStart = pRect->top;
	int yEnd   = pRect->bottom;
	int yInc   = 1;
	int y = yStart;
	switch( iDirection )
	{
	case Right:
		x = pRect->right-1;
		xEnd = pRect->left-1;
		xInc = -1;
		break;
	case Left:
	default:
		x = pRect->left;
		xEnd = pRect->right;
		xInc = 1;
		break;
	}
	for( ; x!=xEnd; x+=xInc )
	{
 		for( y=yStart; y!=yEnd; y+=yInc)
		{
			const DWORD *pBits = (DWORD *)GetBitsAt( &ddsd, x, y );
			if( *pBits==clr )
			{
				bFound = true;
				break;
			}
			pBits += ddsd.lPitch/4; // ack!! 32 bit color
		}
		if( bFound )
			break;
	}
	Unlock();

	if( bFound )
	{
		ptResult.x = x;
		ptResult.y = y;
		return S_OK;
	}
	return E_FAIL;
}


/////////////////////////////////////////////////////////////////////////////

void
CSurface::
GetClientRect(RECT* pRect) const
{
    SetRect(pRect, 0, 0, GetWidth(), GetHeight());
}

/////////////////////////////////////////////////////////////////////////////

const Rect_t&
CSurface::
GetBltRect() const
{
    return m_rcLastBlt;
}

/////////////////////////////////////////////////////////////////////////////

bool
CSurface::
FindSurfaceInRect(
    const CSurface& surface,
    const Rect_t&   findRect,
          POINT&    ptOrigin,
    const POINT*    pptHint) const
{
    // Check if origin is found at hint location
    if ((nullptr != pptHint) && ((0 < pptHint->x) || (0 < pptHint->y)))
    {
        if (Compare(pptHint->x, pptHint->y, surface))
        {
            LogInfo(L"CSurface::FindSurfaceInRect() Hint match");
            ptOrigin = *pptHint;
            return true;
        }
        LogInfo(L"CSurface::FindSurfaceInRect() Hint mis-match");
    }
    Rect_t surfaceRect;
    surface.GetClientRect(&surfaceRect);
    Rect_t scanRect(findRect);
    scanRect.right -= surfaceRect.Width();
    scanRect.bottom -= surfaceRect.Height();
    for (int y = scanRect.top; y < scanRect.bottom; ++y)
    {
        for (int x = scanRect.left; x < scanRect.right; ++x)
        {
            if (Compare(x, y, surface))
            {
                ptOrigin.x = x;
                ptOrigin.y = y;
                return true;
            }
        }
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////

size_t
CSurface::
GetWidthInColorRange(
    int x,
    int y,
    COLORREF low,
    COLORREF hi,
    DWORD    Flags) const
{
    int Width = 0;

    const DWORD lowRed   = GetRValue(low);
    const DWORD lowGreen = GetGValue(low);
    const DWORD lowBlue  = GetBValue(low);
    const DWORD hiRed    = GetRValue(hi);
    const DWORD hiGreen  = GetGValue(hi);
    const DWORD hiBlue   = GetBValue(hi);

#if 0
    DDPIXELFORMAT pf;
    GetPixelFormat(pf);
#endif

	DDSURFACEDESC2	ddsd;
	HRESULT hr;
	hr = Lock( &ddsd );
	if( FAILED(hr) )
		return 0;

    const DWORD *pThisBits = (DWORD *)GetBitsAt(&ddsd, 0, y);
    int SurfaceWidth = int(GetWidth());
    for (; x < SurfaceWidth; ++x, ++Width)
    {
        DWORD pix = pThisBits[x] & 0x00ffffff;
        const DWORD pixRed   = GetMaskValue(pix, m_pf.dwRBitMask);
        const DWORD pixGreen = GetMaskValue(pix, m_pf.dwGBitMask);
        const DWORD pixBlue  = GetMaskValue(pix, m_pf.dwBBitMask);
        if ((pixRed   < lowRed)   || (pixRed   > hiRed) ||
            (pixGreen < lowGreen) || (pixGreen > hiGreen) ||
            (pixBlue  < lowBlue)  || (pixBlue  > hiBlue))
        {
            if (Flags & 1)
            {
                LogInfo(L"GetWidthInColorRange(): RGB(%d,%d,%d) @ (%d,%d) Width(%d)",
                        pixRed, pixGreen, pixBlue, x, y, Width);
            }
            break;
		}
	}
	Unlock();
	return Width;
}

/////////////////////////////////////////////////////////////////////////////

size_t
CSurface::
GetIntensityCount(const Rect_t& Rect, size_t Intensity) const
{
	DDSURFACEDESC2	ddsd;
	HRESULT hr;
	hr = Lock( &ddsd );
	if( FAILED(hr) )
		return false;

#if 0
    DDPIXELFORMAT pf;
    GetPixelFormat(pf);
#endif

    size_t Count = 0;

	DWORD dwHeight = Rect.Height();
	DWORD dwWidth = Rect.Width();
	for( DWORD dwLine=0; dwLine<dwHeight; ++dwLine )
	{
		DWORD *pThisBits = (DWORD *)GetBitsAt( &ddsd, Rect.left, Rect.top+dwLine );
		for( DWORD dwPixel=0; dwPixel<dwWidth; ++dwPixel )
		{
			DWORD pix = pThisBits[dwPixel] & 0x00ffffff;
            const DWORD pixRed   = GetMaskValue(pix, m_pf.dwRBitMask);
            const DWORD pixGreen = GetMaskValue(pix, m_pf.dwGBitMask);
            const DWORD pixBlue  = GetMaskValue(pix, m_pf.dwBBitMask);
            if ((pixRed >= Intensity) ||
                (pixGreen >= Intensity) ||
                (pixBlue >= Intensity))
            {
                ++Count;
            }
		}
	}
	Unlock();
	return Count;
}

/////////////////////////////////////////////////////////////////////////////
// return (color1.allcomponentintensity < color2.allcomponentintensity);
bool
CSurface::
CompareColorIntensityFunc(COLORREF color1, COLORREF color2) const
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

/////////////////////////////////////////////////////////////////////////////
