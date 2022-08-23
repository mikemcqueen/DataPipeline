/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// SsTask_t.cpp
//
// Screen Shot task
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SsTask_t.h"
#include "PipelineManager.h"
#include "DpEvent.h"
#include "DdUtil.h"
#include "Log.h"
#include "Macros.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////

DP::MessageId_t SsTask_t::s_MessageId = DP::Message::Id::Unknown;

/////////////////////////////////////////////////////////////////////////////

SsTask_t::
SsTask_t(
    CDisplay&   Display,
    size_t      SurfaceWidth,
    size_t      SurfaceHeight,
    size_t      PoolSize,
    size_t      DelayMs)
:
    m_Display(Display),
    m_SurfaceWidth(SurfaceWidth),
    m_SurfaceHeight(SurfaceHeight),
    m_PoolSize(PoolSize),
    m_DelayMs(DelayMs),
	m_dwThreadId(0),
	m_lSuspended(1),
    m_lSuspendCount(1),
	m_lEventPending(0),
    m_EventCount(0)
{
	m_szTestSurface[0] = L'\0';
}

/////////////////////////////////////////////////////////////////////////////

SsTask_t::
~SsTask_t()
{
    if (NULL != m_hTimer.get())
    {
        CancelWaitableTimer(m_hTimer.get());
    }
	if (NULL != m_hThread.get())
	{
        if (!IsSuspended())
            Suspend();
        SignalObjectAndWait(m_hExitEvent.get(), m_hThread.get(), INFINITE, FALSE);
	}
    // We've shut down the SS thread.  There may still be outstanding
    // pool items sitting in the PipelineManager queue waiting for 
    // translation, or being actively translated by the DCR thread.
    if (!m_Pool.all_unused())
    {
        LogError(L"~SsTask_t(): Used SurfacePoolItems...");
    }
	for (size_t iSurface = 0; iSurface < m_Pool.size(); ++iSurface)
	{
        SurfacePoolItem_t& t = m_Pool.at(int(iSurface));
		delete t.get();
	}
}

/////////////////////////////////////////////////////////////////////////////

