////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TxOpenBroker.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PipelineManager.h"
#include "TransactionManager.h"
#include "TxOpenBroker.h"
#include "UiEvent.h"
#include "Eq2Broker_t.h"
#include "UiWindow.h"
#include "BrokerUi.h"
#include "SsWindow.h"
#include "UiInput.h"
#include "MainWindow_t.h"
#include "Resource.h"
#include "TrWindowType.h"
#include "DdUtil.h"

namespace Broker
{
namespace Transaction
{
namespace OpenBroker
{

POINT Handler_t::s_lastPoint = { 0, 0 };

////////////////////////////////////////////////////////////////////////////////

Handler_t::
Handler_t(
    Eq2Broker_t& broker)
:
    m_broker(broker)
{
    loadCursor();
}

////////////////////////////////////////////////////////////////////////////////

void
Handler_t::
loadCursor()
{
#if 0 // TODO
    HCURSOR hCursor = LoadCursor(::GetModuleHandle(nullptr), MAKEINTRESOURCE(IDC_BROKER_CURSOR));
    if ((nullptr == hCursor) ||
        !GetCursorBits(hCursor, kCursorWidth, kCursorHeight, m_cursorBits))
    {
        throw logic_error("TxOpenBroker::loadCursor()");
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
ExecuteTransaction(
    DP::Transaction::Data_t& data)
{
    LogInfo(L"TxOpenBroker::ExecuteTransaction()");
    using Broker::Transaction::OpenBroker::Data_t;
    Data_t& txData = static_cast<Data_t&>(data);
    txData.SetState(State::First);
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
OnTransactionComplete(
    const DP::Transaction::Data_t&)
{
    LogInfo(L"TxOpenBroker::TransactionComplete()");
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
MessageHandler(
    const DP::Message::Data_t* pMessage)
{
    LogInfo(L"TxOpenBroker::MessageHandler()");
    DP::TransactionManager_t::AutoRelease_t ar(GetTransactionManager().Acquire());
    DP::Transaction::Data_t* pTxData = ar.get();
    if (nullptr == pTxData)
    {
        throw logic_error("TxOpenBroker::MessageHandler() No transaction active");
    }
    const SsWindow::Acquire::Data_t&
        ssData = static_cast<const SsWindow::Acquire::Data_t&>(*pMessage);
#if 0
    // well, unfortunately at this point the ssData hasn't been updated
    // to include WindowId (via TrWindowType) because as a transaction handler
    // we trump and therefore execute before all translate handlers (including
    // TrWindowType).  Soget the Id from TrWindowType indirectly (via Eq2Broker).
    Ui::WindowId_t windowId = ssData.WindowId;
#else
    Ui::WindowId_t windowId = m_broker.GetWindowId(*ssData.pPoolItem->get());
#endif
    Data_t& txData = static_cast<Data_t&>(*pTxData);
    if (Window::Id::MainChat != windowId)
    {
        if (IsBrokerWindow(windowId))
        {
            txData.Complete();
            return S_OK;
        }
        else if (Ui::Window::Id::Unknown == windowId)
        {
            // Unkown window just because it happens sometimes.. especially
            // in alt/tab testing, never stays here for long
            return S_OK;
        }
        else
        {
            txData.Complete(Error::InvalidWindow);
            return S_FALSE;
        }
    }
    return OpenBroker(ssData, txData);
}

////////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
IsBrokerWindow(
    Ui::WindowId_t windowId)
{
    switch (windowId)
    {
    case Window::Id::BrokerFrame:
    case Window::Id::BrokerBuyTab:
    case Window::Id::BrokerSellTab:
    case Window::Id::BrokerSalesLogTab:
    case Window::Id::BrokerSetPricePopup:
        return true;
    default:
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
OpenBroker(
    const SsWindow::Acquire::Data_t& ssData,
    Data_t& txData) const
{
    switch (txData.GetState())
    {
    case State::MoveToLastPoint:
        if (0 < s_lastPoint.x || 0 < s_lastPoint.y)
        {
            Ui::Input_t::MoveToAbsolute(m_broker.GetMainWindow().GetHwnd(), s_lastPoint);
            txData.point = s_lastPoint;
            txData.skip = true;
        }
        txData.NextState();
        break;

    case State::FindBroker:
        if (IsBrokerCursor())
        {
            txData.NextState();
        }
        else if (txData.skip)
        {
            txData.skip = false;
        }
        else
        {
            POINT pt;
            const CSurface* pSurface = ssData.pPoolItem->get();
            if (GetPositionPoint(pSurface->GetBltRect(), txData.position, pt))
            {
                Ui::Input_t::MoveToAbsolute(m_broker.GetMainWindow().GetHwnd(), pt);
                ++txData.position;
                txData.point = pt;
                txData.skip = true;
            }
            else
            {
                txData.Complete(Error::BrokerNotFound);
            }
        }
        break;

    case State::ClickBroker:
        if (!IsBrokerCursor())
        {
            txData.position = 0;
            txData.PrevState();
        }
        else
        {
            GetPipelineManager().StopAcquiring();
            GetPipelineManager().Flush(DP::Stage::Acquire | DP::Stage::Translate,
                                       GetClass().c_str());
            Ui::Input_t::Click();
            s_lastPoint = txData.point;
            GetPipelineManager().StartAcquiring();
        }
        break;

    default:
        throw logic_error("TxOpenBroker::OpenBroker() Invalid state");
    }
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
IsBrokerCursor() const
{
    char bits[kCursorHeight * (kCursorWidth / 8)];
    if (GetCursorBits(kCursorWidth, kCursorHeight, bits))
    {
        if (0 == memcmp(bits, m_cursorBits, sizeof(bits)))
        {
            LogAlways(L"Cursor(Broker)");
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
GetCursorBits(
    size_t width, 
    size_t height,
    char*  bits) const
{
    CURSORINFO cursorInfo = { sizeof(CURSORINFO) };
    if (GetCursorInfo(&cursorInfo))
    {
        return GetCursorBits(cursorInfo.hCursor, width, height, bits);
    }
    else
    {
        LogError(L"TxOpenBroker::GetCursorBits() GetCursorInfo() failed");
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
GetCursorBits(
    HCURSOR hCursor,
    size_t  width, 
    size_t  height,
    char*   bits) const
{
    ICONINFO iconInfo = { sizeof(ICONINFO) };
    if (GetIconInfo(hCursor, &iconInfo))
    {
        BITMAP bm;
        GetObject(iconInfo.hbmMask, sizeof(bm), &bm);
        if ((1 == bm.bmBitsPixel) &&
            (int(width) == bm.bmWidth) && 
            (int(height) == bm.bmHeight))
        {
            struct monoBITMAPINFO {
                BITMAPINFOHEADER header;
                RGBQUAD          colors[2];
            } bitmapInfo = { 0 };
            bitmapInfo.header.biSize = sizeof(BITMAPINFOHEADER);
            bitmapInfo.header.biWidth = bm.bmWidth;
            bitmapInfo.header.biHeight = -bm.bmHeight;
            bitmapInfo.header.biPlanes = 1;
            bitmapInfo.header.biBitCount = 1;
            bitmapInfo.header.biCompression = BI_RGB;
            HDC hDC = GetDC(nullptr);
            int lines = GetDIBits(hDC, iconInfo.hbmMask, 0, bm.bmHeight, bits,
                                  (LPBITMAPINFO)&bitmapInfo, DIB_PAL_COLORS);
            lines;
            ReleaseDC(nullptr, hDC);
            return true;
        }
        else
        {
            LogInfo(L"TxOpenBroker: Unsupported cursor format");
        }
    }
    else
    {
        LogError(L"TxOpenBroker::GetCursorBits() GetIconInfo() failed");
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
GetPositionPoint(
    const Rect_t& rect,
          size_t  position,
          POINT&  outPoint) const
{
    SIZE move = { 90, 90 };
    POINT center = rect.Center();
    POINT pt = center;
    for (; 0 < position; --position)
    {
        if (pt.y + move.cy < rect.Height())
        {
            pt.y += move.cy;
        }
        else
        {
            const int curColumn = (pt.x - center.x) / move.cx;
            int newColumn = (curColumn <= 0) ? abs(curColumn) + 1 : -curColumn;
            const int newX = center.x + newColumn * move.cx;
            if ((0 < newX) && (rect.Width() > newX))
            {
                pt.x = newX;
                pt.y = center.y;
            }
            else
            {
                return false;
            }
        }
    }
    outPoint = pt;
    return true;
}

////////////////////////////////////////////////////////////////////////////////

} // OpenBroker
} // Transaction
} // Broker

/////////////////////////////////////////////////////////////////////////////
