////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// Log.h
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_LOG_H
#define Include_LOG_H

////////////////////////////////////////////////////////////////////////////////

namespace Log
{
    namespace Output
    {
        enum E : unsigned
        {
            Console = 0x0001,
            Debug   = 0x0002,
            Window  = 0x0004
        };
    }
    typedef Output::E Output_t;

    void SetOutput(Output_t output);
    void SetLevel(int iLevel);
}

#define LOGERROR    1
#define LOGWARNING  2
#define LOGINFO     3

wchar_t* GetTimeString(SYSTEMTIME& t);

void LogAlways (const wchar_t* pszFormat, ...);
void LogInfo   (const wchar_t* pszFormat, ...);
void LogWarning(const wchar_t* pszFormat, ...);
void LogError  (const wchar_t* pszFormat, ...);

void Assert    (bool bExpr, const char* pFile, int iLine);

#undef ASSERT
#ifdef _DEBUG
#define ASSERT(b)  Assert(b, __FILE__, __LINE__)
#else
#define ASSERT(b)
#endif

////////////////////////////////////////////////////////////////////////////////

#endif // Include_LOG_H

