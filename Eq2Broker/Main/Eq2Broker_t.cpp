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
#include "DDUtil.h"

using namespace Broker;

Eq2Broker_t::Eq2Broker_t(
  MainWindow_t& mainWindow,
  const Game::Options_t& options)
  :
  mainWindow_(mainWindow),
  m_options(options),
  m_pImpl(std::make_unique<Eq2BrokerImpl_t>(mainWindow))
{
}

Eq2Broker_t::~Eq2Broker_t()
{
}

bool
Eq2Broker_t::
Initialize() {
  if (!InitHandlers()) {
    LogError(L"InitHandlers() failed");
    return false;
  }
  return true;
}

bool
Eq2Broker_t::
InitHandlers() {
  DP::PipelineManager_t& pm = GetPipelineManager();

  // Acquire

  pm.AddHandler(DP::Stage_t::Acquire, m_pImpl->m_SsWindow, L"SsWindow");

  // Identify

  pm.AddHandler(DP::Stage_t::Translate, m_pImpl->m_TrWindowType, L"IdWindowType");

  // Translate

  //pm.AddHandler(Translate, m_pImpl->m_TrScroll, s_pClass);
  pm.AddHandler(DP::Stage_t::Translate, m_pImpl->buyWindowManager_.GetTranslator(), L"BrokerBuy");

  // Interpret

  pm.AddHandler(DP::Stage_t::Interpret, m_pImpl->buyWindowManager_.GetInterpreter(), L"BrokerBuy");

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool Eq2Broker_t::Start() {
  LogInfo(L"Eq2Broker_t::Start()");
  extern DWORD g_dwSleep;
  if (0 < g_dwSleep) {
    LogAlways(L"Sleeping %d ms...", g_dwSleep);
    Sleep(g_dwSleep);
  }
  if (!m_options.testImagePath.empty()) {
    LoadAndSendTestImage(m_options.testImagePath);
  }
  else {
    constexpr size_t requiredTaskCount = 1; // 1 == SsTask
    auto startedTaskCount = GetPipelineManager().StartAcquiring();
    if (requiredTaskCount != startedTaskCount) {
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
    while (ReadConsoleCommand(buf, _countof(buf))) {
        try {
            if (!DispatchCommand(buf)) {
                return;
            }
        }
        catch (std::exception& e) {
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
    for (;;) {
        MSG msg;
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                return false; //??? retur
            }
            if (!IsWindow(hLog) || !IsDialogMessage(hLog, &msg)) {
                TranslateMessage(&msg) ;
                DispatchMessage(&msg) ;
            }
        } else { 
            DWORD count = 0;
            if (PeekConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &inp, 1, &count)) {
                if ((0 < count) && ReadConsoleCommand(buf, size)) {
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
    if (bValid) {
        buf[dwRead] = L'\0';
        for (--dwRead; iswspace(buf[dwRead]); --dwRead) {
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
    extern bool g_bWriteBmps;

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
    pool<CSurface>* pPool = new pool<CSurface>();
    pPool->reserve(1);
    pool<CSurface>::item_t item(pPool, pSurface);
    item.addref();
    pPool->add(item);
    m_pImpl->m_SsWindow.PostData(nullptr, pPool->get_unused());
 }

Ui::Window_t& Eq2Broker_t::GetWindow(Ui::WindowId_t windowId) {
  return mainWindow_.GetWindow(windowId);
}

