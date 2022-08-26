// DcrTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DdUtil.h"

#include "DcrBrokerBuy.h"
#include "BrokerBuyWindow.h"

#include "DcrBrokerSell.h"
#include "BrokerSellWindow.h"

#include "DcrSetPrice.h"
#include "SetPriceWindow.h"

#include "DcrEq2Login.h"
#include "Eq2LoginWindow.h"

#include "Log_t.h"
#include "Log.h"
#include "SurfacePoolItem_t.h"
#include "Pool.h"
#include "PipelineManager.h"
#include "MainWindow_t.h"
#include "TabWindow.h"
#include "TrWindowType.h"

#include "UiWindowManager.h"
#include "Timer_t.h"

// Declare bogus interpret handlers so we can instantiate a Window::Manager_t
// without having to include TiBroker* and all their dependencies

namespace Broker
{
    namespace Buy
    {
        namespace Interpret
        {
            class Handler_t { public: Handler_t(Window::ManagerBase_t&) {} };
        }
    }
    namespace Sell
    {
        namespace Interpret
        {
            class Handler_t { public: Handler_t(Window::ManagerBase_t&) {} };
        }
    }
    namespace SetPrice
    {
        namespace Interpret
        {
            class Handler_t { public: Handler_t(Window::ManagerBase_t&) {} };
        }
    }
}

///////////////////////////////////////////////////////////////////////////////


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

///////////////////////////////////////////////////////////////////////////////

