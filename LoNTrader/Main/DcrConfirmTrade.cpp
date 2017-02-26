/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DcrConfirmTrade.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DcrConfirmTrade.h"
#include "PipelineManager.h"

namespace ConfirmTrade
{
namespace Translate
{

/////////////////////////////////////////////////////////////////////////////
//
// TODO: placement new/constructor
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
    m_WantText(Table::CharColumnWidths, Table::ColumnCount),
    m_OfferedRequired(1),
    m_WantRequired(1)
{
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
PostData(
    DWORD /*AcquireId*/)
{
    Data_t *pData;
    pData = (Data_t*)
        GetPipelineManager().Alloc(sizeof(Data_t));
    if (NULL == pData)
    {
        LogError(L"ConfirmTrade::Handler_t::PostData(): Alloc callback data failed.");
    }
    else
    {
        pData->Initialize(GetClass().c_str(), m_OfferedText, m_WantText);
        HRESULT hr = GetPipelineManager().Callback(pData);
        ASSERT(SUCCEEDED(hr));
    }
}

/////////////////////////////////////////////////////////////////////////////

} // Translate
} // ConfirmTrade

/////////////////////////////////////////////////////////////////////////////
