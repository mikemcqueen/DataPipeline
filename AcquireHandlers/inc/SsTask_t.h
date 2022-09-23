/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// SsTask_t.h
//
// Screen Shot acquire handler.
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_SSTASK_T_H
#define Include_SSTASK_T_H

#include "DpHandler_t.h"
#include "DpEvent.h"
#include "DpMessage.h"
#include "AutoHand.h"
#include "AutoCs.h"
#include "Pool.h"
#include "Macros.h"

/////////////////////////////////////////////////////////////////////////////

class PipelineManager_t;
class CDisplay;
class CSurface;

/////////////////////////////////////////////////////////////////////////////

class SsTask_t :
    public DP::Handler_t
{

    static const size_t DefaultDelayMs   = 300;
    static const size_t DefaultPoolSize  = 3;

    static const size_t MaxEventDataSize = 1024;

    static DP::MessageId_t s_MessageId;

public:

    static DP::MessageId_t
    GetMessageId()
    {
        ASSERT(DP::Message::Id::Unknown != s_MessageId);
        return s_MessageId;
    }

private:

    CDisplay&   m_Display;
    size_t      m_SurfaceWidth;
    size_t      m_SurfaceHeight;
    size_t      m_DelayMs;
    size_t      m_PoolSize;

    CAutoHandle m_hThread;
    CAutoHandle m_hExitEvent;
    CAutoHandle m_hSuspendEvent;
    CAutoHandle m_hSuspendNotifyEvent;
    CAutoHandle m_hEvent;
    CAutoHandle m_hTimer;

    DWORD             m_dwThreadId;

    mutable volatile
    LONG              m_lSuspended;
    mutable volatile
    LONG              m_lSuspendCount;

    pool<CSurface>     m_Pool;

    mutable volatile
    LONG              m_lEventPending;
    std::vector<char> m_EventData;
    size_t            m_EventCount;

    wchar_t m_szTestSurface[MAX_PATH];

public:

    explicit
    SsTask_t(
        CDisplay& Display,
        size_t    SurfaceWidth,
        size_t    SurfaceHeight,
        size_t    PoolSize = DefaultPoolSize,
        size_t    DelayMs  = DefaultDelayMs);

    virtual
    ~SsTask_t();

    //
    // DP::Handler_t virtual
    //

    bool
    Initialize(
       const wchar_t* pszClass) override;

    HRESULT
    EventHandler(
        DP::Event::Data_t& Data) override;

    //
    // SsTask_t virtual
    //

    virtual
    HWND
    GetSsWindowRect(
        RECT& rcBounds) const = 0;

    virtual
    void
    ThreadProcessEvent() = 0;

    virtual
    void
    PostData(
        HWND               hWnd,
        pool<CSurface>::item_t* pPoolItem) = 0;

protected:

    void SuspendAndFlush();
    void Suspend();
    void Resume();

    void
    AddEventData(
        const DP::Event::Data_t* pData);

    bool
    SetEventPending(
       bool bPending);

    bool
    IsEventPending() const;

    void
    ClearEventData();

    void
    SetEventData(
        const DP::Event::Data_t& Data);

    size_t
    GetEventCount() const
    {
        return m_EventCount;
    }

    const DP::Event::Data_t&
    PeekEvent(
        size_t Event) const;

    void
    GetEventData(
        size_t             Event,
        DP::Event::Data_t& Data,
        size_t             Size) const;

    void
    SetAsyncEvent()
    {
        SetEvent(m_hEvent.get());
    }

private:

    bool
    Start();

    void
    Stop();

    void SetTestSurface(wchar_t* pszPath);

    HRESULT
    InitSurfacePool(
        CDisplay&     Display,
        pool<CSurface>& Pool,
        size_t        cx,
        size_t        cy,
        size_t        Count);

    HRESULT            InitSurfacePool(size_t Size);
    pool<CSurface>&     GetSurfacePool()              { return m_Pool; }
    pool<CSurface>::item_t* GetAvailableSurface();

    void
    Shutter();

    bool
    TakeSnapShot(
        HWND hWnd,
        const RECT& rc,
        CSurface *pSurface );

    bool
    SetSuspended(
        bool bSuspended);

    bool
    IsSuspended() const;

    bool
    SameThread() const
    {
        return GetCurrentThreadId() == m_dwThreadId;
    }

    static
    DWORD WINAPI
    ThreadFunc(void *pvParam);

private:

    SsTask_t() = delete;
    SsTask_t(const SsTask_t&) = delete;
    SsTask_t& operator=(const SsTask_t&) = delete;
};

/////////////////////////////////////////////////////////////////////////////

namespace SsTask
{
namespace Acquire
{
    struct Data_t :
        public DP::Message::Data_t
    {
        pool<CSurface>::item_t* pPoolItem;

        static void ReleaseFn(DP::Message::Data_t&);

        Data_t(
            const wchar_t* pClass,
            pool<CSurface>::item_t* InitPoolItem,
            size_t             Size = sizeof(Data_t))
            :
            DP::Message::Data_t(
                DP::Stage::Acquire,
                SsTask_t::GetMessageId(),
                Size,
                pClass,
                DP::Message::Type::Message,
                ReleaseFn),
            pPoolItem(InitPoolItem)
        { }
    };

} // Acquire
} // SsTask

/////////////////////////////////////////////////////////////////////////////

#endif // Include_SSTASK_T_H

/////////////////////////////////////////////////////////////////////////////