bool
SsTask_t::
Initialize(
    const wchar_t* pszClass)
{
    LogInfo(L"SsTask_t::Initialize()");

    if (!DP::Handler_t::Initialize(pszClass))
        return false;

    //TODO: m_MessageId = GetPipelineManager().RegisterMessage(L"Screenshot");
    s_MessageId = DP::Message::Id::Screenshot;

    HRESULT hr;
    hr = InitSurfacePool(
            m_Display,
            m_Pool,
            m_SurfaceWidth,
            m_SurfaceHeight,
            m_PoolSize);
    if (FAILED(hr))
    {
        LogError(L"InitSurfacePool(%d) failed.", m_PoolSize);
        return false;
    }

	m_hTimer = CreateWaitableTimer(0, FALSE, 0);
    if (NULL == m_hTimer.get())
        return false;

	m_hExitEvent = CreateEvent(0, TRUE, FALSE, 0);
    if (NULL == m_hExitEvent.get())
        return false;

	m_hSuspendEvent = CreateEvent(0, FALSE, FALSE, 0);
    if (NULL == m_hSuspendEvent.get())
        return false;

	m_hSuspendNotifyEvent = CreateEvent(0, FALSE, FALSE, 0);
    if (NULL == m_hSuspendNotifyEvent.get())
        return false;

	m_hEvent = CreateEvent(0, FALSE, FALSE, 0);
    if (NULL == m_hEvent.get())
        return false;

	m_hThread = util::CreateThread(0, 0, ThreadFunc, (void *)this, 0, &m_dwThreadId);
    if (NULL == m_hThread.get())
        return false;

    return true;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
SsTask_t::
InitSurfacePool(
    CDisplay&     Display,
    SurfacePool_t& Pool,
    size_t        cx,
    size_t        cy,
    size_t        Count)
{
	Pool.reserve(Count);
	for (size_t Surface=0; Surface < Count; ++Surface)
	{
		CSurface* pSurface = new CSurface();
        HRESULT hr = Display.CreateSurface(pSurface, int(cx), int(cy));
        if (FAILED(hr))
        {
            LogError(L"CreateSurface(%d,%d) failed. (0x%08x)", cx, cy, hr);
            if (NULL != pSurface)
                delete pSurface;
            return hr;
        }
		SurfacePoolItem_t Item(&Pool, pSurface);
	    Pool.add(Item);
	}
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

#pragma warning(disable:4063)

HRESULT
SsTask_t::
EventHandler(
    DP::Event::Data_t& Data)
{
    HRESULT hr = DP::Handler_t::EventHandler(Data);
    if (S_FALSE != hr)
        return hr;
    switch (Data.Id)
    {
    case DP::Event::Id::Start:
        Start();
        break;

    case DP::Event::Id::Stop:
        Stop();
        break;

    default:
        return S_FALSE;
    }
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

bool
SsTask_t::
Start()
{
    LogInfo(L"SsTask_t::Start()");
    Resume();
    return true;
}

/////////////////////////////////////////////////////////////////////////////

void
SsTask_t::
Stop()
{
    LogInfo(L"SsTask_t::Stop()");
    Suspend();
}

/////////////////////////////////////////////////////////////////////////////

void
SsTask_t::
Suspend()
{
    LONG lCount = InterlockedIncrement(&m_lSuspendCount);
    LogInfo(L"SsTask_t::Suspend(%d)", lCount);
    if (1 == lCount)
    {
    #ifdef _DEBUG
        bool b =
    #endif
        SetSuspended(true);
        ASSERT(b);
        if (!SameThread())
        {
            SignalObjectAndWait(m_hSuspendEvent.get(), m_hSuspendNotifyEvent.get(), INFINITE, FALSE);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void
SsTask_t::
Resume()
{
    LONG lCount = InterlockedDecrement(&m_lSuspendCount);
    LogInfo(L"SsTask_t::Resume(%d)", lCount);
    if (0 == lCount)
    {
    #ifdef _DEBUG
        bool b =
    #endif
        SetSuspended(false);
        ASSERT(b);
    }
    else if (0 > lCount)
    {
        LogError(L"SsTask_t::Resume() mismatched, (%d)\n", lCount);
        throw logic_error("SsTask_t::Resume()");
    }
}

/////////////////////////////////////////////////////////////////////////////

bool
SsTask_t::
SetSuspended(
    bool bSuspend)
{
    LONG lValue = bSuspend ? 1 : 0;
    bool bPrev = (1 == InterlockedExchange(&m_lSuspended, lValue));
    return bSuspend ^ bPrev;
}

/////////////////////////////////////////////////////////////////////////////

bool
SsTask_t::
IsSuspended() const
{
	return 1 == InterlockedCompareExchange(&m_lSuspended, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////

void
SsTask_t::
ClearEventData() 
{
    m_EventData.clear();
    m_EventCount = 0;
}

/////////////////////////////////////////////////////////////////////////////

void
SsTask_t::
AddEventData(
    const DP::Event::Data_t* pData)
{
    ASSERT(NULL != pData);
    if ((0 == pData->Size) || (MaxEventDataSize < pData->Size))
    {
        throw invalid_argument("SsTask_t::AddEventData() invalid size");
    }
    size_t Size = m_EventData.size();
    m_EventData.resize(Size + pData->Size);
    memcpy(&m_EventData[Size], pData, pData->Size);
    ++m_EventCount;
}

/////////////////////////////////////////////////////////////////////////////

void
SsTask_t::
SetEventData(
    const DP::Event::Data_t& Data)
{
    if ((0 == Data.Size) || (MaxEventDataSize < Data.Size))
    {
        throw invalid_argument("SsTask_t::SetEventData() invalid size");
    }
    m_EventData.resize(Data.Size);
    memcpy(&m_EventData[0], &Data, Data.Size);
    m_EventCount = 1;
}

/////////////////////////////////////////////////////////////////////////////

const DP::Event::Data_t&
SsTask_t::
PeekEvent(
    size_t Event) const
{
#if 0
    ASSERT(0 != m_EventData.size());
    ASSERT(m_EventCount > Event);
    ASSERT(0 < Size);
#else
    if (m_EventData.empty() || (Event >= m_EventCount))
    {
        LogError(L"SsTask_t::PeekEvent(): EventData empty (%d), or invalid index (%d)",
            m_EventData.size(), Event);
        throw logic_error("SsTask_t::PeekEvent(): EventData empty, or invalid index");
    }
#endif
    size_t pos = 0;
    while (0 < Event--)
    {
        const DP::Event::Data_t&
            data = reinterpret_cast<const DP::Event::Data_t&>(m_EventData[pos]);
        pos += data.Size;
    }
    return reinterpret_cast<const DP::Event::Data_t&>(m_EventData[pos]);
}

/////////////////////////////////////////////////////////////////////////////

void
SsTask_t::
GetEventData(
    size_t             Event,
    DP::Event::Data_t& Data,
    size_t             Size) const
{
#if 0
    ASSERT(0 != m_EventData.size());
    ASSERT(m_EventCount > Event);
    ASSERT(0 < Size);
    size_t Pos = 0;
    while (0 < Event--)
    {
        const DP::Event::Data_t* pData =
            reinterpret_cast<const DP::Event::Data_t*>(&m_EventData[Pos]);
        Pos += pData->Size;
    }
//    return *reinterpret_cast<const DP::Event::Data_t*>(&m_EventData[Pos]);
//    return *(new (&m_EventData[Pos]) DP::Event::Data_t());
    memcpy(&Data, &m_EventData[Pos], Size);
#else
    const DP::Event::Data_t& data = PeekEvent(Event);
    if (data.Size != Size)
    {
        LogError(L"SsTask_t::GetEventData(): Event size mismatch (%d,%d)",
            Size, data.Size);
        throw logic_error("SsTask_t::GetEventData(): Event size mismatch");
    }
    memcpy(&Data, &data, Size);
#endif
}

/////////////////////////////////////////////////////////////////////////////

bool
SsTask_t::
SetEventPending(
    bool bPending)
{
    bool bPrevious = (1 == InterlockedExchange(&m_lEventPending, bPending ? 1 : 0));
    // return true if previous state is different than current state
    return bPending ^ bPrevious;
}

/////////////////////////////////////////////////////////////////////////////

bool
SsTask_t::
IsEventPending() const
{
	return 1 == InterlockedCompareExchange(&m_lEventPending, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////

void
SsTask_t::
SuspendAndFlush()
{
    Suspend();
    GetPipelineManager().Flush(DP::Stage::Acquire | DP::Stage::Translate,
                               GetClass().c_str());
}

/////////////////////////////////////////////////////////////////////////////

DWORD
WINAPI
SsTask_t::
ThreadFunc(
    void* pvParam)
{
    enum
    {
        Exit = 0,
        Suspend,
        Event,
        Timer,
        cHandles,
    };
    HANDLE aHandles[cHandles] = { 0 };

	SsTask_t* pClass = reinterpret_cast<SsTask_t*>(pvParam);
	util::SetWaitableTimer(pClass->m_hTimer.get(), (DWORD)pClass->m_DelayMs, true);

	aHandles[Exit]    = pClass->m_hExitEvent.get();
	aHandles[Suspend] = pClass->m_hSuspendEvent.get();
    aHandles[Event]   = pClass->m_hEvent.get();
	aHandles[Timer]   = pClass->m_hTimer.get();

	for (;;)
	{
		DWORD dw = WaitForMultipleObjects(cHandles, aHandles, FALSE, INFINITE);
		switch (dw)
		{
		case WAIT_OBJECT_0 + Exit:
            LogInfo(L"SsTask_t::ThreadFunc()::Exit");
			util::ExitThread(0);
			break;

		case WAIT_OBJECT_0 + Suspend:
            LogInfo(L"SsTask_t::ThreadFunc()::Suspend");
			SetEvent(pClass->m_hSuspendNotifyEvent.get());
			break;

        case WAIT_OBJECT_0 + Event:
            LogInfo(L"SsTask_t::ThreadFunc()::Event");
            pClass->ThreadProcessEvent();
            break;

		case WAIT_OBJECT_0 + Timer:
            if (!pClass->IsSuspended())
            {
                LogInfo(L"SsTask_t::ThreadFunc()::Timer");
                pClass->Shutter();
            }
            break;

        default:
            LogInfo(L"SsTask_t::ThreadFunc()::Wait returned %u", dw);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

// TODO: bool
void
SsTask_t::
Shutter()
{
	SurfacePoolItem_t* pPoolItem = GetAvailableSurface();
    if (NULL == pPoolItem) {
        LogWarning(L"SSTask_t::Shutter(): No surface available");
        return;
    }
        
    struct AutoRelease_t
    {
        SurfacePoolItem_t* pPoolItem;

        AutoRelease_t(SurfacePoolItem_t* pItem) : pPoolItem(pItem) {}
        ~AutoRelease_t() { pPoolItem->release(); }
    } AutoRelease(pPoolItem);
    RECT rc;
    HWND hWnd = GetSsWindowRect(rc);
    if (NULL != hWnd) {
        CSurface* pSurface = pPoolItem->get();
        if (TakeSnapShot(hWnd, rc, pSurface))
        {
            pPoolItem->set_state(PF_READY);
            PostData(hWnd, pPoolItem);
        }
    } else {
        LogWarning(L"SSTask_t::Shutter(): NULL hWnd");
    }
}


/////////////////////////////////////////////////////////////////////////////

SurfacePoolItem_t*
SsTask_t::
GetAvailableSurface( void )
{
    return m_Pool.get_unused();
}

/////////////////////////////////////////////////////////////////////////////

bool
SsTask_t::
TakeSnapShot(
    HWND        hWnd,
    const RECT& rc,
    CSurface*   pSurface)
{
    ASSERT(NULL != hWnd);
	if (NULL != hWnd)
	{
		if(SUCCEEDED(pSurface->BltWindow(hWnd, &rc)))
        {
			return true;
        }
        LogError(L"SsTask_t::TakeSnapShot(): BltWindow failed (%d)", GetLastError());
	}
	else if( m_szTestSurface[0] )
	{
		wchar_t szPath[MAX_PATH];
		wsprintf( szPath, L"%s\\%s", L"screens", m_szTestSurface );
	    pSurface->DrawBitmap( szPath, 0, 0 );
		m_szTestSurface[0] = 0;
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////

void
SsTask_t::
SetTestSurface(
    wchar_t* pszPath)
{
	ASSERT(NULL != pszPath);
	wcscpy_s(m_szTestSurface, sizeof(m_szTestSurface), pszPath);
}

/////////////////////////////////////////////////////////////////////////////