bool
StartupInitialize()
{
#define LOGFILE 1
#if LOGFILE
    if (!Log_t::Get().Initialize())
    {
        wprintf(L"Log_t::Initialize() failed\n");
        return false;
    }
    if (!Log_t::Get().Open(L"UTScreenDcr"))
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
    if (!GetPipelineManager().Initialize())
        return false;
    return true;
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

const wchar_t*
GetScrollPosString(
    Ui::Scroll::Position_t ScrollPos)
{
    using namespace Ui::Scroll;
    switch (ScrollPos)
    {
    case Position::Unknown: return L"Unknown";
    case Position::Top:     return L"Top";
    case Position::Middle:  return L"Middle";
    case Position::Bottom:  return L"Bottom";
    default:                return L"Error";
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
        _putws(L"usage: UTScreenDcr [-# (iterations)] [-l# (loglevel)] [-b (writebmps)] [-f (fixtablecolor)] fullwindow.bmp");
        return 0;
    }

    if (!StartupInitialize())
        return -1;
    struct Cleanup_t
    {
        Cleanup_t() {}
        ~Cleanup_t()
        {
            GetPipelineManager().Shutdown();
            FreeDirectDraw();
            Log_t::Get().Shutdown();
        }
    } Cleanup;

    extern bool g_bTableFixColor;
    g_bTableFixColor = false;

    extern bool g_bWriteBmps;
    extern bool g_noDcrPost;
    g_noDcrPost = false;

    int FileArg = 1;
    size_t iterations = 1;
    int logLevel = LOGINFO;
    for (size_t option = 1; L'-' == argv[option][0]; ++option)
    {
        ++FileArg;
        if (argc <= FileArg)
        {
            goto usage;
        }
        if (iswdigit(argv[option][1]))
        {
            iterations = _wtoi(&argv[option][1]);
            if (0 == iterations)
            {
                iterations = 1;
            }
        }
        else {
            switch (argv[option][1])
            {
            case L'b':
                LogAlways(L"WriteBmps");
                g_bWriteBmps = true;
                break;
            case L'f':
                LogAlways(L"FixTableColor");
                g_bTableFixColor = true;
                break;
            case L'l':
                logLevel = _wtoi(&argv[option][2]);
                break;
            }
        }
    }

    Log::SetLevel(logLevel);

    std::auto_ptr<CSurface> spSurface(new CSurface());
    CSurface* pSurface = spSurface.get();
    HRESULT hr = g_pDisplay->CreateSurfaceFromBitmap(pSurface, argv[FileArg]);
    if (FAILED(hr))
    {
        LogError(L"CreateSurfaceFromBitmap (%ls) failed (%08x)\n", argv[FileArg], hr);
		return -1;
    }

    void Execute(CSurface& Surface, size_t iterations);
    Timer_t timer;
#if 1 || defined(NO_EXCEPT)
    try
#endif
    {
        Execute(*pSurface, iterations);
    }
#if 1 || defined(NO_EXCEPT)
    catch(std::exception& e)
    {
        LogError(L"### Caught %hs: %hs ###", typeid(e).name(), e.what());
    }
    catch(...)
    {
        wprintf(L"### unhandled exception ###\n");
    }
#endif
    timer.Show(NULL, true);
    pSurface->WriteBMP(L"diag\\UTScreenDcr_After.bmp");
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

void Execute(CSurface& Surface, size_t iterations)
{
    static const wchar_t Class[] = L""; // L"Eq2Broker";
    using namespace Broker;

    MainWindow_t MainWindow(true);
    Broker::TrWindowType_t trWindowType(MainWindow);
    Ui::WindowId_t WindowId = trWindowType.GetWindowId(Surface);
    if (Ui::Window::Id::Unknown == WindowId)
    {
        LogError(L"Unknown window");
        return;
    }
    Ui::Window_t& Window = MainWindow.GetWindow(WindowId);
    Rect_t TableRect;
    Tab_t Tab = Tab::None;
    switch (WindowId)
    {
    case Window::Id::BrokerBuyTab:  Tab = Tab::Buy; break;
    case Window::Id::BrokerSellTab: Tab = Tab::Sell; break;
    default: break;
    }
    if (Tab::None != Tab)
    {
        Window.UpdateScrollPosition(Ui::Scroll::Bar::Vertical, Surface);
    }
    SsWindow::Acquire::Handler_t SsWindow(*g_pDisplay, MainWindow);
    SsWindow.Initialize(Class);

    // hack to init charsets
    DcrBase_t::InitAllCharsets();

    Buy::Window::ManagerBase_t      BrokerBuy(MainWindow.GetBrokerBuyWindow());
    Sell::Window::ManagerBase_t     BrokerSell(MainWindow.GetBrokerSellWindow());
    SetPrice::Window::ManagerBase_t SetPrice(MainWindow.GetSetPricePopup());
    Eq2Login::Window::ManagerBase_t eq2Login(MainWindow.GetEq2LoginWindow());

    DP::PipelineManager_t& pm = GetPipelineManager();
    using namespace DP::Stage;
    pm.AddHandler(Translate, trWindowType, Class);
    pm.AddHandler(Translate, eq2Login.GetTranslator(), Class);
    pm.AddHandler(Translate, BrokerBuy.GetTranslator(), Class);
    pm.AddHandler(Translate, BrokerSell.GetTranslator(), Class);
    pm.AddHandler(Translate, SetPrice.GetTranslator(), Class);

    SurfacePool_t Pool;
    Pool.reserve(1);
    CSurface* pSurface = &Surface;
    SurfacePoolItem_t poolItem(&Pool, pSurface);
    poolItem.addref();
    Pool.add(poolItem);

    while (0 < iterations--)
    {
        SsWindow.PostData(NULL, &poolItem);
        WaitForSingleObject(GetPipelineManager().GetIdleEvent(), INFINITE);
        switch (WindowId)
        {
        case Window::Id::BrokerBuyTab:
            BrokerBuy.GetTranslator().GetText().Dump(NULL, true);
            if (0 == iterations)
                Window.DumpWidgets(Surface, BrokerBuy.GetWindow().GetTableRect());
            break;
        case Window::Id::BrokerSellTab:
            BrokerSell.GetTranslator().GetText().Dump(NULL, true);
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
    }
//    Sleep(1000);
    poolItem.release();
}