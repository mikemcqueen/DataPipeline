////////////////////////////////////////////////////////////////////////////////
//
// UTItemsDB.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DdUtil.h"
#include "DcrBrokerBuy.h"
#include "Log_t.h"
#include "Log.h"
#include "DbItems_t.h"
#include "Db.h"
#include "MainWindow_t.h"
#include "BrokerWindow.h"
#include "BrokerBuyWindow.h"
#include "AccountsDb.h"

extern bool g_bWriteBmps;

CDisplay*        g_pDisplay;
DWORD            g_dwSleep = 0;

////////////////////////////////////////////////////////////////////////////////

HRESULT
InitDirectDraw(
    HWND hWnd)
{
    static const int WINDOW_WIDTH   = 32;
    static const int WINDOW_HEIGHT  = 4;

	g_pDisplay = new CDisplay();
    return g_pDisplay->CreateWindowedDisplay(hWnd, WINDOW_WIDTH, WINDOW_HEIGHT, false);
}

////////////////////////////////////////////////////////////////////////////////

bool
StartupInitialize()
{
	if (!AfxWinInit(::GetModuleHandle(nullptr), nullptr, ::GetCommandLine(), 0))
	{
		wprintf(L"MFC initialization failed\n");
	    return false;
	}

    if (!Log_t::Get().Initialize())
    {
        wprintf(L"Log_t::Initialize() failed\n");
        return false;
    }
    if (!Log_t::Get().Open(L"UTItemsDb"))
    {
        wprintf(L"Log_t::Open() failed, %d\n", GetLastError());
        return false;
    }
    HRESULT hr = InitDirectDraw(GetDesktopWindow());
    if (FAILED(hr))
    {
        wprintf(L"InitDirectDraw failed. (%08x)\n", hr);
        return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////

void
FreeDirectDraw()
{
    if (nullptr != g_pDisplay)
    {
        delete g_pDisplay;
        g_pDisplay = nullptr;
    }
}

//////////////////////////////////////////////////////////////////////////////

int
wmain(
    int      argc,
    wchar_t* argv[])
{
    if (argc < 2)
    {
usage:
        _putws(L"usage: UTItemsDb [-d file.mdb] [-t1|-t2] [broker_table.bmp]");
        return 0;
    }

    bool bDbSupplied = false;
    int FileArg = 1;

    if (0 == wcscmp(argv[1], L"-d"))
    {
        if (argc < 4)
            goto usage;
        Db::SetDbName(argv[2]);
        bDbSupplied = true;
        FileArg = 3;
    }

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

    if (!bDbSupplied)
    {
        SYSTEMTIME t;
        GetLocalTime(&t);
        wchar_t DbName[MAX_PATH];
        if (!Db::CopyDb(t, DbName, _countof(DbName)))
            return -1;
        Db::SetDbName(DbName);
    }

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
    TextTable_t TextTable(Table::CharColumnWidths, Table::ColumnCount);
    Broker::DcrBase_t DcrBroker(&TextTable, static_cast<Broker::Buy::Window_t&>(mainWindow.GetBrokerBuyWindow()), Translate::Handler_t::s_ScreenTable);

//    BrokerBuy::Translate::Handler_t DcrBroker;
    DcrBroker.Initialize();

    g_bWriteBmps = true;

    Rect_t rc;
    surf.GetClientRect(&rc);
    bool b = 0;
    if (b && !DcrBroker.PreTranslateSurface(&surf, rc))
    {
        LogError(L"PreTranslateSurface() failed");
        return -1;
    }
    else
    {
        // TODO: pSurface->ReplacePixelIntensity(0, 50, Black);
        const COLORREF Black = RGB(0,0,0);
        surf.FixColor(rc, Broker::BkLowColor, Broker::BkHighColor, Black);
        rc.top += Broker::Table::GapSizeY;
    }

    RECT rcClient;
    surf.GetClientRect(&rcClient);
    surf.WriteBMP(L"diag\\dcrtest_file.bmp", rcClient);

    if (!DcrBroker.TranslateSurface(&surf, rc))
        return -1;
    TextTable.Dump(nullptr, true);

    using namespace Accounts::Db;
    size_t Row = 0;
    for (; Row < TextTable.GetEndRow(); ++Row)
    {
        const wchar_t *pItem = TextTable.GetRow(Row) + TextTable.GetColumnOffset(0);
        if (L'\0' != *pItem)
        {
            ItemId_t Id = Items_t::GetItemId(pItem);
            if (0 != Id)
            {
                LogInfo(L"Found '%ls' (%d)", pItem, Id);
            }
            else
            {
                Id = Items_t::AddItem(pItem);
                if (0 == Id)
                {
                    LogError(L"Error adding '%ls'", pItem);
                }
                else
                {
                    LogInfo(L"Added '%ls' (%d)", pItem, Id);
                }
            }
        }
    }
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
