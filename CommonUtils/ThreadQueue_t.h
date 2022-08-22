////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// ThreadQueue_t.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef Include_THREADQUEUE_H
#define Include_THREADQUEUE_H

#include "AutoHand.h"
#include "AutoCs.h"
#include "Macros.h"
#include "Log.h"

////////////////////////////////////////////////////////////////////////////////

namespace ThreadQueue
{
    namespace State
    {
        enum E
        {
            Startup,
            Shutdown,
            Execute,
            Free,
            Display
        };
    }
    typedef State::E State_t;

    enum LogLevel_e
    {
        Off,
        Low,
        High
    };
};

template<
    class Data_t,
    class Param_t,
    class Process_fn,
    class Compare_fn = std::equal_to<Data_t> >
class ThreadQueue_t
{
private:

    enum
    {
        Exit = 0,
        Add,
        HandleCount
    };

    typedef ThreadQueue_t<
                Data_t,
                Param_t,
                Process_fn,
                Compare_fn>    Thread_t;

    typedef std::deque<Data_t> Queue_t;

private:

    Queue_t        m_Queue;
    CAutoCritSec   m_csQueue;
    CAutoHandle    m_hAddEvent;
    CAutoHandle    m_hExitEvent;
    CAutoHandle    m_hIdleEvent;
    CAutoHandle    m_hThread;
    DWORD          m_dwThreadId;
    const wchar_t* m_pszName;
    Param_t*       m_pParam;
    bool           m_bInitialized;
    Process_fn     m_Process;
    Compare_fn     m_Compare;

public:

    ThreadQueue::LogLevel_e m_LogLevel;

public:

    ThreadQueue_t<Data_t, Param_t, Process_fn, Compare_fn>() :
        m_dwThreadId(0L),
        m_pszName(nullptr),
        m_pParam(nullptr),
        m_bInitialized(false),
        m_LogLevel(ThreadQueue::Off)
    {
    }

    ~ThreadQueue_t<Data_t, Param_t, Process_fn, Compare_fn>()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////
    //
    // Initialize
    //
    ////////////////////////////////////////////////////////////////////////////////

    bool
    Initialize(
              Param_t* pParam,
        const wchar_t* pszName,
              bool     bUseLog = true)
    {
        if (IsInitialized())
        {
            if (bUseLog)
                LogWarning(L"ThreadQueue_t<%ls>::Initialize() already called.", m_pszName);
            else
                wprintf(L"ThreadQueue_t<%ls>::Initialize() already called.", m_pszName);
            return false;
        }
        if ((NULL == pszName) || (L'\0' == pszName[0]))
        {
            if (bUseLog)
                LogError(L"ThreadQueue_t<>::Initialize() Invalid name.");
            else
                _putws(L"ThreadQueue_t<>::Initialize() Invalid name.");
        }
        if (bUseLog)
        {
            LogInfo(L"ThreadQueue_t<%ls>::Initialize()", pszName);
        }
        else
        {
            wprintf(L"ThreadQueue_t<%ls>::Initialize()\n", pszName);
        }

        m_hAddEvent = CreateEvent(0, FALSE, FALSE, 0);
        if (NULL == m_hAddEvent.get())
            return false;

        m_hExitEvent = CreateEvent(0, TRUE, FALSE, 0);
        if (NULL == m_hExitEvent.get())
            return false;

        m_hIdleEvent = CreateEvent(0, TRUE, FALSE, 0);
        if (NULL == m_hIdleEvent.get())
            return false;

        m_hThread = util::CreateThread(0, 0, ThreadFunc, (void *)this, 0, &m_dwThreadId);
        if (NULL == m_hThread.get())
            return false;

        m_pParam  = pParam;
        m_pszName = pszName;

        m_bInitialized = true;
        return true;
    }

    ////////////////////////////////////////////////////////////////////////////////
    //
    // Shutdown:
    //
    ////////////////////////////////////////////////////////////////////////////////

    void
    Shutdown()
    {
        if (!m_bInitialized)
        {
            return;
        }

        m_bInitialized = false;

        if (NULL != m_hThread.get())
        {
            SetEvent(m_hExitEvent.get());
            WaitForSingleObjectEx(m_hThread.get(), INFINITE, FALSE);
            m_hThread.Close();
        }

        size_t Size = 0;
        {
            CLock lock(m_csQueue);
            Size = m_Queue.size();
        }
        LogInfo(L"ThreadQueue_t<%ls>::Shutdown() Items in queue (%d)", m_pszName, Size);

        // TODO:
        // Lock callback data queue
        // free all entries
    }

    ////////////////////////////////////////////////////////////////////////////////
    //
    // Queue a data item for processing:
    //
    ////////////////////////////////////////////////////////////////////////////////

    void
    QueueData(
        const Data_t& Data)
    {
        if (!IsInitialized())
        {
            wprintf(L"ERROR: ThreadQueue_t<%s>::QueueData(): Object is not intialized", m_pszName);
            throw E_FAIL;
        }
        m_Process.operator()(ThreadQueue::State::Display, Data, m_pParam);
        CLock lock(m_csQueue);
        m_Queue.push_back(Data);
        SetEvent(m_hAddEvent.get());
        ResetEvent(m_hIdleEvent.get());
    }

    ////////////////////////////////////////////////////////////////////////////////
    //
    // Remove all items which match based on T::operator==()
    //
    ////////////////////////////////////////////////////////////////////////////////

