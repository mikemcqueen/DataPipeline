//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//

#include "stdafx.h"
#include <process.h>

/////////////////////////////////////////////////////////////////////////////

namespace util
{

/////////////////////////////////////////////////////////////////////////////

#define NS100_PER_MS						10000				// 100ns per ms

/////////////////////////////////////////////////////////////////////////////

void SetWaitableTimer( HANDLE hTimer, DWORD dwDelay, bool bPeriodic )
{
	LONG lDelay = -(LONG)dwDelay;
	__int64 qiDelay = (__int64)lDelay * NS100_PER_MS;	// convert from ms to 100ns units
	LARGE_INTEGER liDelay;
	liDelay.LowPart = (DWORD)(qiDelay&0xffffffff);
	liDelay.HighPart = (LONG)(qiDelay>>32);
//	CancelWaitableTimer( hTimer );
	BOOL b = ::SetWaitableTimer( hTimer, &liDelay, bPeriodic ? dwDelay : 0, 0, 0, FALSE );
    b;
}

/////////////////////////////////////////////////////////////////////////////

typedef UINT (WINAPI *BEGINTHREADEX_START_ROUTINE)( void * );

HANDLE CreateThread
(
LPSECURITY_ATTRIBUTES		lpAttr,
DWORD						dwStack,
LPTHREAD_START_ROUTINE		lpStart,
LPVOID						lpParam,
DWORD						dwFlags,
LPDWORD						lpdwID
)
{		 
	return (HANDLE)_beginthreadex( lpAttr,
								   dwStack,
								   (BEGINTHREADEX_START_ROUTINE)lpStart,
								   lpParam,
								   dwFlags,
								   (UINT *)lpdwID );
}

/////////////////////////////////////////////////////////////////////////////

void ExitThread( DWORD dwCode )
{
	_endthreadex( dwCode );
}

/////////////////////////////////////////////////////////////////////////////

bool
IsParent(HWND hParent, HWND hWnd)
{
    HWND h = hWnd;
    do
    {
        h = GetParent(h);
        if (hParent == h)
            return true;
    }
    while (nullptr != h);
    return false;
}

///////////////////////////////////////////////////////////////////////////////////
/*
bool
FileExists( LPCTSTR pszFile )
{
	// NOTE: need to convert wide -> dbcs?		
	// quick & dirty
	DWORD dwAttr = GetFileAttributes( pszFile );
	return dwAttr!=-1;
}

/////////////////////////////////////////////////////////////////////////////

LPTSTR
CopyString( LPCTSTR	pStr )
{
	LPTSTR pNewStr = new TCHAR[_tcslen(pStr)+1];
	_tcscpy( pNewStr, pStr );
	return pNewStr;
}

/////////////////////////////////////////////////////////////////////////////

bool
RollPercent( int iPercent )
{
	return (rand()%100) < iPercent;
}
*/

/////////////////////////////////////////////////////////////////////////////

} // namespace util

/////////////////////////////////////////////////////////////////////////////
