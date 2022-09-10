// DcrTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DdUtil.h"
#include "DcrBrokerBuy.h"
#include "Log_t.h"
#include "Log.h"
#include "MainWindow_t.h"
#include "BrokerBuy.h"

extern bool g_bWriteBmps;

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

bool
StartupInitialize()
{
#if LOGFILE
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
#endif
    HRESULT hr = InitDirectDraw(GetDesktopWindow());
    if (FAILED(hr))
    {
        wprintf(L"InitDirectDraw failed. (%08x)\n", hr);
        return false;
    }
    return true;
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

int
wmain(
    int      argc,
    wchar_t* argv[])
{
    if (argc < 2)
    {
usage:
        _putws(L"usage: UTBuyTable [-1] broker_table.bmp");
        _putws(L"      -1: 1 line of text, not a table");
        return 0;
    }

    bool fTable = true;
    int FileArg = 1;
    if (0 == wcscmp(argv[FileArg], L"-1"))
    {
        fTable = false;
        FileArg++;
    }
/*    else
    {
        wprintf(L"Unknown switch: %ls\n", argv[FileArg]);
        exit(-1);
    }
*/
    if (FileArg >= argc)
        goto usage;

    if (!StartupInitialize())
        return -1;

    struct Cleanup_t
    {
        Cleanup_t() {}
        ~Cleanup_t()
        {
            FreeDirectDraw();
            Log_t::Get().Shutdown();
        }
    } Cleanup;

    Log::SetLevel(LOGINFO);

    std::auto_ptr<CSurface> spSurface(new CSurface());
    HRESULT hr = g_pDisplay->CreateSurfaceFromBitmap(spSurface.get(), argv[FileArg]);
    if (FAILED(hr))
    {
        wprintf(L"CreateSurfaceFromBitmap (%ls) failed (%08x)\n", argv[FileArg], hr);
		return -1;
    }
    CSurface& surf = *spSurface.get();

    Broker::MainWindow_t mainWindow(true);
    using namespace Broker::Buy;
    using Broker::Buy::TextTable_t;
    TextTable_t     TextTable(Table::CharColumnWidths, Table::ColumnCount);
    Broker::DcrBase_t DcrBroker(&TextTable,
                                static_cast<Window_t&>(mainWindow.GetBrokerBuyWindow()),
                                Translate::Handler_t::s_ScreenTable);

//  Translate::Handler_t DcrBroker;
    DcrBroker.Initialize();

    g_bWriteBmps = true;

    Rect_t rc;
    surf.GetClientRect(&rc);
    // TODO: pSurface->ReplacePixelIntensity(0, 50, Black);
    bool b = 0;
    if (b && !DcrBroker.PreTranslateSurface(&surf, rc))
    {
        LogError(L"PreTranslateSurface() failed");
        return -1;
    }
    else
    {
        const COLORREF Black = RGB(0,0,0);Black;

//        surf.FixColor(rc, Broker::BkLowColor, Broker::BkHighColor, Black);
        rc.top += Broker::Table::GapSizeY;
    }

    RECT rcClient;
    surf.GetClientRect(&rcClient);
    surf.WriteBMP(L"diag\\dcrtest_file.bmp", rcClient);

    if (!fTable)
    {
        DWORD flags = DCR_GETTEXT_ALLOW_BAD |
                      DCR_GETTEXT_MAX_TRANS_COLOR;
        wchar_t buf[256] = L"";
        DcrBroker.GetText(&surf, &rc, buf, _countof(buf), 
                          *DcrBroker.GetCharsets().begin(), flags);
       wprintf(L"Chars: '%ls'\n", buf);
    }
    else
    {
        if (!DcrBroker.TranslateSurface(&surf, rc))
            return -1;
        TextTable.Dump(nullptr, true);
    }
	return 0;
}

