////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// Main.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Eq2Broker_t.h"
#include "PipelineManager.h"
#include "DdUtil.h"
#include "Log_t.h"
#include "Db.h"
#include "MainWindow_t.h"
//#include "DcrBase_t.h"
#include "AccountManager_t.h"
#include "Character_t.h"
#include "AccountsDb.h"
#include "TesseractDcrImpl_t.h"
#include "Dcr.h"

///////////////////////////////////////////////////////////////////////////////

static const wchar_t g_szLogPrefix[] = L"eq2";

static CWinApp   theApp;
/*static*/ wchar_t   g_szDbName[MAX_PATH];
///*static*/ wchar_t   g_szCharName[MAX_PATH] = L"Human";
///*static*/ wchar_t   g_szServerName[MAX_PATH] = L"Mistmoore";

CDisplay*        g_pDisplay;
DWORD            g_dwSleep = 0;

///////////////////////////////////////////////////////////////////////////////

HRESULT
InitDirectDraw(
    HWND hWnd)
{
    static const int WINDOW_WIDTH   = 32;
    static const int WINDOW_HEIGHT  = 4;

	g_pDisplay = new CDisplay();
    return g_pDisplay->CreateWindowedDisplay(hWnd, WINDOW_WIDTH, WINDOW_HEIGHT, false);
}

/////////////////////////////////////////////////////////////////////////////

void
FreeDirectDraw()
{
    if (nullptr != g_pDisplay)
    {
        delete g_pDisplay;
        g_pDisplay = nullptr;
    }
}

/////////////////////////////////////////////////////////////////////////////

void
InitLogLevel(
    wchar_t* arg)
{
    int Level = _wtoi(arg);
    Log::SetLevel(Level);
    LogAlways(L"LogLevel=%d", Level);
}

/////////////////////////////////////////////////////////////////////////////

extern int      optopt;
extern wchar_t* optarg;

int
ProcessCommandLine(
  int      argc,
  wchar_t* argv[],
  Game::Options_t* pOptions)
{
  bool bDbSupplied = false;
  int c;

  while ((c = util::getopt(argc, argv, L"c:d:l:p:s:t:")) != -1) {
    switch (wchar_t(c)) {
    case L'c': // Character name
      pOptions->characterName.assign(optarg);
      break;

    case L'd': // Database:
      wcscpy_s(g_szDbName, optarg);
      LogWarning(L"Command line DB is ignored for now");
      bDbSupplied = true;
      break;

    case L'l': // Log level:
      InitLogLevel(optarg);
      break;

    case L'p': // Pause: Sleep @ startup 
      g_dwSleep = _wtoi(optarg);
      if (0 < g_dwSleep)
      {
        Sleep(g_dwSleep * 1000);
      }
      break;

    case L's': // Server name
      pOptions->serverName.assign(optarg);
      break;

    case L't': // Test image path
      pOptions->testImagePath.assign(optarg);
      break;

    case L'?':
      return -1;

    default:
      break;
    }
  }
  return 1;
}

/////////////////////////////////////////////////////////////////////////////

void StartupInitialize(
  int argc,
  wchar_t* argv[],
  Game::Options_t* pOptions)
{
  //InitCommonControls(); // DrawShadowText

  if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0)) {
    throw runtime_error("MFC initialization failed");
  }
  srand(static_cast<unsigned>(time(nullptr)));
  Log::SetOutput(Log::Output::Debug);
  if (!Log_t::Get().Initialize()) {
    throw runtime_error("Log_t::Initialize() failed");
  }
  if (!Log_t::Get().Open(g_szLogPrefix)) {
    throw runtime_error("Log_t::Open() failed");
  }
  HRESULT hr = InitDirectDraw(GetDesktopWindow());
  if (FAILED(hr)) {
    throw runtime_error("InitDirectDraw() failed");
  }

  // Tesseract/DCR.
  TesseractDcrImpl_t::Init();

  if (!GetPipelineManager().Initialize()) {
    throw runtime_error("GetPipelineManager().Initialize() failed");
  }
  if (int result = ProcessCommandLine(argc, argv, pOptions); result <= 0) {
    if (result < 0) {
      throw invalid_argument("ProcessCommandLine() failed.");
    }
    exit(result);
  }
#if 0
  if (!Accounts::Db::Initialize(m_pImpl->m_strServerName.c_str())) {
    throw runtime_error("Accounts::Db::Initialize() failed");
  }
#endif
}

////////////////////////////////////////////////////////////////////////////////

void ShutdownCleanup() {
    GetPipelineManager().Shutdown();
    FreeDirectDraw();
    TesseractDcrImpl_t::Cleanup();
    Log_t::Get().Shutdown();
}

////////////////////////////////////////////////////////////////////////////////

void BrokerLoop(const Game::Options_t& options) {
  // TODO: account move to broker class?
/*
  AccountManager_t am;
  Account_t& acct = am.GetAccount(Game::Id::Eq2);
  Character_t& chr = acct.GetCharacter(0, options.characterName.c_str());    // 0 == serverid
  Character_t::SetCharacter(&chr);
  LogAlways(L"Server: %s - Char: %s", options.serverName.c_str(), chr.GetName().c_str());
  // move to broker.init()?
  if (!Accounts::Db::Initialize(options.serverName.c_str())) {
      throw runtime_error("AccountsDb::Initialize() failed");
  }
*/
  MainWindow_t mainWindow;
  Eq2Broker_t broker(mainWindow, options);
  if (!broker.Initialize()) {
    throw runtime_error("broker.Initialize() failed");
  }
  if (!broker.Start()) {
    throw runtime_error("broker.Start() failed");
  }
  broker.ReadConsoleLoop();
  broker.Stop();
}

////////////////////////////////////////////////////////////////////////////////

int wmain(
    int      argc,
    wchar_t* argv[],
    wchar_t* /*envp[]*/)
{
    struct Cleanup_t {
        ~Cleanup_t() { ShutdownCleanup(); }
    } cleanup;

    extern bool g_bTableFixColor;
    g_bTableFixColor = false;

    Game::Options_t options;
    try {
        StartupInitialize(argc, argv, &options);
        BrokerLoop(options);
    } catch (std::exception& e) {
        LogError(L"wmain() ### Caught %hs: %hs ###", typeid(e).name(), e.what());
    } catch (CDBException* e) {
        LogError(L"wmain() ### Caught CDBException: %s ###", (LPCTSTR)e->m_strError);
        e->Delete();
    } catch (...) {
        LogError(L"wmain() ### Unhandled exception ###");
    }
    return 0;
}
