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
#include "DcrBase_t.h"
#include "AccountManager_t.h"
#include "Character_t.h"
#include "AccountsDb.h"

///////////////////////////////////////////////////////////////////////////////

static const wchar_t g_szLogPrefix[] = L"eq2";

static CWinApp   theApp;
/*static*/ wchar_t   g_szDbName[MAX_PATH];
/*static*/ wchar_t   g_szCharName[MAX_PATH] = L"Human";
/*static*/ wchar_t   g_szServerName[MAX_PATH] = L"Mistmoore";

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
    if (NULL != g_pDisplay)
    {
        delete g_pDisplay;
        g_pDisplay = NULL;
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
    wchar_t* argv[])
{
    bool bDbSupplied = false;
    int c;

    while ((c = util::getopt (argc, argv, L"c:d:l:s:p:")) != -1)
    {
        switch (wchar_t(c))
        {
        case L'c': // Character:
            wcscpy_s(g_szCharName, optarg);
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
        case L's': // Server:
            wcscpy_s(g_szServerName, optarg);
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

void
StartupInitialize(
    int      argc,
    wchar_t* argv[])
{
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
        throw logic_error("MFC initialization failed");
	}
	srand((unsigned)time(NULL));
    Log::SetOutput(Log::Output::Debug);
    if (!Log_t::Get().Initialize())
    {
        throw logic_error("Log_t::Initialize() failed");
    }
    if (!Log_t::Get().Open(g_szLogPrefix))
    {
        throw logic_error("Log_t::Open() failed");
    }
    HRESULT hr;
    hr = InitDirectDraw(GetDesktopWindow());
    if (FAILED(hr))
    {
        throw logic_error("InitDirectDraw() failed");
    }
    if (!GetPipelineManager().Initialize())
    {
        throw logic_error("GetPipelineManager().Initialize() failed");
    }
    int iRet = ProcessCommandLine(argc, argv);
    if (0 >= iRet)
    {
        if (0 > iRet)
        {
            throw logic_error("ProcessCommandLine() failed.");
        }
        exit(iRet);
    }
#if 0
    if (!Accounts::Db::Initialize(m_pImpl->m_strServerName.c_str()))
    {
        throw logic_error("Accounts::Db::Initialize() failed");
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////

void
ShutdownCleanup()
{
    GetPipelineManager().Shutdown();
    FreeDirectDraw();
    Log_t::Get().Shutdown();
}

////////////////////////////////////////////////////////////////////////////////

void
BrokerLoop(
    const wchar_t* pServerName)
{
    // TODO: account move to broker class?
    AccountManager_t am;
    Account_t& acct = am.GetAccount(Game::Id::Eq2);
    Character_t& chr = acct.GetCharacter(0, g_szCharName);    // 0 == serverid
    Character_t::SetCharacter(&chr);
    LogAlways(L"Server: %s - Char: %s", pServerName, chr.GetName().c_str());
    // move to broker.init()?
    if (!Accounts::Db::Initialize(pServerName))
    {
        throw logic_error("AccountsDb::Initialize() failed");
    }
    Broker::MainWindow_t window;
    Eq2Broker_t broker(window);
    if (broker.Initialize())
    {
        if (broker.Start())
        {
            broker.ReadConsoleLoop();
            broker.Stop();
            return;
        }
        throw logic_error("broker.Start() failed");
    }
    throw logic_error("broker.Initialize() failed");
}

////////////////////////////////////////////////////////////////////////////////

int
wmain(
    int      argc,
    wchar_t* argv[],
    wchar_t* /*envp[]*/)
{
    struct Cleanup_t
    {
        ~Cleanup_t() { ShutdownCleanup(); }
    } Cleanup;

    extern bool g_bTableFixColor;
    g_bTableFixColor = false;

    try
    {
        StartupInitialize(argc, argv);
        BrokerLoop(g_szServerName);
    }
    catch (std::exception& e)
    {
        LogError(L"wmain() ### Caught %hs: %hs ###", typeid(e).name(), e.what());
    }
    catch (CDBException* e)
    {
        LogError(L"wmain() ### Caught CDBException: %s ###", e->m_strError);
        e->Delete();
    }
    catch (...)
    {
        LogError(L"wmain() ### Unhandled exception ###");
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
