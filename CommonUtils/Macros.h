/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// Macros.h
//
/////////////////////////////////////////////////////////////////////////////

#ifndef Include_MACROS_H
#define Include_MACROS_H
#pragma once

/////////////////////////////////////////////////////////////////////////////

#define SAFE_DELETE(p)            { if(nullptr != (p)) { delete (p); (p)=nullptr; } }
#define SAFE_DELETE_ARRAY(p)    { if(nullptr != (p)) { delete[] (p); (p)=nullptr; } }
#define SAFE_RELEASE(p)            { if(nullptr != (p)) { (p)->Release(); (p)=nullptr; } }

#define POINT2LPARAM(pt)        MAKELPARAM((pt).x,(pt).y)

#define    HOUR2MS(h)                ((DWORD)(h)*3600000)
#define MINUTE2MS(m)            ((DWORD)(m)*60000)
#define SECOND2MS(s)            ((DWORD)(s)*1000)

constexpr auto RECTWIDTH(const RECT& r) {
    return r.right - r.left;
}
constexpr auto RECTHEIGHT(const RECT& r) {
    return r.bottom - r.top;
}

#ifdef _DEBUG
#define SET_CRT_DEBUG_FIELD(a)        _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#define CLEAR_CRT_DEBUG_FIELD(a)    _CrtSetDbgFlag(~(a) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#else
#define SET_CRT_DEBUG_FIELD(a)        ((void) 0)
#define CLEAR_CRT_DEBUG_FIELD(a)    ((void) 0)
#endif

__inline void            ThrowIfFailed( HRESULT hr )        { if(FAILED(hr)) throw (hr); }
#define                    ReturnIfFailed( hr )            { if(FAILED(hr)) return (hr); }

/////////////////////////////////////////////////////////////////////////////

namespace util
{

bool IsParent(HWND hParent, HWND hWnd);

void  SetWaitableTimer( HANDLE hTimer, DWORD dwDelay, bool bPeriodic );

HANDLE CreateThread( LPSECURITY_ATTRIBUTES lpAttr, DWORD dwStack,
                              LPTHREAD_START_ROUTINE lpStart, LPVOID lpParam,
                              DWORD dwFlags, LPDWORD lpdwID );

void   ExitThread( DWORD dwCode );

int getopt(int argc, wchar_t* argv[], const wchar_t* opts);

/*
bool                    FileExists( LPCTSTR pszFile );

__inline void            SetPoint( POINT& pt, int x, int y ) { pt.x = x; pt.y = y; }

LPTSTR                    CopyString( LPCTSTR pStr );

bool                    RollPercent( int iPercent );
*/

}

/////////////////////////////////////////////////////////////////////////////

#endif // Include_MACROS_H

