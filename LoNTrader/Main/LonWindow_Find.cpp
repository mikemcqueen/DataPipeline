/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// LonWindow_Find.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LonWindow_t.h"
#include "Log.h"

using namespace Lon;

////////////////////////////////////////////////////////////////////////////////

static const wchar_t AcceptButton[] = L"acceptButton";
static const wchar_t RejectButton[] = L"rejectButton";
static const wchar_t CloseButton[]  = L"closeButton";

/*
namespace PromptDialog
{
    static const wchar_t* Windows[] =
    {
        L"dialogFrame",
        L"PromptDialog",
    };

}

*/
namespace Prompt
{
namespace TradeBuilderExitPrompt
{
    static const wchar_t* Windows[] =
    {
        L"dialogFrame",
        L"PromptDialog",
    };
}

namespace SystemMessageWindow 
{
    static const wchar_t* Windows[] =
    {
        L"dialogFrame",
        L"SystemMessageDialog",
    };
}

namespace NetworkStatusWindow
{
    static const wchar_t* Windows[] =
    {
        L"dialogFrame",
        L"MessageBoxDialog",
    };
}

namespace AcceptTradeWindow
{
    static const wchar_t* Windows[] =
    {
        L"dialogFrame",
        L"PromptDialog",
    };
}

namespace DeliveryWindow
{
    static const wchar_t* Windows[] =
    {
        L"dialogFrame",
        L"DeliveryDialog",
    };
}

namespace JoinGuildWindow
{
    static const wchar_t* Windows[] =
    {
        L"dialogFrame",
        L"PromptDialog",
    };
}

namespace CancelTradeWindow
{
    static const wchar_t* Windows[] =
    {
        L"dialogFrame",
        L"PromptDialog",
    };
}
}


////////////////////////////////////////////////////////////////////////////////

