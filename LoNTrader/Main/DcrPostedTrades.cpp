/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// Handler_t.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DcrPostedTrades.h"
#include "SsTrades_t.h"
#include "PipelineManager.h"
#include "LonWindow_t.h"

namespace PostedTrades
{
namespace Translate
{

/////////////////////////////////////////////////////////////////////////////

Handler_t::
Handler_t() :
    TrWindow_t(TopWindowType, m_TranslatePolicy, m_ValidatePolicy),
    m_TranslatePolicy(TopWindowType, m_Dcr, DcrWindowType, &m_rcBounds),
    m_Dcr(&m_Text, Table::PixelColumnWidths, Table::ColumnCount),
    m_Text(Table::CharColumnWidths, Table::ColumnCount)
{
    SetRect(&m_rcBounds, 0, 0, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
PostData(
    DWORD /*Unused*/)
{
    Translate::Data_t *pData = reinterpret_cast<Translate::Data_t*>
        (GetPipelineManager().Alloc(sizeof(Translate::Data_t)));
    if (NULL == pData)
    {
        LogError(L"Handler_t::PostData(): Alloc callback data failed.");
    }
    else
    {
        m_Text.Dump(L"DcrPostedTrades::PostData()");
        pData->Initialize(this);
        HRESULT hr = GetPipelineManager().Callback(pData);
        ASSERT(SUCCEEDED(hr));
    }
}

/////////////////////////////////////////////////////////////////////////////

void
Data_t::
Initialize(
    const Handler_t* pClass)
{
    wcscpy_s(Class, _countof(Class), pClass->GetClass().c_str());
    Stage      = DP::Stage::Translate;
    Type       = DP::Message::Type::Message;
    Id         = Lon::Message::Id::TextTable;
    WindowType = pClass->TopWindowType;
    Text       = pClass->m_Text.GetData();
    Rect       = pClass->m_rcBounds;
    Size       = sizeof(Translate::Data_t);
}

/////////////////////////////////////////////////////////////////////////////

} // Translate
} // PostedTrades