    size_t
    RemoveAll(
        const Data_t& Data)
    {
        LogInfo(L"ThreadQueue_t<%s>::RemoveAll()", m_pszName);

        if (!IsInitialized())
        {
            wprintf(L"ERROR: ThreadQueue_t<%s>::RemoveAll(): Object is not intialized", m_pszName);
            throw E_FAIL;
        }

        size_t Freed = 0;
        size_t Skipped = 0;
        size_t Size = 0;
        {
            CLock lock(m_csQueue);
            Queue_t::iterator it = m_Queue.begin();
            while (m_Queue.end() != it)
            {
                if (m_Compare.operator()(Data, *it))
                {
                    m_Process.operator()(ThreadQueue::State::Display, *it, m_pParam);
                    m_Process.operator()(ThreadQueue::State::Free,    *it, m_pParam);
                    it = m_Queue.erase(it);
                    ++Freed;
                }
                else
                {
                    ++it;
                    ++Skipped;
                }
            }
            Size = m_Queue.size();
        }
        if (ThreadQueue::High == m_LogLevel)
            LogInfo(L"Freed %d, Skipped %d, Size=%d", Freed, Skipped, Size);
        return Freed;
    }

    ////////////////////////////////////////////////////////////////////////////////

    bool IsInitialized() const { return m_bInitialized; }
    HANDLE GetThread() const { return m_hThread.get(); }
    HANDLE GetIdleEvent() const { return m_hIdleEvent.get(); }

private:

    ////////////////////////////////////////////////////////////////////////////////
    //
    // Thread function:
    //
    ////////////////////////////////////////////////////////////////////////////////

    static
    DWORD WINAPI
    ThreadFunc(
        void *pvParam)
    {
        Thread_t *pThread  = static_cast<Thread_t*>(pvParam);

        DWORD RetCode = 0;
        try
        {
            RetCode = pThread->Func();
        }
        catch (std::exception& e)
        {
            LogError(L"### ThreadQueue_t<%ls> Caught %hs: %hs ###",
                     pThread->m_pszName, typeid(e).name(), e.what());
        }
        catch (...)
        {
            LogError(L"### ThreadQueue_t<%ls> Unhandled exception ###",
                     pThread->m_pszName);
        }
        return RetCode;
    }

    ////////////////////////////////////////////////////////////////////////////////

    DWORD
    Func()
    {
        HANDLE Handles[HandleCount];
        Handles[Exit] = m_hExitEvent.get();
        Handles[Add]  = m_hAddEvent.get();

        OnStartup();
        bool bExit = false;
        while (!bExit)
        {
            DWORD dw = WaitForMultipleObjectsEx(HandleCount, Handles, FALSE, 0, TRUE);
            if (WAIT_TIMEOUT == dw)
            {
                SetEvent(m_hIdleEvent.get());
                dw = WaitForMultipleObjectsEx(HandleCount, Handles, FALSE, INFINITE, TRUE);
                ResetEvent(m_hIdleEvent.get());
            }
            switch (dw)
            {
            case WAIT_OBJECT_0 + Exit:
                bExit = true;
                break;
            case WAIT_OBJECT_0 + Add:
                ProcessQueue();
                break;
            case WAIT_IO_COMPLETION:
                LogInfo(L"ThreadQueue_t<%s>::ThreadFunc()::WAIT_IO_COMPLETION", m_pszName);
                break;
            case WAIT_FAILED:
            default:
                ASSERT(false);
                break;
            }
        }
        OnShutdown();
        util::ExitThread(0);
        return 0;
    }

    ////////////////////////////////////////////////////////////////////////////////

    void
    OnStartup()
    {
        LogInfo(L"ThreadQueue_t<%s>::OnStartup()", m_pszName);
        m_Process(ThreadQueue::State::Startup, Data_t(), m_pParam);
    }

    ////////////////////////////////////////////////////////////////////////////////

    void
    OnShutdown()
    {
        LogInfo(L"ThreadQueue_t<%s>::OnShutdown()", m_pszName);
        m_Process(ThreadQueue::State::Shutdown, Data_t(), m_pParam);
    }

    ////////////////////////////////////////////////////////////////////////////////
    //
    // Process the queue until it's empty:
    //
    ////////////////////////////////////////////////////////////////////////////////

    void
    ProcessQueue()
    {
        for (;;)
        {
            Data_t Data;
            size_t Count = 0;
            {
                // Keep lock only long enough to pop front.
                CLock lock(m_csQueue);
                if (m_Queue.empty())
                {
                    // LogInfo(L"Queue is empty");
                    if (!ResetEvent(m_hAddEvent.get()))
                        LogError(L"ResetEvent failed (%d)", GetLastError());
                    return;
                }
                Count = m_Queue.size();
                Data = m_Queue.front();
                m_Queue.pop_front();
            }
            if (ThreadQueue::High == m_LogLevel)
            {
                LogInfo(L"ProcessQueue: Found entry, queue size was %d", Count);
            }
/*
            if (NULL == pData)
            {
                ASSERT(NULL != pData);
                LogError(L"ThreadQueue_t<%s>::ProcessQueue() error: pData == NULL (%d)",
                    m_pszName, m_Queue.size());
                return;
            }
*/
            if (ThreadQueue::High == m_LogLevel)
            {
                LogInfo(L"++ThreadQueue_t<%s>::ProcessData", m_pszName);
                m_Process(ThreadQueue::State::Display, Data, m_pParam);
            }

            try
            {
                m_Process(ThreadQueue::State::Execute, Data, m_pParam);
            }
            catch(std::exception& e)
            {
                LogError(L"ThreadQueue_t<%s>::ProcessData: Caught std::exception '%hs'",
                         m_pszName, e.what());
            }
            m_Process(ThreadQueue::State::Free, Data, m_pParam);
            if (ThreadQueue::High == m_LogLevel)
            {
                LogInfo(L"--ThreadQueue_t<%s>::ProcessData", m_pszName);
            }
            // must allow queued APCs to execute
            SleepEx(0, TRUE);
        }
    }
};

#endif // Include_THREADQUEUE_H
