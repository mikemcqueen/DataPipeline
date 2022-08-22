
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//

#pragma once
#ifndef Include_TIMER_T_H
#define Include_TIMER_T_H

#include "Log.h"

class Timer_t
{

private:

    std::wstring strText;
    FILETIME     FileTime;

public:

    explicit
    Timer_t(
        const wchar_t* pszText = L"Timer_t",
        bool           bLogNow = true)
    :
        strText(pszText)
    {
        Now(FileTime);
        if (bLogNow)
        {
            LogAlways(L"++%ls", strText.c_str());
        }
    }

    ~Timer_t()
    {
        Show();
    }

    static
    const FILETIME&
    Now(
        FILETIME& ft)
    {
        SYSTEMTIME st;
        GetSystemTime(&st);
        SystemTimeToFileTime(&st, &ft);
        return ft;
    }

    size_t
    Diff()
    {
        FILETIME ft;
        Now(ft);
        LARGE_INTEGER lNow;
        lNow.LowPart =  ft.dwLowDateTime;
        lNow.HighPart = ft.dwHighDateTime;
        LARGE_INTEGER lThen;
        lThen.LowPart =  FileTime.dwLowDateTime;
        lThen.HighPart = FileTime.dwHighDateTime;
        FileTime = ft;
        return (size_t)((lNow.QuadPart - lThen.QuadPart) / 10000000);
    }

    void
    Show(const wchar_t* pszText = NULL,
               bool     bAlways = false)
    {
        size_t diff = Diff();
        if (NULL == pszText)
        {
            if (bAlways)
                LogAlways(L"--%ls: %d seconds", strText.c_str(), diff);
            else
                LogInfo(L"--%ls: %d seconds", strText.c_str(), diff);
        }
        else
        {
            if (bAlways)
                LogAlways(L"%ls: %d seconds", pszText, diff);
            else
                LogInfo(L"%ls: %d seconds", pszText, diff);
        }
    }

    void
    Set(const FILETIME& Time)
    {
        FileTime = Time;
    }

    void
    Set()
    {
        Now(FileTime);
    }

};

#endif // Include_TIMER_T_H
