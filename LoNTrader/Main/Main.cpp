////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// Main.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LonTrader_t.h"
#include "LonWindow_t.h"
#include "LonPlayer_t.h"
#include "PipelineManager.h"
#include "DdUtil.h"
#include "Log_t.h"
#include "Log.h"
#include "DcrTrades_t.h" // unnecessary if DpHandler_t::Shutdown()

extern bool DcrTest(wchar_t* arg, bool bTable);

///////////////////////////////////////////////////////////////////////////////

static CWinApp   theApp;
static wchar_t   g_szDbName[MAX_PATH];
static bool      g_bGetYourCards = true;

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

bool
StartupInitialize()
{
    //	SET_CRT_DEBUG_FIELD( _CRTDBG_DELAY_FREE_MEM_DF );
    //	SET_CRT_DEBUG_FIELD( _CRTDBG_CHECK_ALWAYS_DF );
    //	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_WNDW );
    //	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_WNDW );
    //	SetRegistryKey(_T("LonTrader"));

	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		wprintf(L"MFC initialization failed\n");
	    return false;
	}
	srand((unsigned)time(NULL));
    if (!Log_t::Get().Initialize())
    {
        wprintf(L"Log_t::Initialize() failed\n");
        return false;
    }
    if (!Log_t::Get().Open(L"lon"))
    {
        wprintf(L"Log_t::Open() failed, %d\n", GetLastError());
        return false;
    }
    HRESULT hr;
    hr = InitDirectDraw(GetDesktopWindow());
    if (FAILED(hr))
    {
        wprintf(L"InitDirectDraw() failed\n");
        return false;
    }
    if (!GetPipelineManager().Initialize())
        return false;

    return true;
}

////////////////////////////////////////////////////////////////////////////////

void
ShutdownCleanup()
{
    GetPipelineManager().Shutdown();
//    DcrTrades_t::ShutdownCleanup();
    FreeDirectDraw();
    Log_t::Get().Shutdown();
}

/////////////////////////////////////////////////////////////////////////////

void
InitLogLevel(
    wchar_t* arg)
{
    int Level = _wtoi(arg);
    Log_t::Get().SetLogLevel(Level);
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

    while ((c = util::getopt (argc, argv, L"b:d:l:s:t:y")) != -1)
    {
        switch (wchar_t(c))
        {
        case L'b': // Basic DcrTest
            DcrTest(optarg, false);
            return 0;
        case L'd': // database
            LonTrader_t::SetDbName(optarg);
            bDbSupplied = true;
            break;
        case L'l': // log level
            InitLogLevel(optarg);
            break;
        case L's': // Sleep @ startup 
            g_dwSleep = _wtoi(optarg);
            break;
        case L't': // Table DcrTest
            DcrTest(optarg, true);
            return 0;
        case L'y':
            LogAlways(L"Skipping GetYourCards...");
            g_bGetYourCards = false;
            break;
        case L'?':
            return -1;
        }
    }
    // may need to do this prior to DcrTest in the future
    if (!bDbSupplied)
    {
        SYSTEMTIME t;
        GetLocalTime(&t);
        wchar_t DbName[MAX_PATH];
        if (!LonTrader_t::CopyDb(t, DbName, _countof(DbName)))
            return -1;
        LonTrader_t::SetDbName(DbName);
    }
    return 1;
}

////////////////////////////////////////////////////////////////////////////////

int
wmain(
    int      argc,
    wchar_t* argv[],
    wchar_t* envp[])
{
    envp;
    struct Cleanup_t
    {
        ~Cleanup_t() { ShutdownCleanup(); }
    } Cleanup;

    try
    {
        // get all throwing stuff out of startup init
        if (!StartupInitialize())
        {
            _putws(L"StartupInitialize() failed.");
            return -1;
        }
        int iRet = ProcessCommandLine(argc, argv);
        if (0 >= iRet)
        {
            if (0 > iRet)
                LogError(L"ProcessCommandLine failed.");
            return iRet;
        }

        LonWindow_t Window;
        LonTrader_t Trader(Window, L"ChararcterName");
        if (!Trader.Initialize())
        {
            LogError(L"Trader.Initialize() failed");
            return -1;
        }
        if (!Trader.Start(g_szDbName, g_bGetYourCards))
        {
            LogError(L"LonTrader_t::Start() failed.");
            return -1;
        }
        Trader.ReadConsoleLoop();
        Trader.Stop();

        DcrTrades_t::Shutdown();
    }
    catch(std::exception& e)
    {
        LogError(L"### Caught %hs: %hs ###", typeid(e).name(), e.what());
    }
    catch(HRESULT hr)
    {
        LogError(L"### Caught HRESULT=0x%08x - terminating ###", hr);
    }
    catch(...)
    {
        LogError(L"### Unhandled exception ###");
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
