
//
// UTGetWindowType.cpp
//

#include "stdafx.h"
#include "DdUtil.h"
#include "BrokerBuyTypes.h"
#include "Log_t.h"
#include "Log.h"
#include "MainWindow_t.h"
#include "TrWindowType.h"
#include "BrokerUi.h"
#include "BrokerWindow.h"
#include "TabWindow.h"

bool g_bWriteBmps = true;

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
#define LOGFILE 1
#if LOGFILE
    if (!Log_t::Get().Initialize())
    {
        wprintf(L"Log_t::Initialize() failed\n");
        return false;
    }
    if (!Log_t::Get().Open(L"UTBuyScreen"))
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
//    if (!GetPipelineManager().Initialize())
//        return false;
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

typedef set<Ui::WidgetId_t> WidgetIdSet_t;

bool
GetWidgetIds(
    Ui::WindowId_t WindowId,
    WidgetIdSet_t& widgetIds)
{
    using namespace Broker;
    using namespace Broker::Window;
    Ui::WidgetId_t idFirst = 0;
    Ui::WidgetId_t idLast = 0;
    switch (WindowId)
    {
    case Broker::Window::Id::BrokerFrame:
    case Ui::Window::Id::Unknown:
    case Broker::Window::Id::Eq2Loading:
    case Broker::Window::Id::Zoning:
    case Broker::Window::Id::MainChat:
        return false;
/*
// old style login
    case Id::Login:
        idFirst = Login::Widget::Id::First;
        idLast = Login::Widget::Id::Last;
        return true;
*/
    case Id::Eq2Login:
        idFirst = Eq2Login::Widget::Id::First;
        idLast = Eq2Login::Widget::Id::Last;
        break;
    case Id::BrokerBuyTab:
        idFirst = Buy::Widget::Id::First; 
        idLast = Buy::Widget::Id::Last; 
        widgetIds.insert(Frame::Widget::Id::SellTab);
        widgetIds.insert(Frame::Widget::Id::SalesLogTab);
        break;
    case Id::BrokerSellTab:
        idFirst = Sell::Widget::Id::First;
        idLast = Sell::Widget::Id::Last;
        widgetIds.insert(Frame::Widget::Id::BuyTab);
        widgetIds.insert(Frame::Widget::Id::SalesLogTab);
        break;
    case Id::BrokerSalesLogTab:
        idFirst = SalesLog::Widget::Id::First;
        idLast = SalesLog::Widget::Id::Last;
        widgetIds.insert(Frame::Widget::Id::BuyTab);
        widgetIds.insert(Frame::Widget::Id::SellTab);
        return false;
    case Id::BrokerSetPricePopup:
        idFirst = SetPrice::Widget::Id::First;
        idLast = SetPrice::Widget::Id::Last;
        break;
    default:
        return false;
    }
    for (Ui::WidgetId_t id = idFirst; id < idLast; ++id)
    {
        widgetIds.insert(id);
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////

void
Outline(
          CSurface& surface,
    const Rect_t&   widgetRect)
{
    surface.ColorFill(&widgetRect, RGB(255, 0, 0));
}

/////////////////////////////////////////////////////////////////////////////

int
wmain(
    int      argc,
    wchar_t* argv[])
{
    if (argc < 2)
    {
        _putws(L"usage: UTGetWindowType fullwindow.bmp");
        return 0;
    }

    int FileArg = 1;

    if (!StartupInitialize())
        return -1;

    struct Cleanup_t
    {
        Cleanup_t() {}
        ~Cleanup_t()
        {
//            GetPipelineManager().Shutdown();
            FreeDirectDraw();
            Log_t::Get().Shutdown();
        }
    } Cleanup;

    Log::SetLevel(LOGINFO);

    std::auto_ptr<CSurface> spSurface(new CSurface());
    HRESULT hr = g_pDisplay->CreateSurfaceFromBitmap(spSurface.get(), argv[FileArg]);
    if (FAILED(hr))
    {
        LogError(L"CreateSurfaceFromBitmap (%ls) failed (%08x)\n", argv[FileArg], hr);
		return -1;
    }
    CSurface& Surface = *spSurface.get();
    
try
{

    Broker::MainWindow_t Window(true);
    Broker::TrWindowType_t trWindowType(Window);
    LogInfo(L"First pass:");
    Ui::WindowId_t WindowId = trWindowType.GetWindowId(Surface);
    LogInfo(L"WindowId(%d) %ls", WindowId, trWindowType.GetWindowName(WindowId));

    LogInfo(L"Second pass:");
    WindowId = trWindowType.GetWindowId(Surface);
    LogInfo(L"WindowId(%d) %ls", WindowId, trWindowType.GetWindowName(WindowId));

    if (WindowId == Ui::Window::Id::Unknown)
    {
        return 0;
    }

#if 0
    const Ui::Window_t& originWindow = Window.GetWindow(WindowId);
    Rect_t origin;
    // Hack around the fact that we want BrokerWindow.Origin() if we're on buy/sell/setprice tab
    // only, this doesn't work because the real parent of buy/sell/setprice tabs is "MainWindow" not "BrokerWindow"
    if (&pOriginWindow->GetParent() != pOriginWindow)
    {
        pOriginWindow = &pOriginWindow->GetParent();
    }
//#else
    using namespace Broker::Window;
    switch (WindowId)
    {
    case Id::BrokerBuyTab:
    case Id::BrokerSellTab:
    case Id::BrokerSalesLogTab:
        origin = static_cast<const TableWindow_t&>(originWindow).GetTableRect();
        break;
    case Id::BrokerSetPricePopup:
        origin = Window.GetPopupRect();
        break;
    default :
        {
            const POINT& ptOrigin = originWindow.GetLastOrigin();
            SetRect(&origin, ptOrigin.x, ptOrigin.y, ptOrigin.x + 1, ptOrigin.y + 1);
        }
    }
#endif

    int widgetCount = 0;
    WidgetIdSet_t widgetIds;
    if (GetWidgetIds(WindowId, widgetIds) && !widgetIds.empty())
    {
        LogInfo(L"%d widgets found", widgetIds.size());
        WidgetIdSet_t::const_iterator it = widgetIds.begin();
        for (; widgetIds.end() != it; ++it)
        {
            Rect_t widgetRect;
            if (Window.GetWindow(WindowId).GetWidgetRect(*it, widgetRect))
            {
                Outline(Surface, widgetRect);
                ++widgetCount;
            }
        }
        LogInfo(L"%d widgets drawn", widgetCount);
        Surface.WriteBMP(L"diag\\UTGetWindowType.bmp", Surface.GetBltRect());
    }
    else
    {
        LogError(L"No widgets found!");
    }
}
catch(exception& e)
{
    LogError(L"### Caught %hs: %hs ###", typeid(e).name(), e.what());
}
	return 0;
}

