////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// Eq2Broker_t.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Eq2Broker_t.h"
#include "Eq2BrokerImpl_t.h"
#include "PipelineManager.h"
#include "TransactionManager.h"
#include "Character_t.h"
#include "Log.h"
#include "Macros.h"
#include "BrokerId.h"
#include "MainWindow_t.h"
#include "Character_t.h"
#include "Log_t.h"
#include "SurfacePoolItem_t.h"

using namespace Broker;

////////////////////////////////////////////////////////////////////////////////
// Eq2Broker_t static definitions.

const wchar_t* Eq2Broker_t::s_pClass = nullptr; // L"Eq2Broker";

////////////////////////////////////////////////////////////////////////////////
//
// Constructor.
//

Eq2Broker_t::
Eq2Broker_t(
    Broker::MainWindow_t& mainWindow,
    const Broker::Options_t& options)
    :
    m_mainWindow(mainWindow),
    m_options(options),
    m_pImpl(std::make_unique<Eq2BrokerImpl_t>(*this, mainWindow))
{
}

////////////////////////////////////////////////////////////////////////////////

Eq2Broker_t::
~Eq2Broker_t()
{
}

////////////////////////////////////////////////////////////////////////////////

bool
Eq2Broker_t::
Initialize()
{
    if (!InitHandlers()) {
        LogError(L"InitHandlers() failed");
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
Eq2Broker_t::
InitHandlers()
{
    // TODO: objects responsible for adding themselves?
    DP::PipelineManager_t& pm = GetPipelineManager();
    using namespace DP::Stage;
    using namespace Broker::Transaction;
    pm.AddHandler(Acquire, m_pImpl->m_SsWindow, L"SsWindow");

    //
    // Translators
    //
    // NOTE: TrWindowType must be first translator
    pm.AddHandler(Translate, m_pImpl->m_TrWindowType, s_pClass);
    // NOTE: Scroll before window managers.
    //pm.AddHandler(Translate, m_pImpl->m_TrScroll, s_pClass);
    pm.AddHandler(Translate, m_pImpl->m_BuyWindow.GetTranslator(), s_pClass);

    //
    // Interpreters
    //
    pm.AddHandler(Interpret, m_pImpl->m_BuyWindow.GetInterpreter(), s_pClass);

    return true;
}

////////////////////////////////////////////////////////////////////////////////

MainWindow_t&
Eq2Broker_t::
GetMainWindow()
{
    return m_mainWindow;
}

////////////////////////////////////////////////////////////////////////////////

Ui::Window_t&
Eq2Broker_t::
GetWindow(
    Ui::WindowId_t windowId)
{
    return GetMainWindow().GetWindow(windowId);
}

////////////////////////////////////////////////////////////////////////////////

bool
Eq2Broker_t::
Start()
{
    LogInfo(L"Eq2Broker_t::Start()");
    extern DWORD g_dwSleep;
    if (0 < g_dwSleep)
    {
        LogAlways(L"Sleeping %d ms...", g_dwSleep);
        Sleep(g_dwSleep);
    }
    if (!m_options.testImagePath.empty()) {
        LoadAndSendTestImage(m_options.testImagePath);
    } else {
        constexpr size_t requiredTaskCount = 1; // 1 == SsTask
        auto startedTaskCount = GetPipelineManager().StartAcquiring();
        if (requiredTaskCount != startedTaskCount)
        {
            LogError(L"Only (%d) of (%d) acquire handler(s) started",
                startedTaskCount, requiredTaskCount);
            return false;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

void
Eq2Broker_t::
Stop()
{
    LogInfo(L"Eq2Broker_t::Stop()");
    DP::Event::StopAcquire_t EventStop;
    GetPipelineManager().SendEvent(EventStop);
}

////////////////////////////////////////////////////////////////////////////////

void
Eq2Broker_t::
ReadConsoleLoop()
{
    const size_t ConsoleBufSize = 256;
    wchar_t buf[ConsoleBufSize];
    while (ReadConsoleCommand(buf, _countof(buf)))
//    while (CommandLoop(buf, _countof(buf)))
    {
        try
        {
            if (!DispatchCommand(buf))
            {
                return;
            }
        }
        catch (std::exception& e)
        {
            LogError(L"Eq2Broker_t::ReadConsoleLoop() Exception: %hs", e.what());
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

bool
Eq2Broker_t::
CommandLoop(wchar_t* buf, DWORD size)
{
    HWND hLog = Log_t::Get().GetLogWindow().GetHwnd();
    INPUT_RECORD inp;
    for (;;)
    {
        MSG msg;
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                return false; //??? retur
            }
            if (!IsWindow(hLog) || !IsDialogMessage(hLog, &msg)) 
            {
                TranslateMessage(&msg) ;
                DispatchMessage(&msg) ;
            }
        }
        else
        { 
            DWORD count = 0;
            if (PeekConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &inp, 1, &count))
            {
                if ((0 < count) && ReadConsoleCommand(buf, size))
                {
                    return true;
                }
            }
        } 
    }
} 

////////////////////////////////////////////////////////////////////////////////

bool
Eq2Broker_t::
ReadConsoleCommand(TCHAR buf[], DWORD dwSize)
{
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    DWORD dwRead;
    bool bValid = FALSE != ReadConsole(h, buf, dwSize, &dwRead, nullptr);
    if (bValid)
    {
        buf[dwRead] = L'\0';
        for (--dwRead; iswspace(buf[dwRead]); --dwRead)
        {
            buf[dwRead] = L'\0';
        }
    }
    return bValid;
}

////////////////////////////////////////////////////////////////////////////////

bool
Eq2Broker_t::
DispatchCommand(
    const wchar_t* buf)
{
    LogAlways(L"CMD: %ls", buf);
    bool bHandled = true;
    bool bCmd = false;
    switch(buf[0])
    {
//    case L'b': bCmd = CmdBuilder(&buf[1]); break;
//    case L'c': bCmd = CmdCharacter(&buf[1]);   break;
//    case L'k': bCmd = CmdControl(&buf[1]); break;
    case L'l':
        {
            int iLevel = _wtoi(&buf[1]);
            Log::SetLevel(iLevel);
            LogAlways(L"LogLevel=%d", iLevel);
            bCmd = true;
        }
        break;
//    case L't': bCmd = CmdTransaction(&buf[1]); break;
//  case L'?': ShowHelp(commands);         break;
    case L'.' : return false;
    default:   bHandled = false;           break;
    }
    if (!bHandled)
        LogError(L"Unknown command (%s)", buf);
    else if (!bCmd)
        LogError(L"Command failed (%s)", buf);
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
Eq2Broker_t::
CmdControl(
    const wchar_t* pszCmd)
{
    extern bool       g_bWriteBmps;

    size_t Pos = 0;
    switch (pszCmd[Pos++])
    {
    case L'b':  // toggle g_bWriteBmps
        g_bWriteBmps = !g_bWriteBmps;
        LogAlways(L"WriteBmps=%d", g_bWriteBmps);
        return true;
    case L'k':  // toggle clicking
        {
            bool bClick = m_pImpl->m_SsWindow.ToggleClick();
            LogAlways(L"Click=%d", int(bClick));
        }
        return true;
    case L'.': //test whatever
        break;
    default:
        break;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
// TODO: Misc.cpp
const wchar_t*
GetCoinString(
    size_t   Value,
    wchar_t* pBuffer,
    size_t   BufferCount)
{
    static wchar_t Buffer[32];
    if (nullptr == pBuffer)
    {
        pBuffer = Buffer;
        BufferCount = _countof(Buffer);
    }
    else if (0 == BufferCount)
    {
        throw std::invalid_argument("GetCoinString()");
    }
    pBuffer[0] = L'\0';
    size_t Plat = Value / 100;
    size_t Gold = Value % 100;
    if (0 < Plat)
    {
        swprintf_s(pBuffer, BufferCount, L"%dp", Plat);
        if (0 < Gold)
        {
            wcscat_s(pBuffer, BufferCount, L",");
        }
    }
    if (0 < Gold)
    {
        wchar_t szGold[8];
        swprintf_s(szGold, L"%dg", Gold);
        if (0 < Plat)
        {
            wcscat_s(pBuffer, BufferCount, szGold);
        }
        else
        {
            wcscpy_s(pBuffer, BufferCount, szGold);
        }
    }
    if (0 == Value)
    {
        wcscpy_s(pBuffer, BufferCount, L"0c");
    }
    return pBuffer;
}

////////////////////////////////////////////////////////////////////////////////

Ui::WindowId_t
Eq2Broker_t::
GetWindowId(
    const CSurface& surface)
{
    return m_pImpl->m_TrWindowType.GetWindowId(surface);
}

////////////////////////////////////////////////////////////////////////////////

void
Eq2Broker_t::
LoadAndSendTestImage(const wstring& testImagePath)
{
    LogInfo(L"Loading and processing test image: %s", testImagePath.c_str());

    extern CDisplay* g_pDisplay;

    CSurface* pSurface = new CSurface();
    HRESULT hr = g_pDisplay->CreateSurfaceFromBitmap(pSurface, testImagePath.c_str());
    if (FAILED(hr)) {
        throw invalid_argument("CreateSurfaceFromBitmap failed");
    }
    // TODO: pool leaks.
    SurfacePool_t* pPool = new SurfacePool_t();
    pPool->reserve(1);
    pool<CSurface>::item_t item(pPool, pSurface);
    item.addref();
    pPool->add(item);
    m_pImpl->m_SsWindow.PostData(nullptr, pPool->get_unused());
    /*
    WaitForSingleObject(GetPipelineManager().GetIdleEvent(), INFINITE);
    switch (WindowId)
    {
    case Window::Id::BrokerBuyTab:
        BrokerBuy.GetTranslator().GetText().Dump(nullptr, true);
        if (0 == iterations)
            Window.DumpWidgets(Surface, BrokerBuy.GetWindow().GetTableRect());
        break;
    case Window::Id::BrokerSellTab:
        BrokerSell.GetTranslator().GetText().Dump(nullptr, true);
        if (0 == iterations)
            Window.DumpWidgets(Surface, BrokerSell.GetWindow().GetTableRect());
        break;
    case Window::Id::BrokerSetPricePopup:
        if (0 == iterations)
            Window.DumpWidgets(Surface, MainWindow.GetPopupRect());
        break;
    default:
        break;
    }
    LogAlways(L"VScroll = %ls", GetScrollPosString(Window.GetScrollPosition(Ui::Scroll::Bar::Vertical)));
    */
}