
// DataPipeline stdafx.h

#pragma once

#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#ifdef INCLUDE_AFXDB
	#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit
	#include <afx.h>
	#include <afxdb.h>
	#include <odbcinst.h>
#else
	#include <windows.h>
	#include <windowsx.h>
    #include <minwinbase.h>
#endif

#if 0 // debug stuff
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#ifdef _DEBUG
   #define DP_DEBUG_NEW   new( _CLIENT_BLOCK, __FILE__, __LINE__)
   #define DP_DEBUG_PLACEMENT_NEW(pv)   new( _CLIENT_BLOCK, __FILE__, __LINE__, pv)
#endif // _DEBUG
#endif // debug stuff

#include <shlwapi.h>
#include <time.h>

#include <chrono>
#include <concepts>
#include <type_traits>
#include <cassert>
#include <format>
#include <vector>
#include <queue>
#include <stack>
#include <map>
#include <unordered_map>
#include <set>
#include <string>
#include <cwctype>
#include <iostream>
#include <fstream>
#include <algorithm>
//#include <hash_map>
//#include <hash_set>
#include <functional>
#include <numeric>
#include <memory>

#include "xmllite.h"

#if _MSC_VER >= 1000
    #pragma warning(disable:4063) // case statement of enum type with integer
	#pragma warning(disable:4355)  // 'this' used in member initializer list
	#pragma warning(disable:4480)  // enum : type
	#pragma warning(disable:4481)  // 'override' keyword
	#pragma warning(disable:28125) // InitialzeCriticalSection in try/except block
#endif

using namespace std;
