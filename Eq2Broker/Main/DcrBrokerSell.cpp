////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DcrBrokerSell.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BrokerSell.h"
#include "PipelineManager.h"
#include "SsWindow.h"
#include "DdUtil.h"
#include "BrokerId.h"

#ifdef _DEBUG
//#define new DP_DEBUG_PLACEMENT_NEW
#endif

namespace Broker
{
namespace Sell
{
namespace Translate
{

//DP::Message::Id_t Data_t::s_MessageId      = Broker::Message::Id::Sell; // DP::Message::Id::Unknown;

static RECT
TextRects[Table::ColumnCount] =
{
    // Quantity text is bottom "QuantityTextHeight" lines of first column
    { 0, Broker::Table::RowHeight - Broker::Table::QuantityTextHeight,
      Table::PixelColumnWidths[0], Broker::Table::RowHeight },
    { 0 },
    { 0 },
    { 0 },
};

const ScreenTable_t
Handler_t::s_ScreenTable = 
{
    Broker::Table::RowHeight + Broker::Table::GapSizeY,
    Broker::Table::CharHeight,
    Broker::Table::GapSizeY,
    Table::ColumnCount,
    Table::PixelColumnWidths,
    TextRects
};

////////////////////////////////////////////////////////////////////////////////

Handler_t::
Handler_t(
    Window::ManagerBase_t& Manager)
:
    HandlerBase_t(
        TopWindowId,
        m_TranslatePolicy,
        m_ValidatePolicy,
        L"BrokerSell"),
    m_TranslatePolicy(m_DcrVector),
    m_DcrTable(&m_TextTable, Manager.GetWindow(), s_ScreenTable),
    m_TextTable(Table::CharColumnWidths, Table::ColumnCount),
    m_Manager(Manager)
{
    m_DcrVector.push_back(&m_DcrTable);
}

////////////////////////////////////////////////////////////////////////////////

void
Handler_t::
PostData(
    DWORD /*Unused*/)
{
    extern bool g_noDcrPost;
    if (g_noDcrPost)
    {
        return;
    }

    void *pBuffer = GetPipelineManager().Alloc(sizeof(Data_t));
    if (NULL != pBuffer)
    {
        m_TextTable.Dump(L"DcrBrokerSell::PostData()");
        LogInfo(L"SeletecedRow(%d)", m_DcrTable.GetSelectedRow());
        Data_t* pData = new (pBuffer)
            Data_t(GetClass().c_str(),
                   m_TextTable,
                   m_DcrTable.GetSelectedRow(),
                   GetManager().GetWindow().GetScrollPosition(Ui::Scroll::Bar::Vertical));
        GetPipelineManager().Callback(pData);
    }
    else
    {
        LogError(L"DcrBrokerSell::PostData(): Alloc callback data failed.");
    }
}

////////////////////////////////////////////////////////////////////////////////

} // Translate
} // Sell
} // Broker

////////////////////////////////////////////////////////////////////////////////