HWND
LonWindow_t::
FindWindow(
    const Window::Type_e WindowType) //const
{
    using namespace Window;
    switch (WindowType)
    {
    case MainWindow:
        return ::FindWindow(NULL, _T("Legends of Norrath"));
    case PostedTradesWindow:
    case PostedTradesList:
    case PostedTradesView:
    case PostedTradesVScroll:
    case PostedTradesCreateButton:
        return FindPostedTradesWindow(WindowType);
    case PostedTradeDetailWindow:
    case PostedTradeDetailDialog:
    case PostedTradeDetailOfferedView:
    case PostedTradeDetailOfferedVScroll:
    case PostedTradeDetailWantView:
    case PostedTradeDetailWantVScroll:
    case PostedTradeDetailCancel:
    case PostedTradeDetailAccept:
        return FindPostedTradeDetailWindow(WindowType);
    case TradeBuilderWindow:
    case TradeBuilderCollectionFrame:
    case TradeBuilderCollectionTab:
    case TradeBuilderSearchEdit:
    case TradeBuilderSearchEditClear:
    case TradeBuilderYourTableList:
    case TradeBuilderYourTableView:
    case TradeBuilderYourTableVScroll:
    case TradeBuilderYourTableHScroll:
    case TradeBuilderTheirTableList:
    case TradeBuilderTheirTableView:
    case TradeBuilderTheirTableVScroll:
    case TradeBuilderTheirTableHScroll:
    case TradeBuilderClear:
    case TradeBuilderSubmit:
    case TradeBuilderExit:
        return FindTradeBuilderWindow(WindowType);
    case ConfirmTradeWindow:
    case ConfirmTradeYouGetView:
    case ConfirmTradeYouGetVScroll:
    case ConfirmTradeTheyGetView:
    case ConfirmTradeTheyGetVScroll:
    case ConfirmTradeCancel:
    case ConfirmTradeConfirm:
        return FindConfirmTradeWindow(WindowType);
/*
        CannotTradeWindow 
        CannotTradeOk,
*/
    case TradeBuilderExitPrompt:
    case TradeBuilderExitPromptYes:
    case SystemMessageWindow:
    case SystemMessageClose:
    case AcceptTradeWindow:
    case AcceptTradeYes:
    case AcceptTradeNo:
    case DeliveryWindow:
    case DeliveryClose:
    case NetworkStatusWindow:
    case NetworkStatusOk:
    case JoinGuildWindow:
    case JoinGuildNo:
    case CancelTradeWindow:
    case CancelTradeYes:
    case CancelTradeNo:
        return FindPromptWindow(WindowType);

    default:
        break;
    }
    LogError(L"FindWindow: Unsupported WindowType (%d)", WindowType);
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////

HWND
LonWindow_t::
FindPromptWindow(
    const Window::Type_e WindowType) //const
{
    using namespace Window;
    switch (WindowType)
    {
    case TradeBuilderExitPrompt:
    case SystemMessageWindow:
    case AcceptTradeWindow:
    case DeliveryWindow:
    case NetworkStatusWindow:
    case JoinGuildWindow:
    case CancelTradeWindow:
        return FindChildWindow(WindowType);

    case TradeBuilderExitPromptYes:
        return FindChildWindow(TradeBuilderExitPrompt,
                               Prompt::TradeBuilderExitPrompt::Windows,
                               _countof(Prompt::TradeBuilderExitPrompt::Windows),
                               AcceptButton);

    case SystemMessageClose:
        return FindChildWindow(SystemMessageWindow,
                               Prompt::SystemMessageWindow::Windows,
                               _countof(Prompt::SystemMessageWindow::Windows),
                               CloseButton);

    case AcceptTradeYes:
        return FindChildWindow(AcceptTradeWindow,
                               Prompt::AcceptTradeWindow::Windows,
                               _countof(Prompt::AcceptTradeWindow::Windows),
                               AcceptButton);
    case AcceptTradeNo:
        return FindChildWindow(AcceptTradeWindow,
                               Prompt::AcceptTradeWindow::Windows,
                               _countof(Prompt::AcceptTradeWindow::Windows),
                               RejectButton);

    case DeliveryClose:
        return FindChildWindow(DeliveryWindow,
                               Prompt::DeliveryWindow::Windows,
                               _countof(Prompt::DeliveryWindow::Windows),
                               CloseButton);

    case NetworkStatusOk:
        return FindChildWindow(NetworkStatusWindow,
                               Prompt::NetworkStatusWindow::Windows,
                               _countof(Prompt::NetworkStatusWindow::Windows),
                               AcceptButton);

    case JoinGuildNo:
        return FindChildWindow(JoinGuildWindow,
                               Prompt::JoinGuildWindow::Windows,
                               _countof(Prompt::JoinGuildWindow::Windows),
                               RejectButton);

    case CancelTradeYes:
        return FindChildWindow(CancelTradeWindow,
                               Prompt::CancelTradeWindow::Windows,
                               _countof(Prompt::CancelTradeWindow::Windows),
                               AcceptButton);
    case CancelTradeNo:
        return FindChildWindow(CancelTradeWindow,
                               Prompt::CancelTradeWindow::Windows,
                               _countof(Prompt::CancelTradeWindow::Windows),
                               RejectButton);

    default:
        break;
    }
    ASSERT(false);
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////

HWND
LonWindow_t::
FindChildWindow(
          Window::Type_e   TopWindowType,
    const wchar_t* ChildWindows[],
          size_t   Count,
    const wchar_t* WindowName) //const
{
    Handle_t Top(GetTopWindow());
    if (TopWindowType != Top.Type)
        return NULL;
    // Kinda weird. OK for my usage. Should allow for
    // ChildWindows == NULL, WindowName != NULL however
    if (NULL == ChildWindows)
        return Top.hWnd;
    HWND hWnd = WalkChildWindowTitles(Top.hWnd, ChildWindows, Count);
    if (NULL == hWnd)
    {
        ASSERT(false);
        return NULL;
    }
    return GetChildWindow(hWnd, WindowName);
}

/////////////////////////////////////////////////////////////////////////////

HWND
LonWindow_t::
WalkChildWindowTitles(
    HWND hWnd,
    const wchar_t* ChildWindowTitles[],
    const size_t   Count) //const
{
    for (size_t Child = 0; Count > Child; ++Child)
    {
        hWnd = GetChildWindow(hWnd, ChildWindowTitles[Child]);
        if (NULL == hWnd)
            return NULL;
    }
    return hWnd;
}

/////////////////////////////////////////////////////////////////////////////

HWND
LonWindow_t::
FindPostedTradesWindow(
    Window::Type_e WindowType) //const
{
    using namespace Window;
    Handle_t Top(GetTopWindow());
    if (PostedTradesWindow != Top.Type)
        return NULL;
    if (PostedTradesWindow == WindowType)
        return Top.hWnd;

    static const wchar_t*
    ChildWindowTitles[] =
    {
        L"dialogFrame",
        L"PostedTradesDialog",
    };
    HWND hWnd = WalkChildWindowTitles(Top.hWnd, ChildWindowTitles, _countof(ChildWindowTitles));
    if (NULL == hWnd)
    {
        ASSERT(false);
        return NULL;
    }
    switch (WindowType)
    {
    case PostedTradesCreateButton:
        hWnd = GetChildWindow(hWnd, L"buttonBar");
        if (NULL == hWnd)
            return NULL;
        return GetChildWindow(hWnd, L"createButton");
    default:
        break;
    }

    hWnd = GetChildWindow(hWnd, L"treeView");
    if (NULL == hWnd)
        return NULL;
    switch (WindowType)
    {
    case PostedTradesList:
        return hWnd;
    case PostedTradesView:
        return GetChildWindow(hWnd, L"qt_scrollarea_viewport");
    case PostedTradesVScroll:
        return GetChildWindow(hWnd, L"qt_scrollarea_vcontainer");
    default:
        break;
    }
    ASSERT(false);
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////

HWND
LonWindow_t::
FindPostedTradeDetailWindow(
    Window::Type_e WindowType) //const
{
    using namespace Window;
    Handle_t Top(GetTopWindow());
    if (PostedTradeDetailWindow != Top.Type)
        return NULL;
    if (PostedTradeDetailWindow == WindowType)
        return Top.hWnd;

    static const wchar_t*
    ChildWindowTitles[] =
    {
        L"dialogFrame",
        L"PostedTradeDetailDialog",
    };
    HWND hWnd = WalkChildWindowTitles(Top.hWnd, ChildWindowTitles, _countof(ChildWindowTitles));
    if (NULL == hWnd)
    {
        ASSERT(false);
        return NULL;
    }
    switch (WindowType)
    {
    case PostedTradeDetailDialog:
        return hWnd;
    case PostedTradeDetailCancel:
        return GetChildWindow(hWnd, L"cancelButton");
    case PostedTradeDetailAccept:
        return GetChildWindow(hWnd, L"acceptButton");
    case PostedTradeDetailOfferedView:
    case PostedTradeDetailOfferedVScroll:
        hWnd = GetChildWindow(hWnd, L"offeredTreeWidget");
        break;
    case PostedTradeDetailWantView:
    case PostedTradeDetailWantVScroll:
        hWnd = GetChildWindow(hWnd, L"wantTreeWidget");
        break;
    default:
        ASSERT(false);
        return NULL;
    }
    if (NULL == hWnd)
        return NULL;

    switch (WindowType)
    {
    case PostedTradeDetailOfferedView:
    case PostedTradeDetailWantView:
        return GetChildWindow(hWnd, L"qt_scrollarea_viewport");
    case PostedTradeDetailOfferedVScroll:
    case PostedTradeDetailWantVScroll:
        return GetChildWindow(hWnd, L"qt_scrollarea_vcontainer");
    default:
        break;
    }
    ASSERT(false);
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////

HWND
LonWindow_t::
FindTradeBuilderWindow(
    Window::Type_e WindowType) //const
{
    using namespace Window;
    Handle_t Top(GetTopWindow());
    if (TradeBuilderWindow != Top.Type)
        return NULL;
    if (TradeBuilderWindow == WindowType)
        return Top.hWnd;

    static const wchar_t*
    ChildWindowTitles[] = {
        L"dialogFrame",
        L"tradeBuilderWrapper",
        L"postedTradeBuilder",
        L"TradeScreen",
    };
    HWND hWnd = WalkChildWindowTitles(Top.hWnd, ChildWindowTitles, _countof(ChildWindowTitles));
    if (NULL == hWnd)
    {
        ASSERT(false);
        return NULL;
    }
    switch (WindowType)
    {
    case TradeBuilderClear:
        return GetChildWindow(hWnd, L"clearButton");
    case TradeBuilderSubmit:
        return GetChildWindow(hWnd, L"theyGetAcceptButton");
    case TradeBuilderExit:
        return GetChildWindow(hWnd, L"exitButton");
    default:
        break;
    }

    hWnd = GetChildWindow(hWnd, L"collectionFrame");
    if (NULL == hWnd)
        return NULL;

    bool bYour = false;
    switch (WindowType)
    {
    case TradeBuilderCollectionFrame:
        return hWnd;

    case TradeBuilderCollectionTab:
        hWnd = GetChildWindow(hWnd, L"ContainerHeader");
        if (NULL == hWnd)
            return NULL;
        return GetChildWindow(hWnd, L"tabBar");

    case TradeBuilderYourTableList:
    case TradeBuilderYourTableView:
    case TradeBuilderYourTableVScroll:
    case TradeBuilderYourTableHScroll:
        bYour = true;
        // fall through
    case TradeBuilderTheirTableList:
    case TradeBuilderTheirTableView:
    case TradeBuilderTheirTableVScroll:
    case TradeBuilderTheirTableHScroll:
        if (bYour)
            hWnd = GetChildWindow(hWnd, L"collectionArea");
        else
            hWnd = GetChildWindow(hWnd, L"theirCollectionArea");
        if (NULL == hWnd)
            return NULL;

        hWnd = GetChildWindow(hWnd, L"collectionTableView");
        if (NULL == hWnd)
            return NULL;
        switch (WindowType)
        {
        case TradeBuilderYourTableList:
        case TradeBuilderTheirTableList:
            return hWnd;
        case TradeBuilderYourTableView:
        case TradeBuilderTheirTableView:
            return GetChildWindow(hWnd, L"qt_scrollarea_viewport");
            break;
        case TradeBuilderYourTableVScroll:
        case TradeBuilderTheirTableVScroll:
            return GetChildWindow(hWnd, L"qt_scrollarea_vcontainer");
        case TradeBuilderYourTableHScroll:
        case TradeBuilderTheirTableHScroll:
            return GetChildWindow(hWnd, L"qt_scrollarea_hcontainer");
        default:
            break;
        }
        break;

    case TradeBuilderSearchEdit:
    case TradeBuilderSearchEditClear:
        hWnd = GetChildWindow(hWnd, L"collectionButtonBar");
        if (NULL == hWnd)
            return NULL;
        hWnd = GetChildWindow(hWnd, L"searchEdit");
        if (TradeBuilderSearchEdit == WindowType)
            return hWnd;
        if (NULL == hWnd)
            return NULL;
        return ::GetWindow(hWnd, GW_CHILD);

    default:
        break;
    }
    ASSERT(false);
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////

HWND
LonWindow_t::
FindConfirmTradeWindow(
    Window::Type_e WindowType)
{
    using namespace Window;
    Handle_t Top(GetTopWindow());
    if (ConfirmTradeWindow != Top.Type)
        return NULL;
    if (ConfirmTradeWindow == WindowType)
        return Top.hWnd;

    static const wchar_t*
    ChildWindowTitles[] =
    {
        L"dialogFrame",
        L"TradeConfirmDialog"
    };
    HWND hWnd = WalkChildWindowTitles(Top.hWnd, ChildWindowTitles, _countof(ChildWindowTitles));
    if (NULL == hWnd)
    {
        ASSERT(false);
        return NULL;
    }
    switch (WindowType)
    {
    case ConfirmTradeCancel:
        return GetChildWindow(hWnd, L"cancelButton");
    case ConfirmTradeConfirm:
        return GetChildWindow(hWnd, L"confirmButton");

    case ConfirmTradeYouGetView:
    case ConfirmTradeYouGetVScroll:
        hWnd = GetChildWindow(hWnd, L"youGetTreeWidget");
        if (NULL == hWnd)
            return NULL;
        break;
    case ConfirmTradeTheyGetView:
    case ConfirmTradeTheyGetVScroll:
        hWnd = GetChildWindow(hWnd, L"theyGetTreeWidget");
        if (NULL == hWnd)
            return NULL;
        break;
    default:
        ASSERT(false);
        return NULL;
    }

    switch (WindowType)
    {
    case ConfirmTradeYouGetView:
    case ConfirmTradeTheyGetView:
        return GetChildWindow(hWnd, L"qt_scrollarea_viewport");
    case ConfirmTradeYouGetVScroll:
    case ConfirmTradeTheyGetVScroll:
        return GetChildWindow(hWnd, L"qt_scrollarea_vcontainer");
    default:
        break;
    }
    ASSERT(false);
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
