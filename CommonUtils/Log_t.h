////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// Log_t.h
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif

#ifndef Include_LOG_T_H
#define Include_LOG_T_H

#include "ThreadQueue_t.h"
#include "AutoCs.h"
#include "Log.h"

////////////////////////////////////////////////////////////////////////////////

class LogWindow_t
{
    HWND m_hWnd;

public:

    LogWindow_t() : m_hWnd(0)
    {}

    void
        Init();

    void Close()
    {
        if (::IsWindow(m_hWnd))
        {
            DestroyWindow(m_hWnd);
            m_hWnd = 0;
        }
    }

    void Log(std::shared_ptr<const std::wstring>& buf);
    void Log(const wchar_t* buf);

    HWND GetHwnd() const { return m_hWnd; }

};

////////////////////////////////////////////////////////////////////////////////

class Log_t
{

public:

    static const size_t StringLength       = 256;

    struct ProcessThreadData
    {
        void operator()(
            ThreadQueue::State_t State,
            const std::wstring&  Data,
            Log_t*               pLog) const
        {
            if (ThreadQueue::State::Execute == State)
            {
                pLog->LogString(Data.c_str());
            }
        }
    };

private:

    typedef ThreadQueue_t<std::wstring, Log_t, ProcessThreadData> Thread_t;
    friend class Thread_t;

private:

    LogWindow_t    m_logWindow;
    Thread_t       m_LogThread;
    std::wofstream m_Log;
    bool           m_bInitialized;
    int            m_iLevel;
    Log::Output_t  m_output;

public:

    Log_t();
    ~Log_t();

    bool
    Initialize(
        bool initWindow = true);

    void
    Shutdown();

    bool
    Open(
        const wchar_t* pszSuffix);

    void
    Close();

    void
    Log(
        const wchar_t* pszBuf);

    static
    Log_t&
    Get();

    std::wofstream&
    GetLog()
    {
        return m_Log;
    }

    void
    SetOutput(
        Log::Output_t output)
    {
        m_output = output;
    }

    void
    SetLogLevel(
        int iLevel)
    {
        m_iLevel = iLevel;
    }

    int
    GetLogLevel() const
    {
        return m_iLevel;
    }

    void
    Process(
        ThreadQueue::State_t State,
        const std::wstring&  Data,
              Log_t*         pLog);

    const LogWindow_t& GetLogWindow() const { return m_logWindow; }

private:

    void
    LogString(
        const wchar_t* pszBuf);

private:
    Log_t(const Log_t&);
    Log_t& operator=(const Log_t&);
};

////////////////////////////////////////////////////////////////////////////////

#endif // Include_LOG_T_H

