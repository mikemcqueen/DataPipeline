////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// Log.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Log_t.h"
#include "Log.h"
#include <time.h>

////////////////////////////////////////////////////////////////////////////////

Log_t::
Log_t() :
    m_iLevel(LOGWARNING),
    m_bInitialized(false),
    m_output(Log::Output::Console)
{    
}

////////////////////////////////////////////////////////////////////////////////

Log_t::
~Log_t()
{
}

////////////////////////////////////////////////////////////////////////////////

bool
Log_t::
Initialize(
    bool initWindow /*= true*/)
{
    if (!m_LogThread.Initialize(this, L"LogThread", false))
    {
        return false;
    }
    if (initWindow)
    {
        m_logWindow.Init();
//        return false;
    }
    m_bInitialized = true;
    return true;
}

////////////////////////////////////////////////////////////////////////////////

void
Log_t::
Shutdown()
{
    if (m_bInitialized)
    {
        m_LogThread.Shutdown();
        m_logWindow.Close();
        Close();
    }
}

////////////////////////////////////////////////////////////////////////////////

Log_t&
Log_t::
Get()
{
    static std::shared_ptr<Log_t> spLog;
    if (!spLog)
    {
        spLog.reset(new Log_t);
    }
    return *spLog.get();
}

////////////////////////////////////////////////////////////////////////////////

void
Log_t::
Log(
    const wchar_t* pszBuf)
{
    if (m_bInitialized && m_LogThread.IsInitialized())
    {
        std::wstring str(pszBuf);
        m_LogThread.QueueData(str);
//        m_logWindow.Log(str);
    }
    else
    {
        LogString(pszBuf);
    }
}

////////////////////////////////////////////////////////////////////////////////
// TODO: move to utils
// NOT THREADSAFE.  Take a buffer as param & return pointer to it.
//
wchar_t*
GetTimeString(SYSTEMTIME& t)
{
    static wchar_t szBuf[256];
    _snwprintf_s(szBuf, _countof(szBuf), _TRUNCATE, L"%02d%02d%02d_%02d%02d%04d", 
        t.wHour,
		t.wMinute,
		t.wSecond,
		t.wMonth,
		t.wDay,
		t.wYear);
    return szBuf;
}

////////////////////////////////////////////////////////////////////////////////

bool
Log_t::
Open(
    const wchar_t* pszSuffix)
{
	SYSTEMTIME t;
	GetLocalTime(&t);
    wchar_t szBuf[256];
	_snwprintf_s(szBuf, /*_countof(szBuf),*/ _TRUNCATE, L"log\\%s_%s.log", GetTimeString(t), pszSuffix);
    m_Log.clear();
	m_Log.open(szBuf);
    return !m_Log.bad() && !m_Log.fail();
}

////////////////////////////////////////////////////////////////////////////////

void
Log_t::
Close()
{
	m_Log.close();
}

/////////////////////////////////////////////////////////////////////////////

void
Log_t::
LogString(
    const wchar_t* pszBuf)
{
    if (m_bInitialized)
    {
        if (m_Log.is_open())
        {
            m_Log.write(pszBuf, std::streamsize(wcslen(pszBuf)));
            m_Log.flush();
        }
        m_logWindow.Log(pszBuf);
//        return;
    }
    if (Log::Output::Console & m_output)
    {
        wprintf(L"%ls", pszBuf);
    }
    if (Log::Output::Debug & m_output)
    {
        OutputDebugString(pszBuf);
    }
}

/////////////////////////////////////////////////////////////////////////////

namespace Log
{
    void
    SetOutput(Output_t output)
    {
        Log_t::Get().SetOutput(output);
    }

    void
    SetLevel(int iLevel)
    {
        Log_t::Get().SetLogLevel(iLevel);
    }
}

/////////////////////////////////////////////////////////////////////////////

static
int
InitLogBuffer(
          wchar_t* pszBuf,
          size_t   CchBuffer,
    const wchar_t* pszType)
{
    int Count = _snwprintf_s(pszBuf, CchBuffer, _TRUNCATE, L"%ls: ",
                             const_cast<wchar_t*>(pszType));
    if (Count < 0) {
      throw std::runtime_error("InitLogBuffer");
    }
    return Count;
}

/////////////////////////////////////////////////////////////////////////////

static
void
AppendNewline(
    wchar_t* Buf,
    size_t   Count)
{
    size_t Last = Count - 2;
    size_t Pos = 0;
    for (; Pos < Last; ++Pos)
    {
        if (L'\0' == Buf[Pos])
            break;
    }
    Buf[Pos++] = L'\n';
    Buf[Pos]   = L'\0';
}

/////////////////////////////////////////////////////////////////////////////

void
LogAlways(
    const wchar_t* pszFormat,
    ...)
{
    wchar_t szBuf[Log_t::StringLength];
    int Len = InitLogBuffer(szBuf, _countof(szBuf), L"LOG");
    if (0 == Len) Len = 1;
    va_list marker;
    va_start(marker, pszFormat);
    _vsnwprintf_s(&szBuf[Len], _countof(szBuf) - Len, _TRUNCATE, pszFormat, marker);
    va_end(marker);
    AppendNewline(szBuf, _countof(szBuf));
    Log_t::Get().Log(szBuf);
}

/////////////////////////////////////////////////////////////////////////////

void
LogInfo(
    const wchar_t* pszFormat,
    ...)
{
    if (LOGINFO > Log_t::Get().GetLogLevel())

      return;

    wchar_t szBuf[Log_t::StringLength];
    int Len = InitLogBuffer(szBuf, _countof(szBuf), L"INF");
    va_list marker;
    va_start(marker, pszFormat);
    _vsnwprintf_s(&szBuf[Len], _countof(szBuf) - Len, _TRUNCATE, pszFormat, marker);
    va_end(marker);
    AppendNewline(szBuf, _countof(szBuf));
    Log_t::Get().Log(szBuf);
}

/////////////////////////////////////////////////////////////////////////////

void
LogWarning(
    const wchar_t* pszFormat,
    ...)
{
    if (LOGWARNING > Log_t::Get().GetLogLevel())
        return;

    wchar_t szBuf[Log_t::StringLength];
    int Len = InitLogBuffer(szBuf, _countof(szBuf), L"WRN");
    va_list marker;
    va_start(marker, pszFormat);
    _vsnwprintf_s(&szBuf[Len], _countof(szBuf) - Len, _TRUNCATE, pszFormat, marker);
    va_end(marker);
    AppendNewline(szBuf, _countof(szBuf));
    Log_t::Get().Log(szBuf);
}

/////////////////////////////////////////////////////////////////////////////

void
LogError(
    const wchar_t* pszFormat,
    ...)
{
    if (LOGERROR > Log_t::Get().GetLogLevel())
        return;

    wchar_t szBuf[Log_t::StringLength];
    int Len = InitLogBuffer(szBuf, _countof(szBuf), L"ERR");
    va_list marker;
    va_start(marker, pszFormat);
    _vsnwprintf_s(&szBuf[Len], _countof(szBuf) - Len, _TRUNCATE, pszFormat, marker);
    va_end(marker);
    AppendNewline(szBuf, _countof(szBuf));
    Log_t::Get().Log(szBuf);
}

////////////////////////////////////////////////////////////////////////////

void
Assert(
          bool  bExpr,
    const char* pFile,
          int   iLine)
{
	if (!bExpr)
	{
		LogError(L"ASSERTION FALSE: File: %hs, Line %d", pFile, iLine);
	}
}

/////////////////////////////////////////////////////////////////////////////

