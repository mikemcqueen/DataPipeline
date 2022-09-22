////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DcrBrokerBuy.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BrokerBuy.h"
#include "DcrBrokerBuy.h"
#include "PipelineManager.h"
#include "SsWindow.h"
#include "DdUtil.h"
#include "BrokerId.h"
#include "BrokerUi.h"

namespace Broker
{
namespace Buy
{
namespace Translate
{

// TODO:
// columndata offset/width pairs; column widths is sum if specified
// 0:  2,42
// 1:  11,560
// 2:   ....
// Take a look at ColumnData_t in ScreenTable_t.  It seems we could 
// combine PixelColumnWidths, along with "gap" columns (widths) and
// "text" column (rects) perhaps as a union

static RECT
TextRects[Table::ColumnCount] = {
    { 2, 26, // first gap + quantitytexttop 
      2 + 44, 26 + 13 },  // first gap + first column data width , quantitytexttop + quantitytextheight
    { 2 + 44 + 7, 0,  // first gap + first column data width + second gap 
      Table::PixelColumnWidths[1], Broker::Table::RowHeight },
    { 0 },
    { 767, 0,
      767 + 23, Broker::Table::RowHeight },
    { 0 }
};

constexpr TableParams_t TableParams = {
    Broker::Table::RowHeight,//+ Broker::Table::GapSizeY,
    Broker::Table::CharHeight,
    Broker::Table::RowGapSize,
    Table::ColumnCount
};

////////////////////////////////////////////////////////////////////////////////

Handler_t::
Handler_t(
    Window::ManagerBase_t& windowManager)
    :
    BaseHandler_t(
        TopWindowId,
        m_TranslatePolicy,
        m_ValidatePolicy,
        L"BrokerBuy"),
    m_TranslatePolicy(
        *this,
        m_DcrVector), //variadic args?
    m_DcrTable(
        Broker::Buy::Widget::Id::Table,
        &m_TextTable,
        windowManager.GetWindow(),
        TableParams,
        std::span{ Table::PixelColumnWidths },
        std::span{ TextRects }),
    m_TextTable(
        std::span{ Table::CharColumnWidths }),
    m_DcrSearchEdit(
        Broker::Buy::Widget::Id::SearchEdit,
        //windowManager.GetWindow(),
        //Widget::Id::SearchEdit,
        //DcrBase_t::GetCharset(),
        nullptr,
        true),
    m_DcrSearchDropdown(
        Broker::Buy::Widget::Id::SearchDropdown),
        //windowManager.GetWindow(),
        //Widget::Id::SearchDropdown,
        //DcrBase_t::GetCharset()),
    m_DcrPageNumber(
        Broker::Buy::Widget::Id::PageNumber),
        //windowManager.GetWindow(),
        //Widget::Id::PageNumber),
    m_windowManager(windowManager)
{
    m_DcrVector.push_back(&m_DcrTable);
    m_DcrVector.push_back(&m_DcrSearchEdit);
    m_DcrVector.push_back(&m_DcrSearchDropdown);
    m_DcrVector.push_back(&m_DcrPageNumber);
}

////////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
PreTranslateSurface(
    CSurface* pSurface,
    Ui::WindowId_t windowId,
    int dcrId,
    Rect_t* pRect) const
{
    extern bool g_bTableFixColor;
    windowId;
    pSurface;
    switch (dcrId) {
    case Broker::Buy::Widget::Id::Table:
    {
        //rcSurface;
        Rect_t rect = m_windowManager.GetWindow().GetClientRect();
        if (!IsRectEmpty(&rect))
        {
            rect.top += Broker::Table::TopRowOffset;
            //m_selectedRow = GetSelectedRow(*pSurface, rect);
#if 0
            if (g_bTableFixColor)
            {
                // TODO: pSurface->ReplaceColorRange
                pSurface->FixColor(rect, BkLowColor, BkHighColor, Black);
            }
#endif
            *pRect = rect;
            return true;
        }
    }
    default:
        break;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

void
Handler_t::
PostData(
    DWORD /*Unused*/) const
{
    extern bool g_noDcrPost;
    if (g_noDcrPost)
    {
        return;
    }
    //bool ExtraLog = 0;
    // 
    PageNumber_t PageNumber;
    if (!m_DcrPageNumber.GetText().empty() &&
        !PageNumber.Parse(m_DcrPageNumber.GetText()))
    {
        LogError(L"DcrBrokerBuy::PostData(): PageNumber.Parse(%ls) failed",
                 m_DcrPageNumber.GetText().c_str());
        return;
    }
    void *pBuffer = GetPipelineManager().Alloc(sizeof(Data_t));
    if (nullptr == pBuffer)
    {
        LogError(L"DcrBrokerBuy::PostData(): Alloc callback data failed.");
    }
    else
    {
        m_TextTable.GetData().Dump(L"DcrBrokerBuy::PostData()");
        //LogInfo(L"SeletecedRow(%d)", m_DcrTable.GetSelectedRow());
        Data_t* pData = new (pBuffer)
            Data_t(
                GetClass().c_str(),
                m_TextTable,
                m_DcrTable.GetSelectedRow(),
                PageNumber,
                m_DcrSearchEdit.GetText(),
                m_DcrSearchEdit.GetHasCaret(),
                m_DcrSearchDropdown.GetText());
        HRESULT hr = GetPipelineManager().Callback(pData);
        if (FAILED(hr))
        {
            LogError(L"DcrBrokerBuy::PostData(): PM.Callback() failed.");
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

} // Translate
} // Buy
} // Broker

////////////////////////////////////////////////////////////////////////////////
