/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DcrTradeDetail.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DcrTradeDetail.h"
#include "SsTrades_t.h"
#include "PipelineManager.h"

namespace TradeDetail
{
namespace Translate
{

/////////////////////////////////////////////////////////////////////////////
//
// call DcrTrades_t::data::init ?
//

void
Data_t::
Initialize(
    const wchar_t*       pszClass,
    const Table::Text_t& Offered,
    const Table::Text_t& Want)
{
    wcscpy_s(Class, _countof(Class), pszClass);
    Stage       = DP::Stage::Translate;
    Type        = DP::Message::Type::Message;
    Id          = Lon::Message::Id::TextTable;
    WindowType  = TopWindowType;
    OfferedText = Offered.GetData();
    WantText    = Want.GetData();
}

/////////////////////////////////////////////////////////////////////////////

Handler_t::
Handler_t() :
    TrWindow_t(TopWindowType, m_TranslatePolicy, m_ValidatePolicy),
    m_TranslatePolicy(TopWindowType, m_DcrOffered, DcrOfferedWindowType,
                                     m_DcrWant,    DcrWantWindowType),
    m_DcrOffered(&m_OfferedText, Table::PixelColumnWidths, Table::ColumnCount),
    m_OfferedText(Table::CharColumnWidths, Table::ColumnCount),
    m_DcrWant(&m_WantText, Table::PixelColumnWidths, Table::ColumnCount),
    m_WantText(Table::CharColumnWidths, Table::ColumnCount)
{
}

/////////////////////////////////////////////////////////////////////////////

#if 0
void
Handler_t::
Translate(
    const DP::AcquireData_t* pData)
{
// If we got empty text it could be a bug.
if ((L'\0' == m_WantText.GetLine(0)[0]) ||
    (L'\0' == m_OfferedText.GetLine(0)[0]))
{
    static int num = 0;
    wchar_t buf[256];
    wsprintf(buf, L"bmp\\tradedetail_notext_%d.bmp", ++num);
    pSurface->WriteBMP(buf, rcSurface);
}
}
#endif

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
PostData(
    DWORD /*AcquireId*/)
{
    Data_t* pData = (Data_t*)GetPipelineManager().Alloc(sizeof(Data_t));
    if (NULL == pData)
    {
        LogError(L"TradeDetail::Translate::Handler_t::PostData(): Alloc callback data failed.");
    }
    else
    {
        pData->Initialize(GetClass().c_str(), m_WantText, m_OfferedText); 
        HRESULT hr = GetPipelineManager().Callback(pData);
        ASSERT(SUCCEEDED(hr));
    }
}

/////////////////////////////////////////////////////////////////////////////

} // Translate
} // TradeDetail

/////////////////////////////////////////////////////////////////////////////
