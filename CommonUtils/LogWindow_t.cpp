#include "stdafx.h"
#include "Log_t.h"

LRESULT CALLBACK FormWndProc(__in HWND hwnd, __in UINT message, __in WPARAM wParam, __in LPARAM lParam);
INT_PTR CALLBACK DialogProc(__in HWND hwnd, __in UINT message, __in WPARAM wParam, __in LPARAM lParam);

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

HWND RegisterAndCreate(__in HINSTANCE hInstance);

// ----------------------------------------------------------------------------

void
LogWindow_t::
Init()
{
#if 1
return;
#else
    HINSTANCE hInst = (HINSTANCE)&__ImageBase;
#if 0
    m_hWnd = RegisterAndCreate(hInst);
#else
    m_hWnd = ::CreateDialog(hInst, MAKEINTRESOURCE(101), 0, DialogProc);
    if (0 != m_hWnd)
    {
        ::ShowWindow(m_hWnd, SW_SHOW);
    }
#endif
    if (0 == m_hWnd)
    {
        throw logic_error("LogWindow_t::Init()");
    }
#endif
}

// ----------------------------------------------------------------------------

/*
void
LogWindow_t::
Log(
    shared_ptr<const wstring>& buf)
{
    if (::IsWindow(GetHwnd()))
    {
        shared_ptr<const wstring>* spStr = new shared_ptr<const wstring>(buf);
        PostMessage(GetHwnd(), WM_USER, 0, LPARAM(spStr));
    }
}
*/

// ----------------------------------------------------------------------------

void
LogWindow_t::
Log(
    const wchar_t* buf)
{
    if (::IsWindow(GetHwnd()))
    {
        SendMessage(GetHwnd(), WM_USER+1, 0, LPARAM(buf));
    }
}

// ----------------------------------------------------------------------------

HWND RegisterAndCreate(__in HINSTANCE hInstance)
{
//    RegisterControl(hInstance);

    WNDCLASS wc;
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = FormWndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInstance;
    wc.hIcon            = NULL;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = NULL;
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = L"LogWindow_t";

    RegisterClass(&wc);

    return CreateWindow(wc.lpszClassName, L"Log Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 300, NULL, NULL, hInstance, NULL);
}

// ----------------------------------------------------------------------------

LRESULT CALLBACK FormWndProc(__in HWND hwnd, __in UINT message, __in WPARAM wParam, __in LPARAM lParam)
{
    static HWND g_hwndList;

    switch (message) 
    {
        case WM_CREATE:
        {
            g_hwndList = CreateWindow(L"listbox", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOINTEGRALHEIGHT | LBS_HASSTRINGS | WS_VSCROLL,
                                      0, 0, 1, 1, hwnd, NULL, (HINSTANCE)&__ImageBase, NULL);
            HFONT hFont = GetStockFont(DEFAULT_GUI_FONT);
            SendMessage(g_hwndList, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            break;
        }

        case WM_SIZE:
        {
            RECT rc;
            GetClientRect(hwnd, & rc);
            SetWindowPos(g_hwndList, NULL, rc.left, rc.top, rc.right, rc.bottom, SWP_NOZORDER | SWP_NOACTIVATE);
            break;
        }

        case WM_CLOSE:
            PostQuitMessage(0);
            break;

        case WM_SETFOCUS:
            SetFocus(g_hwndList);
            break;

        default:
            break;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

INT_PTR CALLBACK DialogProc(__in HWND hwnd, __in UINT message, __in WPARAM wParam, __in LPARAM lParam)
{
wParam;
    HWND hChild = GetWindow(hwnd, GW_CHILD);
    switch (message) 
    {
    case WM_INITDIALOG:
        return FALSE;
    case WM_SIZE:
        {
            RECT rc;
            GetClientRect(hwnd, &rc);
            SetWindowPos(hChild, NULL, rc.left, rc.top, rc.right, rc.bottom, SWP_NOZORDER | SWP_NOACTIVATE);
        }
        return TRUE;
    case WM_USER:
        {
            ASSERT(0);
            shared_ptr<const wstring> *str = (shared_ptr<const wstring>*)lParam;
            SendMessage(hChild, LB_ADDSTRING, 0, LPARAM(str->get()->c_str()));
            delete str;
        }
        return TRUE;
    case WM_USER+1:
        {
            const wchar_t *str = (const wchar_t*)lParam;
            SendMessage(hChild, LB_ADDSTRING, 0, LPARAM(str));
        }
        return TRUE;
    default:
        break;
    }
    return FALSE;
}

